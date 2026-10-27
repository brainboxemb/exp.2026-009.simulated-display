#include "matrix_status_icon.h"

#include <stddef.h>

static const uint16_t WIFI[MATRIX_STATUS_ICON_HEIGHT] = {
    0x000,
    0x3fe,
    0x3fe,
    0x000,
    0x0f8,
    0x0f8,
    0x000,
    0x060,
    0x060,
    0x000,
    0x000
};

static const uint16_t LINK[MATRIX_STATUS_ICON_HEIGHT] = {
    0x00e,
    0x011,
    0x021,
    0x042,
    0x074,
    0x090,
    0x170,
    0x210,
    0x420,
    0x440,
    0x380
};

static const uint16_t SYNC[MATRIX_STATUS_ICON_HEIGHT] = {
    0x000,
    0x008,
    0x3fc,
    0x3fe,
    0x008,
    0x000,
    0x080,
    0x1fe,
    0x3fe,
    0x080,
    0x000
};

static const uint16_t STRIKE[MATRIX_STATUS_ICON_HEIGHT] = {
    0x600,
    0x300,
    0x180,
    0x0c0,
    0x060,
    0x030,
    0x018,
    0x00c,
    0x006,
    0x003,
    0x003
};

static const uint16_t *glyph(
        MatrixStatusIcon icon) {
    switch (icon) {
        case MATRIX_STATUS_ICON_WIFI:
            return WIFI;
        case MATRIX_STATUS_ICON_LINK:
            return LINK;
        case MATRIX_STATUS_ICON_SYNC:
            return SYNC;
    }

    return NULL;
}

static void fill(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixPixel color) {
    for (uint16_t row = 0u;
            row < MATRIX_STATUS_ICON_HEIGHT;
            row++) {
        for (uint16_t column = 0u;
                column < MATRIX_STATUS_ICON_WIDTH;
                column++) {
            matrix_framebuffer_set(
                    framebuffer,
                    (uint16_t)(x + column),
                    (uint16_t)(y + row),
                    color);
        }
    }
}

static bool progress_pixel_visible(
        MatrixStatusIcon icon,
        uint8_t animation_frame,
        uint16_t column,
        uint16_t row) {
    const uint8_t frame =
            (uint8_t)(animation_frame % 4u);

    if (icon == MATRIX_STATUS_ICON_WIFI) {
        uint16_t first_visible_row = 7u;

        if (frame == 1u || frame == 3u) {
            first_visible_row = 4u;
        } else if (frame == 2u) {
            first_visible_row = 1u;
        }

        return row >= first_visible_row;
    }

    if (icon == MATRIX_STATUS_ICON_LINK) {
        switch (frame) {
            case 0u:
                return column <= 3u;
            case 1u:
                return column <= 6u;
            case 2u:
                return true;
            case 3u:
                return column >= 4u;
        }
    }

    return true;
}

void matrix_status_icon_draw(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixStatusIcon icon,
        MatrixPixel background_color,
        MatrixPixel icon_color,
        bool strike,
        MatrixPixel strike_color) {
    const uint16_t *icon_glyph =
            glyph(icon);

    if (framebuffer == NULL
            || icon_glyph == NULL
            || x + MATRIX_STATUS_ICON_WIDTH
                    > MATRIX_DISPLAY_WIDTH
            || y + MATRIX_STATUS_ICON_HEIGHT
                    > MATRIX_DISPLAY_HEIGHT) {
        return;
    }

    fill(
            framebuffer,
            x,
            y,
            background_color);

    for (uint16_t row = 0u;
            row < MATRIX_STATUS_ICON_HEIGHT;
            row++) {
        for (uint16_t column = 0u;
                column < MATRIX_STATUS_ICON_WIDTH;
                column++) {
            const uint16_t mask =
                    (uint16_t)(
                            1u
                            << (MATRIX_STATUS_ICON_WIDTH
                                    - 1u
                                    - column));

            if ((icon_glyph[row] & mask) != 0u) {
                matrix_framebuffer_set(
                        framebuffer,
                        (uint16_t)(x + column),
                        (uint16_t)(y + row),
                        icon_color);
            }

            if (strike
                    && (STRIKE[row] & mask) != 0u) {
                matrix_framebuffer_set(
                        framebuffer,
                        (uint16_t)(x + column),
                        (uint16_t)(y + row),
                        strike_color);
            }
        }
    }
}

void matrix_status_icon_draw_progress(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixStatusIcon icon,
        uint8_t animation_frame,
        MatrixPixel background_color,
        MatrixPixel icon_color) {
    const uint16_t *icon_glyph =
            glyph(icon);

    if (framebuffer == NULL
            || icon_glyph == NULL
            || x + MATRIX_STATUS_ICON_WIDTH
                    > MATRIX_DISPLAY_WIDTH
            || y + MATRIX_STATUS_ICON_HEIGHT
                    > MATRIX_DISPLAY_HEIGHT) {
        return;
    }

    fill(
            framebuffer,
            x,
            y,
            background_color);

    for (uint16_t row = 0u;
            row < MATRIX_STATUS_ICON_HEIGHT;
            row++) {
        for (uint16_t column = 0u;
                column < MATRIX_STATUS_ICON_WIDTH;
                column++) {
            const uint16_t mask =
                    (uint16_t)(
                            1u
                            << (MATRIX_STATUS_ICON_WIDTH
                                    - 1u
                                    - column));

            if ((icon_glyph[row] & mask) != 0u
                    && progress_pixel_visible(
                            icon,
                            animation_frame,
                            column,
                            row)) {
                matrix_framebuffer_set(
                        framebuffer,
                        (uint16_t)(x + column),
                        (uint16_t)(y + row),
                        icon_color);
            }
        }
    }
}
