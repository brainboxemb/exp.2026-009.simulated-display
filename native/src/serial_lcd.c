#include "serial_lcd.h"

#include "custom_character_map.h"

#include <stdlib.h>

#define COMMAND_PREFIX 0x7cu
#define SPRITE_MARKER 0xfeu

#define CMD_CLEAR 0x00u
#define CMD_BACKLIGHT 0x02u
#define CMD_CIRCLE 0x03u
#define CMD_DEMO 0x04u
#define CMD_ERASE_BLOCK 0x05u
#define CMD_BAUD 0x07u
#define CMD_LINE 0x0cu
#define CMD_BOX 0x0fu
#define CMD_PIXEL 0x10u
#define CMD_REVERSE 0x12u
#define CMD_SPLASH 0x13u
#define CMD_SET_X 0x18u
#define CMD_SET_Y 0x19u

static void set_pixel(SerialLcd *lcd, int x, int y, bool on) {
    if (x < 0 || y < 0 || x >= (int)DISPLAY_WIDTH || y >= (int)DISPLAY_HEIGHT) {
        return;
    }

    if (display_framebuffer_get(&lcd->framebuffer, (uint16_t)x, (uint16_t)y) == on) {
        return;
    }

    display_framebuffer_set(&lcd->framebuffer, (uint16_t)x, (uint16_t)y, on);
    lcd->dirty = true;
}

static void reset_text_history(SerialLcd *lcd) {
    lcd->text_length = 0;
}

static void clear_screen(SerialLcd *lcd) {
    display_framebuffer_clear(&lcd->framebuffer);
    lcd->cursor_x = lcd->text_origin_x;
    lcd->cursor_y = lcd->text_origin_y;
    reset_text_history(lcd);
    lcd->dirty = true;
}

