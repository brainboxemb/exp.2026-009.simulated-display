#include "matrix_theme.h"

MatrixTheme matrix_theme_default_create(void) {
    const MatrixPixel red =
            matrix_pixel_rgb565_create(255, 0, 0);
    const MatrixPixel dark_gray =
            matrix_pixel_rgb565_create(0x33, 0x33, 0x33);
    const MatrixPixel gray =
            matrix_pixel_rgb565_create(0x80, 0x80, 0x80);

    MatrixTheme theme = {
        .background_color = dark_gray,
        .line_color = gray,
        .header = {
            .font = {MATRIX_FONT_UNSCII, 10},
            .color = red
        },
        .row_number = {
            .font = {MATRIX_FONT_UNSCII, 10},
            .color = red
        },
        .row_time = {
            .font = {MATRIX_FONT_UNSCII, 10},
            .color = red
        },
        .large_value = {
            .font = {MATRIX_FONT_UNSCII, 16},
            .color = red
        },
        .footer = {
            .font = {MATRIX_FONT_UNSCII, 10},
            .color = red
        }
    };

    return theme;
}
