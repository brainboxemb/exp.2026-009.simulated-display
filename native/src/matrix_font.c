#include "matrix_font.h"

#include "matrix_numeric_font.h"
#include "matrix_pixel_font.h"
#include "matrix_vbz_font.h"

const lv_font_t *matrix_font_resolve(
        MatrixFontSpec font) {
    switch (font.family) {
        case MATRIX_FONT_UNSCII:
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

        case MATRIX_FONT_VBZ:
            return font.size == 9
                    ? &matrix_font_vbz_9
                    : NULL;

        default:
            return NULL;
    }
}

const lv_font_t *matrix_large_font_resolve(
        MatrixFontSpec font) {
    if (font.family == MATRIX_FONT_VBZ
            && font.size == 12) {
        return &matrix_font_numeric_12;
    }

    return matrix_font_resolve(font);
}
