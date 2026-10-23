#include "matrix_font.h"

const lv_font_t *matrix_font_resolve(
        MatrixFontSpec font) {
    switch (font.family) {
        case MATRIX_FONT_MONTSERRAT:
            switch (font.size) {
                case 8: return &lv_font_montserrat_8;
                case 10: return &lv_font_montserrat_10;
                case 12: return &lv_font_montserrat_12;
                case 14: return &lv_font_montserrat_14;
                case 16: return &lv_font_montserrat_16;
                case 18: return &lv_font_montserrat_18;
                case 20: return &lv_font_montserrat_20;
                default: return NULL;
            }

        case MATRIX_FONT_UNSCII:
            switch (font.size) {
                case 8: return &lv_font_unscii_8;
                case 16: return &lv_font_unscii_16;
                default: return NULL;
            }

        default:
            return NULL;
    }
}
