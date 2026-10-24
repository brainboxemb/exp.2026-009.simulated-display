#include "serial_lcd.h"

#include "custom_bitmap.h"
#include "custom_character_map.h"

#define COMMAND_PREFIX ((uint8_t)'/')
#define SPRITE_MARKER 0xfeu

/*
 * Legacy dispatcher mapping:
 *   B = predefined bitmap
 *   G = graph burst
 *
 * Instruction.asm contains a stale "/Bxxbbb" comment inside GRAPHBURST;
 * the actual command letter for graph burst is G.
 */
#define CMD_BITMAP ((uint8_t)'B')
#define CMD_CLEAR ((uint8_t)'C')
#define CMD_POSITION ((uint8_t)'L')
#define CMD_SPRITE_MENU ((uint8_t)'N')

#define SPRITE_CMD_VELOCITY ((uint8_t)'V')

#define CHARACTER_HEIGHT 7u
#define SPRITE_ADVANCE 8u
#define SPRITE_DEFAULT_VELOCITY 4u

static void set_pixel(SerialLcd *lcd, int x, int y, bool on) {
    if (x < 0 || y < 0 || x >= (int)DISPLAY_WIDTH || y >= (int)DISPLAY_HEIGHT) {
        return;
    }

    if (display_framebuffer_get(
                &lcd->framebuffer,
                (uint16_t)x,
                (uint16_t)y) == on) {
        return;
    }

    display_framebuffer_set(
            &lcd->framebuffer,
            (uint16_t)x,
            (uint16_t)y,
            on);
    lcd->dirty = true;
}

static void reset_text_history(SerialLcd *lcd) {
    lcd->text_length = 0;
}

static void reset_sprites(SerialLcd *lcd) {
    lcd->sprite_count = 0;
    lcd->sprite_frame = 0;
    lcd->sprite_elapsed_ticks = 0;
}

static void clear_screen(SerialLcd *lcd) {
    display_framebuffer_clear(&lcd->framebuffer);
    lcd->cursor_x = 0;
    lcd->cursor_y = 0;
    reset_text_history(lcd);
    reset_sprites(lcd);
    lcd->dirty = true;
}

static void next_line(SerialLcd *lcd) {
    lcd->cursor_x = 0;

    const uint16_t next_y = (uint16_t)lcd->cursor_y + 8u;
    lcd->cursor_y =
            next_y < DISPLAY_HEIGHT
                    ? (uint8_t)next_y
                    : 0;
}

static void ensure_horizontal_space(
        SerialLcd *lcd,
        uint8_t advance) {
    if ((uint16_t)lcd->cursor_x + advance > DISPLAY_WIDTH) {
        next_line(lcd);
    }
}

static void draw_columns_at(
        SerialLcd *lcd,
        uint8_t origin_x,
        uint8_t origin_y,
        const uint8_t columns[CUSTOM_CHARACTER_COLUMNS],
        uint8_t width) {
    for (uint8_t x = 0; x < width; x++) {
        const uint8_t visible_bits =
                (uint8_t)(columns[x] & 0x7fu);

        for (uint8_t y = 0; y < CHARACTER_HEIGHT; y++) {
            set_pixel(
                    lcd,
                    origin_x + x,
                    origin_y + y,
                    (visible_bits & (uint8_t)(1u << y)) != 0);
        }
    }
}

static void draw_columns(
        SerialLcd *lcd,
        const uint8_t columns[CUSTOM_CHARACTER_COLUMNS]) {
    draw_columns_at(
            lcd,
            lcd->cursor_x,
            lcd->cursor_y,
            columns,
            CUSTOM_CHARACTER_COLUMNS);
}

static void record_text_entry(
        SerialLcd *lcd,
        uint8_t clear_width) {
    if (lcd->text_length >= SERIAL_LCD_TEXT_HISTORY_CAPACITY) {
        reset_text_history(lcd);
    }

    SerialLcdTextEntry *entry =
            &lcd->text_history[lcd->text_length++];

    entry->x = lcd->cursor_x;
    entry->y = lcd->cursor_y;
    entry->clear_width = clear_width;
}

static void advance_cursor(
        SerialLcd *lcd,
        uint8_t advance) {
    lcd->cursor_x = (uint8_t)(lcd->cursor_x + advance);

    if (lcd->cursor_x >= DISPLAY_WIDTH) {
        next_line(lcd);
    }
}

static void draw_character(
        SerialLcd *lcd,
        uint8_t code) {
    CustomCharacterGlyph glyph;

    if (!custom_character_map_lookup(code, &glyph)) {
        return;
    }

    ensure_horizontal_space(lcd, glyph.advance);
    draw_columns(lcd, glyph.columns);
    record_text_entry(lcd, CUSTOM_CHARACTER_COLUMNS);
    advance_cursor(lcd, glyph.advance);
}

