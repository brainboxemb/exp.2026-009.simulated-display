#include "matrix_font.h"

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
            switch (font.size) {
                case 9:
                    return &matrix_font_vbz_9;
                case 10:
                    return &matrix_font_vbz_10;
                default:
                    return NULL;
            }

        default:
            return NULL;
    }
}
