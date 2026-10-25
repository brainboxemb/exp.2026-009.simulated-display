#include "matrix_theme.h"

MatrixTheme matrix_theme_default_create(void) {
    const MatrixPixel red =
            matrix_pixel_rgb565_create(255, 0, 0);
    const MatrixPixel dark_gray =
            matrix_pixel_rgb565_create(0x33, 0x33, 0x33);
    const MatrixPixel gray =
            matrix_pixel_rgb565_create(0x80, 0x80, 0x80);
    const MatrixPixel soft_white =
            matrix_pixel_rgb565_create(0xee, 0xee, 0xee);
    const MatrixPixel blue =
            matrix_pixel_rgb565_create(0, 96, 255);
    const MatrixPixel green =
            matrix_pixel_rgb565_create(0, 160, 72);

    MatrixTheme theme = {
        .background_color = dark_gray,
        .line_color = gray,
        .sprite_color = soft_white,
        .exchange_point_color = soft_white,
        .flag_color = soft_white,
        .warning_background_color = blue,
        .warning_text_color = soft_white,
        .warning_strike_color = red,
        .sync_background_color = green,
        .sync_icon_color = soft_white,
        .header = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = red
        },
        .row_number = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = red
        },
        .row_time = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = red
        },
        .large_value = {
            .font = {MATRIX_FONT_VBZ, 14},
            .color = red
        },
        .footer = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = red
        }
    };

    return theme;
}
