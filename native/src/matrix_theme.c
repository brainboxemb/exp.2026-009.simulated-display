#include "matrix_theme.h"

MatrixTheme matrix_theme_default_create(void) {
    const MatrixPixel red =
            matrix_pixel_rgb565_create(255, 0, 0);

    MatrixTheme theme = {
        .background_color = 0x0000u,
        .line_color = red,
        .header = {
            .font = {MATRIX_FONT_UNSCII, 8},
            .color = red
        },
        .row_number = {
            .font = {MATRIX_FONT_UNSCII, 8},
            .color = red
        },
        .row_time = {
            .font = {MATRIX_FONT_UNSCII, 8},
            .color = red
        },
        .large_value = {
            .font = {MATRIX_FONT_UNSCII, 16},
            .color = red
        },
        .footer = {
            .font = {MATRIX_FONT_UNSCII, 8},
            .color = red
        }
    };

    return theme;
}