static void draw_line(SerialLcd *lcd, int x0, int y0, int x1, int y1, bool on) {
    const int dx = abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    for (;;) {
        set_pixel(lcd, x0, y0, on);
        if (x0 == x1 && y0 == y1) {
            return;
        }

        const int twice_error = 2 * error;
        if (twice_error >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

static void draw_box(SerialLcd *lcd, int x0, int y0, int x1, int y1, bool on) {
    draw_line(lcd, x0, y0, x1, y0, on);
    draw_line(lcd, x1, y0, x1, y1, on);
    draw_line(lcd, x1, y1, x0, y1, on);
    draw_line(lcd, x0, y1, x0, y0, on);
}

static void draw_circle(SerialLcd *lcd, int cx, int cy, int radius, bool on) {
    int x = radius;
    int y = 0;
    int error = 1 - radius;

    while (x >= y) {
        set_pixel(lcd, cx + x, cy + y, on);
        set_pixel(lcd, cx + y, cy + x, on);
        set_pixel(lcd, cx - y, cy + x, on);
        set_pixel(lcd, cx - x, cy + y, on);
        set_pixel(lcd, cx - x, cy - y, on);
        set_pixel(lcd, cx - y, cy - x, on);
        set_pixel(lcd, cx + y, cy - x, on);
        set_pixel(lcd, cx + x, cy - y, on);

        y++;
        if (error < 0) {
            error += 2 * y + 1;
        } else {
            x--;
            error += 2 * (y - x + 1);
        }
    }
}

static void erase_block(SerialLcd *lcd, int x0, int y0, int x1, int y1) {
    if (x0 > x1) {
        const int swap = x0;
        x0 = x1;
        x1 = swap;
    }
    if (y0 > y1) {
        const int swap = y0;
        y0 = y1;
        y1 = swap;
    }

    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            set_pixel(lcd, x, y, false);
        }
    }
}

static void next_line(SerialLcd *lcd) {
    lcd->cursor_x = lcd->text_origin_x;

    const uint16_t next_y = (uint16_t)lcd->cursor_y + 8u;
    if (next_y + 7u >= DISPLAY_HEIGHT) {
        lcd->cursor_y = lcd->text_origin_y;
        reset_text_history(lcd);
        return;
    }

    lcd->cursor_y = (uint8_t)next_y;
}

static void ensure_horizontal_space(SerialLcd *lcd, uint8_t advance) {
    if ((uint16_t)lcd->cursor_x + advance > DISPLAY_WIDTH) {
        next_line(lcd);
    }
}

static void draw_columns(
        SerialLcd *lcd,
        const uint8_t columns[CUSTOM_CHARACTER_COLUMNS]) {
    for (uint8_t x = 0; x < CUSTOM_CHARACTER_COLUMNS; x++) {
        for (uint8_t y = 0; y < 8u; y++) {
            set_pixel(
                    lcd,
                    lcd->cursor_x + x,
                    lcd->cursor_y + y,
                    (columns[x] & (uint8_t)(1u << y)) != 0);
        }
    }
}

static void record_text_entry(SerialLcd *lcd, uint8_t clear_width) {
    if (lcd->text_length >= SERIAL_LCD_TEXT_HISTORY_CAPACITY) {
        reset_text_history(lcd);
    }

    SerialLcdTextEntry *entry = &lcd->text_history[lcd->text_length++];
    entry->x = lcd->cursor_x;
    entry->y = lcd->cursor_y;
    entry->clear_width = clear_width;
}

static void advance_cursor(SerialLcd *lcd, uint8_t advance) {
    lcd->cursor_x = (uint8_t)(lcd->cursor_x + advance);
    if (lcd->cursor_x >= DISPLAY_WIDTH) {
        next_line(lcd);
    }
}

static void draw_character(SerialLcd *lcd, uint8_t code) {
    CustomCharacterGlyph glyph;
    if (!custom_character_map_lookup(code, &glyph)) {
        return;
    }

    ensure_horizontal_space(lcd, glyph.advance);
    draw_columns(lcd, glyph.columns);
    record_text_entry(lcd, CUSTOM_CHARACTER_COLUMNS);
    advance_cursor(lcd, glyph.advance);
}

static void draw_sprite(SerialLcd *lcd) {
    ensure_horizontal_space(lcd, CUSTOM_SPRITE_WIDTH);
    draw_columns(lcd, custom_character_map_sprite_frame(lcd->sprite_frame));
    record_text_entry(lcd, CUSTOM_CHARACTER_COLUMNS);
    advance_cursor(lcd, CUSTOM_SPRITE_WIDTH);

    lcd->sprite_frame =
            (uint8_t)((lcd->sprite_frame + 1u) % CUSTOM_SPRITE_FRAME_COUNT);
}

static void erase_previous_text_entry(SerialLcd *lcd) {
    if (lcd->text_length == 0) {
        return;
    }

    const SerialLcdTextEntry entry = lcd->text_history[--lcd->text_length];
    lcd->cursor_x = entry.x;
    lcd->cursor_y = entry.y;

    for (uint8_t x = 0; x < entry.clear_width; x++) {
        for (uint8_t y = 0; y < 8u; y++) {
            set_pixel(lcd, entry.x + x, entry.y + y, false);
        }
    }
}

static void process_text_byte(SerialLcd *lcd, uint8_t value) {
    if (lcd->pending_sprite_marker) {
        lcd->pending_sprite_marker = false;

        if (value == SPRITE_MARKER) {
            draw_sprite(lcd);
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

static uint8_t command_parameter_count(uint8_t command) {
    switch (command) {
        case CMD_CLEAR:
        case CMD_DEMO:
        case CMD_REVERSE:
        case CMD_SPLASH:
            return 0;
        case CMD_BACKLIGHT:
        case CMD_BAUD:
        case CMD_SET_X:
        case CMD_SET_Y:
            return 1;
        case CMD_PIXEL:
            return 3;
        case CMD_CIRCLE:
        case CMD_ERASE_BLOCK:
            return 4;
        case CMD_LINE:
        case CMD_BOX:
            return 5;
        default:
            return 0;
    }
}

static bool known_command(uint8_t command) {
    switch (command) {
        case CMD_CLEAR:
        case CMD_BACKLIGHT:
        case CMD_CIRCLE:
        case CMD_DEMO:
        case CMD_ERASE_BLOCK:
        case CMD_BAUD:
        case CMD_LINE:
        case CMD_BOX:
        case CMD_PIXEL:
        case CMD_REVERSE:
        case CMD_SPLASH:
        case CMD_SET_X:
        case CMD_SET_Y:
            return true;
        default:
            return false;
    }
}

static void execute_command(SerialLcd *lcd) {
    const uint8_t *parameters = lcd->parameters;

    switch (lcd->command) {
        case CMD_CLEAR:
            clear_screen(lcd);
            break;
        case CMD_SET_X:
            if ((uint16_t)parameters[0] < DISPLAY_WIDTH) {
                lcd->text_origin_x = parameters[0];
                lcd->cursor_x = parameters[0];
                reset_text_history(lcd);
            }
            break;
        case CMD_SET_Y:
            if ((uint16_t)parameters[0] + 7u < DISPLAY_HEIGHT) {
                lcd->text_origin_y = parameters[0];
                lcd->cursor_y = parameters[0];
                reset_text_history(lcd);
            }
            break;
        case CMD_PIXEL:
            set_pixel(lcd, parameters[0], parameters[1], parameters[2] != 0);
            break;
        case CMD_LINE:
            draw_line(
                    lcd,
                    parameters[0],
                    parameters[1],
                    parameters[2],
                    parameters[3],
                    parameters[4] != 0);
            break;
        case CMD_CIRCLE:
            draw_circle(
                    lcd,
                    parameters[0],
                    parameters[1],
                    parameters[2],
                    parameters[3] != 0);
            break;
        case CMD_BOX:
            draw_box(
                    lcd,
                    parameters[0],
                    parameters[1],
                    parameters[2],
                    parameters[3],
                    parameters[4] != 0);
            break;
        case CMD_ERASE_BLOCK:
            erase_block(
                    lcd,
                    parameters[0],
                    parameters[1],
                    parameters[2],
                    parameters[3]);
            break;
        default:
            break;
    }
}

void serial_lcd_init(SerialLcd *lcd) {
    display_framebuffer_clear(&lcd->framebuffer);

    lcd->text_origin_x = 0;
    lcd->text_origin_y = 0;
    lcd->cursor_x = 0;
    lcd->cursor_y = 0;
    lcd->text_length = 0;
    lcd->parser_state = SERIAL_LCD_TEXT;
    lcd->command = 0;
    lcd->parameter_count = 0;
    lcd->parameter_expected = 0;
    lcd->sprite_frame = 0;
    lcd->pending_sprite_marker = false;
    lcd->dirty = true;
}

void serial_lcd_feed(SerialLcd *lcd, const uint8_t *data, size_t size) {
    for (size_t index = 0; index < size; index++) {
        const uint8_t value = data[index];

        switch (lcd->parser_state) {
            case SERIAL_LCD_TEXT:
                process_text_byte(lcd, value);
                break;
            case SERIAL_LCD_COMMAND:
                lcd->command = value;
                lcd->parameter_count = 0;
                lcd->parameter_expected = command_parameter_count(value);

                if (!known_command(value)) {
                    lcd->parser_state = SERIAL_LCD_TEXT;
                } else if (lcd->parameter_expected == 0) {
                    execute_command(lcd);
                    lcd->parser_state = SERIAL_LCD_TEXT;
                } else {
                    lcd->parser_state = SERIAL_LCD_PARAMETERS;
                }
                break;
            case SERIAL_LCD_PARAMETERS:
                lcd->parameters[lcd->parameter_count++] = value;

                if (lcd->parameter_count == lcd->parameter_expected) {
                    execute_command(lcd);
                    lcd->parser_state = SERIAL_LCD_TEXT;
                }
                break;
        }
    }
}

bool serial_lcd_dirty(const SerialLcd *lcd) {
    return lcd->dirty;
}

void serial_lcd_mark_clean(SerialLcd *lcd) {
    lcd->dirty = false;
}
