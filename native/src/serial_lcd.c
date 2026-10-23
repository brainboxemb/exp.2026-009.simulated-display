#include "serial_lcd.h"

#include "font5x7.h"

#include <stdlib.h>

#define COMMAND_PREFIX 0x7cu
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
    if (x < 0 || y < 0 || x >= (int)DISPLAY_WIDTH || y >= (int)DISPLAY_HEIGHT) return;
    if (display_framebuffer_get(&lcd->framebuffer, (uint16_t)x, (uint16_t)y) != on) {
        display_framebuffer_set(&lcd->framebuffer, (uint16_t)x, (uint16_t)y, on);
        lcd->dirty = true;
    }
}

static void clear_screen(SerialLcd *lcd) {
    display_framebuffer_clear(&lcd->framebuffer);
    lcd->cursor_x = lcd->text_origin_x;
    lcd->cursor_y = lcd->text_origin_y;
    lcd->text_length = 0;
    lcd->dirty = true;
}

static void draw_line(SerialLcd *lcd, int x0, int y0, int x1, int y1, bool on) {
    const int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    const int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    for (;;) {
        set_pixel(lcd, x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        const int twice_error = 2 * error;
        if (twice_error >= dy) { error += dy; x0 += sx; }
        if (twice_error <= dx) { error += dx; y0 += sy; }
    }
}

static void draw_box(SerialLcd *lcd, int x0, int y0, int x1, int y1, bool on) {
    draw_line(lcd, x0, y0, x1, y0, on);
    draw_line(lcd, x1, y0, x1, y1, on);
    draw_line(lcd, x1, y1, x0, y1, on);
    draw_line(lcd, x0, y1, x0, y0, on);
}

static void draw_circle(SerialLcd *lcd, int cx, int cy, int radius, bool on) {
    int x = radius, y = 0, error = 1 - radius;
    while (x >= y) {
        set_pixel(lcd, cx + x, cy + y, on); set_pixel(lcd, cx + y, cy + x, on);
        set_pixel(lcd, cx - y, cy + x, on); set_pixel(lcd, cx - x, cy + y, on);
        set_pixel(lcd, cx - x, cy - y, on); set_pixel(lcd, cx - y, cy - x, on);
        set_pixel(lcd, cx + y, cy - x, on); set_pixel(lcd, cx + x, cy - y, on);
        y++;
        if (error < 0) error += 2 * y + 1;
        else { x--; error += 2 * (y - x + 1); }
    }
}

static void erase_block(SerialLcd *lcd, int x0, int y0, int x1, int y1) {
    if (x0 > x1) { const int swap = x0; x0 = x1; x1 = swap; }
    if (y0 > y1) { const int swap = y0; y0 = y1; y1 = swap; }
    for (int y = y0; y <= y1; y++) for (int x = x0; x <= x1; x++) set_pixel(lcd, x, y, false);
}

static void next_line(SerialLcd *lcd) {
    lcd->cursor_x = lcd->text_origin_x;
    lcd->cursor_y = (uint8_t)(lcd->cursor_y + 8u);
    if ((uint16_t)lcd->cursor_y + 7u >= DISPLAY_HEIGHT) lcd->cursor_y = lcd->text_origin_y;
}

static void draw_character(SerialLcd *lcd, uint8_t ascii) {
    if (ascii == '\r') { next_line(lcd); return; }
    if (ascii == '\b') {
        if (lcd->text_length == 0) return;
        lcd->text_length--;
        if (lcd->cursor_x == lcd->text_origin_x) {
            if (lcd->cursor_y == lcd->text_origin_y) {
                uint8_t y = lcd->text_origin_y;
                while ((uint16_t)y + 15u < DISPLAY_HEIGHT) y = (uint8_t)(y + 8u);
                lcd->cursor_y = y;
            } else lcd->cursor_y = (uint8_t)(lcd->cursor_y - 8u);
            uint8_t x = lcd->text_origin_x;
            while ((uint16_t)x + 11u < DISPLAY_WIDTH) x = (uint8_t)(x + 6u);
            lcd->cursor_x = x;
        } else lcd->cursor_x = (uint8_t)(lcd->cursor_x - 6u);
        for (uint8_t x = 0; x < 5; x++) for (uint8_t y = 0; y < 7; y++)
            set_pixel(lcd, lcd->cursor_x + x, lcd->cursor_y + y, false);
        return;
    }

    uint8_t columns[5];
    if (!font5x7_columns(ascii, columns)) return;
    if ((uint16_t)lcd->cursor_x + 5u > DISPLAY_WIDTH) next_line(lcd);
    for (uint8_t x = 0; x < 5; x++) for (uint8_t y = 0; y < 7; y++)
        set_pixel(lcd, lcd->cursor_x + x, lcd->cursor_y + y, (columns[x] & (1u << y)) != 0);
    lcd->text_length++;
    lcd->cursor_x = (uint8_t)(lcd->cursor_x + 6u);
    if ((uint16_t)lcd->cursor_x + 5u > DISPLAY_WIDTH) next_line(lcd);
}

static uint8_t command_parameter_count(uint8_t command) {
    switch (command) {
        case CMD_CLEAR: case CMD_DEMO: case CMD_REVERSE: case CMD_SPLASH: return 0;
        case CMD_BACKLIGHT: case CMD_BAUD: case CMD_SET_X: case CMD_SET_Y: return 1;
        case CMD_PIXEL: return 3;
        case CMD_CIRCLE: case CMD_ERASE_BLOCK: return 4;
        case CMD_LINE: case CMD_BOX: return 5;
        default: return 0;
    }
}

static bool known_command(uint8_t command) {
    switch (command) {
        case CMD_CLEAR: case CMD_BACKLIGHT: case CMD_CIRCLE: case CMD_DEMO:
        case CMD_ERASE_BLOCK: case CMD_BAUD: case CMD_LINE: case CMD_BOX:
        case CMD_PIXEL: case CMD_REVERSE: case CMD_SPLASH: case CMD_SET_X: case CMD_SET_Y: return true;
        default: return false;
    }
}

static void execute_command(SerialLcd *lcd) {
    const uint8_t *p = lcd->parameters;
    switch (lcd->command) {
        case CMD_CLEAR: clear_screen(lcd); break;
        case CMD_SET_X:
            if ((uint16_t)p[0] + 6u <= DISPLAY_WIDTH) {
                lcd->text_origin_x = p[0]; lcd->cursor_x = p[0]; lcd->text_length = 0;
            }
            break;
        case CMD_SET_Y:
            if ((uint16_t)p[0] + 7u < DISPLAY_HEIGHT) {
                lcd->text_origin_y = p[0]; lcd->cursor_y = p[0]; lcd->text_length = 0;
            }
            break;
        case CMD_PIXEL: set_pixel(lcd, p[0], p[1], p[2] != 0); break;
        case CMD_LINE: draw_line(lcd, p[0], p[1], p[2], p[3], p[4] != 0); break;
        case CMD_CIRCLE: draw_circle(lcd, p[0], p[1], p[2], p[3] != 0); break;
        case CMD_BOX: draw_box(lcd, p[0], p[1], p[2], p[3], p[4] != 0); break;
        case CMD_ERASE_BLOCK: erase_block(lcd, p[0], p[1], p[2], p[3]); break;
        default: break;
    }
}

void serial_lcd_init(SerialLcd *lcd) {
    display_framebuffer_clear(&lcd->framebuffer);
    lcd->text_origin_x = lcd->text_origin_y = lcd->cursor_x = lcd->cursor_y = 0;
    lcd->text_length = 0;
    lcd->parser_state = SERIAL_LCD_TEXT;
    lcd->command = lcd->parameter_count = lcd->parameter_expected = 0;
    lcd->dirty = true;
}

void serial_lcd_feed(SerialLcd *lcd, const uint8_t *data, size_t size) {
    for (size_t index = 0; index < size; index++) {
        const uint8_t value = data[index];
        switch (lcd->parser_state) {
            case SERIAL_LCD_TEXT:
                if (value == COMMAND_PREFIX) lcd->parser_state = SERIAL_LCD_COMMAND;
                else if ((value >= 0x20u && value <= 0x7eu) || value == '\r' || value == '\b') draw_character(lcd, value);
                break;
            case SERIAL_LCD_COMMAND:
                lcd->command = value;
                lcd->parameter_count = 0;
                lcd->parameter_expected = command_parameter_count(value);
                if (!known_command(value)) lcd->parser_state = SERIAL_LCD_TEXT;
                else if (lcd->parameter_expected == 0) { execute_command(lcd); lcd->parser_state = SERIAL_LCD_TEXT; }
                else lcd->parser_state = SERIAL_LCD_PARAMETERS;
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

bool serial_lcd_dirty(const SerialLcd *lcd) { return lcd->dirty; }
void serial_lcd_mark_clean(SerialLcd *lcd) { lcd->dirty = false; }
