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
    const MatrixPixel warm_yellow =
            matrix_pixel_rgb565_create(0xff, 0xcc, 0x66);
    const MatrixPixel lime =
            matrix_pixel_rgb565_create(0, 255, 0);
    const MatrixPixel blue =
            matrix_pixel_rgb565_create(0, 96, 255);
    MatrixTheme theme = {
        .background_color = dark_gray,
        .line_color = gray,
        .sprite_color = warm_yellow,
        .exchange_point_background_color = lime,
        .exchange_point_text_color = soft_white,
        .flag_color = soft_white,
        .warning_background_color = blue,
        .warning_text_color = soft_white,
        .warning_strike_color = red,
        .sync_background_color = lime,
        .sync_icon_color = soft_white,
        .header = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = warm_yellow
        },
        .row_number = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = soft_white
        },
        .row_time = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = soft_white
        },
        .large_value = {
            .font = {MATRIX_FONT_VBZ, 14},
            .color = lime
        },
        .footer = {
            .font = {MATRIX_FONT_VBZ, 9},
            .color = warm_yellow
        }
    };

    return theme;
}
