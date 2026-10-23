#ifndef SERIAL_LCD_H
#define SERIAL_LCD_H

#include "display_framebuffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SERIAL_LCD_TEXT,
    SERIAL_LCD_COMMAND,
    SERIAL_LCD_PARAMETERS
} SerialLcdParserState;

typedef struct {
    DisplayFramebuffer framebuffer;
    uint8_t text_origin_x;
    uint8_t text_origin_y;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint16_t text_length;
    SerialLcdParserState parser_state;
    uint8_t command;
    uint8_t parameters[5];
    uint8_t parameter_count;
    uint8_t parameter_expected;
    bool dirty;
} SerialLcd;

void serial_lcd_init(SerialLcd *lcd);
void serial_lcd_feed(SerialLcd *lcd, const uint8_t *data, size_t size);
bool serial_lcd_dirty(const SerialLcd *lcd);
void serial_lcd_mark_clean(SerialLcd *lcd);

#endif
