#include "matrix_font.h"

#include "matrix_pixel_font.h"

const lv_font_t *matrix_font_resolve(
        MatrixFontSpec font) {
    if (font.family != MATRIX_FONT_UNSCII) {
        return NULL;
    }

    switch (font.size) {
        case 8:
            return &lv_font_unscii_8;

        case 10:
            return &matrix_font_unscii_10;

        case 16:
            return &lv_font_unscii_16;

        default:
            return NULL;
    }
}