static void render_sprite(
        SerialLcd *lcd,
        const SerialLcdSprite *sprite) {
    draw_columns_at(
            lcd,
            sprite->x,
            sprite->y,
            custom_character_map_sprite_frame(lcd->sprite_frame),
            CUSTOM_SPRITE_WIDTH);
}

static void place_sprite(SerialLcd *lcd) {
    ensure_horizontal_space(lcd, SPRITE_ADVANCE);

    if (lcd->sprite_count >= SERIAL_LCD_SPRITE_CAPACITY) {
        return;
    }

    SerialLcdSprite *sprite =
            &lcd->sprites[lcd->sprite_count++];

    sprite->x = lcd->cursor_x;
    sprite->y = lcd->cursor_y;

    /*
     * Character 254 reserves seven sprite columns plus one blank separator.
     * Bit 7 is metadata in the legacy display memory and is never visible.
     */
    for (uint8_t x = 0; x < SPRITE_ADVANCE; x++) {
        for (uint8_t y = 0; y < CHARACTER_HEIGHT; y++) {
            set_pixel(
                    lcd,
                    lcd->cursor_x + x,
                    lcd->cursor_y + y,
                    false);
        }
    }

    render_sprite(lcd, sprite);
    record_text_entry(lcd, SPRITE_ADVANCE);
    advance_cursor(lcd, SPRITE_ADVANCE);
}

static void draw_bitmap(
        SerialLcd *lcd,
        const CustomBitmap *bitmap) {
    for (uint8_t y = 0; y < bitmap->height; y++) {
        const uint8_t band = (uint8_t)(y / 7u);
        const uint8_t bit = (uint8_t)(y % 7u);

        for (uint8_t x = 0; x < bitmap->width; x++) {
            const uint8_t column =
                    bitmap->bands[
                            (size_t)band * bitmap->width + x];

            set_pixel(
                    lcd,
                    lcd->cursor_x + x,
                    lcd->cursor_y + y,
                    (column & (uint8_t)(1u << bit)) != 0);
        }
    }
}

static void draw_bitmap_digit(
        SerialLcd *lcd,
        uint8_t value) {
    if (value < (uint8_t)'0' || value > (uint8_t)'9') {
        return;
    }

    CustomBitmap bitmap;

    if (!custom_bitmap_digit_lookup(
                (uint8_t)(value - (uint8_t)'0'),
                &bitmap)) {
        return;
    }

    draw_bitmap(lcd, &bitmap);
}

static void erase_previous_text_entry(SerialLcd *lcd) {
    if (lcd->text_length == 0) {
        return;
    }

    const SerialLcdTextEntry entry =
            lcd->text_history[--lcd->text_length];

    lcd->cursor_x = entry.x;
    lcd->cursor_y = entry.y;

    for (uint8_t x = 0; x < entry.clear_width; x++) {
        for (uint8_t y = 0; y < CHARACTER_HEIGHT; y++) {
            set_pixel(
                    lcd,
                    entry.x + x,
                    entry.y + y,
                    false);
        }
    }
}

static bool ascii_digit(uint8_t value) {
    return value >= (uint8_t)'0'
            && value <= (uint8_t)'9';
}

static uint8_t command_parameter_count(uint8_t command) {
    switch (command) {
        case CMD_BITMAP:
        case CMD_CLEAR:
            return 1;

        case CMD_SPRITE_MENU:
            return 2;

        case CMD_POSITION:
            return 4;

        default:
            return 0;
    }
}

static bool command_known(uint8_t command) {
    switch (command) {
        case CMD_BITMAP:
        case CMD_CLEAR:
        case CMD_POSITION:
        case CMD_SPRITE_MENU:
            return true;

        default:
            return false;
    }
}

static void execute_position(SerialLcd *lcd) {
    const uint8_t *parameters = lcd->parameters;

    if (!ascii_digit(parameters[0])
            || !ascii_digit(parameters[1])
            || !ascii_digit(parameters[2])
            || !ascii_digit(parameters[3])) {
        return;
    }

    const uint8_t y =
            (uint8_t)((parameters[0] - (uint8_t)'0') * 10u
                    + parameters[1] - (uint8_t)'0');
    const uint8_t x =
            (uint8_t)((parameters[2] - (uint8_t)'0') * 10u
                    + parameters[3] - (uint8_t)'0');

    if ((uint16_t)y >= DISPLAY_HEIGHT
            || (uint16_t)x >= DISPLAY_WIDTH) {
        return;
    }

    lcd->cursor_y = y;
    lcd->cursor_x = x;
    reset_text_history(lcd);
}

