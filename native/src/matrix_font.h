#ifndef MATRIX_FONT_H
#define MATRIX_FONT_H

#include "matrix_theme.h"
#include "lvgl.h"

const lv_font_t *matrix_font_resolve(
        MatrixFontSpec font);

const lv_font_t *matrix_large_font_resolve(
        MatrixFontSpec font);

#endif
