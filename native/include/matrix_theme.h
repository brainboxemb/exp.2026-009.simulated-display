#ifndef MATRIX_THEME_H
#define MATRIX_THEME_H

#include "matrix_framebuffer.h"

#include <stdint.h>

typedef enum {
    MATRIX_FONT_UNSCII = 0,
    MATRIX_FONT_VBZ = 1
} MatrixFontFamily;

typedef struct {
    MatrixFontFamily family;
    uint8_t size;
} MatrixFontSpec;

typedef struct {
    MatrixFontSpec font;
    MatrixPixel color;
} MatrixTextStyle;

typedef struct {
    MatrixPixel background_color;
    MatrixPixel line_color;
    MatrixPixel sprite_color;
    MatrixPixel exchange_point_color;
    MatrixPixel flag_color;
    MatrixPixel warning_background_color;
    MatrixPixel warning_text_color;
    MatrixPixel warning_strike_color;
    MatrixPixel sync_background_color;
    MatrixPixel sync_icon_color;
    MatrixTextStyle header;
    MatrixTextStyle row_number;
    MatrixTextStyle row_time;
    MatrixTextStyle large_value;
    MatrixTextStyle footer;
} MatrixTheme;

MatrixTheme matrix_theme_default_create(void);

#endif