/*
 * Legacy sprite velocity command:
 *
 *     /NVv
 *
 * N selects the sprite command menu, V selects velocity and v is 1..9.
 * The original firmware stores the general-timer interval as 2 * v.
 */
static void execute_sprite_velocity_command(SerialLcd *lcd) {
    const uint8_t subcommand = lcd->parameters[0];
    const uint8_t velocity = lcd->parameters[1];

    if (subcommand != SPRITE_CMD_VELOCITY
            && subcommand != (uint8_t)'v') {
        return;
    }

    if (velocity < (uint8_t)'1'
            || velocity > (uint8_t)'9') {
        return;
    }

    lcd->sprite_interval_ticks =
            (uint8_t)(2u * (velocity - (uint8_t)'0'));
    lcd->sprite_elapsed_ticks = 0;
}

static void execute_command(SerialLcd *lcd) {
    switch (lcd->command) {
        case CMD_CLEAR:
            clear_screen(lcd);
            break;

        case CMD_POSITION:
            execute_position(lcd);
            break;

        case CMD_BITMAP:
            draw_bitmap_digit(lcd, lcd->parameters[0]);
            break;

        case CMD_SPRITE_MENU:
            execute_sprite_velocity_command(lcd);
            break;

        default:
            break;
    }
}

static void process_text_byte(
        SerialLcd *lcd,
        uint8_t value) {
    if (lcd->pending_sprite_marker) {
        lcd->pending_sprite_marker = false;

        if (value == SPRITE_MARKER) {
            place_sprite(lcd);
            return;
        }

        draw_character(lcd, SPRITE_MARKER);
    }

    if (value == SPRITE_MARKER) {
        lcd->pending_sprite_marker = true;
        return;
    }

    if (value == COMMAND_PREFIX) {
        lcd->parser_state = SERIAL_LCD_COMMAND;
        return;
    }

    if (value == '\r') {
        next_line(lcd);
        return;
    }

    if (value == '\b') {
        erase_previous_text_entry(lcd);
        return;
    }

    if (value >= CUSTOM_CHARACTER_FIRST) {
        draw_character(lcd, value);
    }
}

void serial_lcd_init(SerialLcd *lcd) {
    display_framebuffer_clear(&lcd->framebuffer);

    lcd->cursor_x = 0;
    lcd->cursor_y = 0;
    lcd->text_length = 0;
    lcd->sprite_count = 0;
    lcd->sprite_frame = 0;
    lcd->sprite_interval_ticks =
            (uint8_t)(2u * SPRITE_DEFAULT_VELOCITY);
    lcd->sprite_elapsed_ticks = 0;
    lcd->parser_state = SERIAL_LCD_TEXT;
    lcd->command = 0;
    lcd->parameter_count = 0;
    lcd->parameter_expected = 0;
    lcd->pending_sprite_marker = false;
    lcd->dirty = true;
}

void serial_lcd_feed(
        SerialLcd *lcd,
        const uint8_t *data,
        size_t size) {
    for (size_t index = 0; index < size; index++) {
        const uint8_t value = data[index];

        switch (lcd->parser_state) {
            case SERIAL_LCD_TEXT:
                process_text_byte(lcd, value);
                break;

            case SERIAL_LCD_COMMAND:
                lcd->command = value;
                lcd->parameter_count = 0;
                lcd->parameter_expected =
                        command_parameter_count(value);

                if (!command_known(value)) {
                    lcd->parser_state = SERIAL_LCD_TEXT;
                } else {
                    lcd->parser_state = SERIAL_LCD_PARAMETERS;
                }
                break;

            case SERIAL_LCD_PARAMETERS:
                lcd->parameters[lcd->parameter_count++] = value;

                if (lcd->parameter_count
                        == lcd->parameter_expected) {
                    execute_command(lcd);
                    lcd->parser_state = SERIAL_LCD_TEXT;
                }
                break;
        }
    }
}

void serial_lcd_general_timer_tick(SerialLcd *lcd) {
    if (lcd->sprite_count == 0) {
        return;
    }

    lcd->sprite_elapsed_ticks++;

    if (lcd->sprite_elapsed_ticks < lcd->sprite_interval_ticks) {
        return;
    }

    lcd->sprite_elapsed_ticks = 0;
    lcd->sprite_frame =
            (uint8_t)((lcd->sprite_frame + 1u)
                    % CUSTOM_SPRITE_FRAME_COUNT);

    for (uint16_t index = 0; index < lcd->sprite_count; index++) {
        render_sprite(lcd, &lcd->sprites[index]);
    }
}

bool serial_lcd_dirty(const SerialLcd *lcd) {
    return lcd->dirty;
}

void serial_lcd_mark_clean(SerialLcd *lcd) {
    lcd->dirty = false;
}
