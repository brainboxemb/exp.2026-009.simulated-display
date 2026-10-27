#include "matrix_startup_screen.h"

#include "matrix_status_icon.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define GLYPH_WIDTH 5u
#define GLYPH_HEIGHT 7u
#define GLYPH_SCALE 4u
#define GLYPH_GAP 4u
#define GLYPH_STRIDE     (GLYPH_WIDTH * GLYPH_SCALE + GLYPH_GAP)

#define INFO_GLYPH_HEIGHT 5u
#define INFO_GLYPH_WIDTH 3u
#define INFO_GLYPH_GAP 1u
#define INFO_RIGHT_MARGIN 1u
#define INFO_ID_Y 13u
#define INFO_SSID_Y 20u
#define VERSION_BOTTOM_MARGIN 1u

static const uint8_t GLYPH_R[GLYPH_HEIGHT] = {
    0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11
};
static const uint8_t GLYPH_E[GLYPH_HEIGHT] = {
    0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f
};
static const uint8_t GLYPH_L[GLYPH_HEIGHT] = {
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f
};
static const uint8_t GLYPH_A[GLYPH_HEIGHT] = {
    0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11
};
static const uint8_t GLYPH_Y[GLYPH_HEIGHT] = {
    0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04
};
static const uint8_t GLYPH_C[GLYPH_HEIGHT] = {
    0x0f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0f
};

static const uint8_t ROW_SLANT[GLYPH_HEIGHT] = {
    6u, 6u, 4u, 4u, 2u, 2u, 0u
};

static const uint8_t INFO_DIGITS[10][INFO_GLYPH_HEIGHT] = {
    {0x7, 0x5, 0x5, 0x5, 0x7},
    {0x2, 0x6, 0x2, 0x2, 0x7},
    {0x6, 0x1, 0x2, 0x4, 0x7},
    {0x6, 0x1, 0x2, 0x1, 0x6},
    {0x5, 0x5, 0x7, 0x1, 0x1},
    {0x7, 0x4, 0x6, 0x1, 0x6},
    {0x3, 0x4, 0x6, 0x5, 0x2},
    {0x7, 0x1, 0x2, 0x2, 0x2},
    {0x2, 0x5, 0x2, 0x5, 0x2},
    {0x2, 0x5, 0x3, 0x1, 0x6}
};

static const uint8_t INFO_LETTERS[26][INFO_GLYPH_HEIGHT] = {
    {0x2, 0x5, 0x7, 0x5, 0x5}, /* A */
    {0x6, 0x5, 0x6, 0x5, 0x6}, /* B */
    {0x3, 0x4, 0x4, 0x4, 0x3}, /* C */
    {0x6, 0x5, 0x5, 0x5, 0x6}, /* D */
    {0x7, 0x4, 0x6, 0x4, 0x7}, /* E */
    {0x7, 0x4, 0x6, 0x4, 0x4}, /* F */
    {0x3, 0x4, 0x5, 0x5, 0x3}, /* G */
    {0x5, 0x5, 0x7, 0x5, 0x5}, /* H */
    {0x7, 0x2, 0x2, 0x2, 0x7}, /* I */
    {0x1, 0x1, 0x1, 0x5, 0x2}, /* J */
    {0x5, 0x5, 0x6, 0x5, 0x5}, /* K */
    {0x4, 0x4, 0x4, 0x4, 0x7}, /* L */
    {0x5, 0x7, 0x7, 0x5, 0x5}, /* M */
    {0x5, 0x7, 0x7, 0x7, 0x5}, /* N */
    {0x2, 0x5, 0x5, 0x5, 0x2}, /* O */
    {0x6, 0x5, 0x6, 0x4, 0x4}, /* P */
    {0x2, 0x5, 0x5, 0x3, 0x1}, /* Q */
    {0x6, 0x5, 0x6, 0x5, 0x5}, /* R */
    {0x3, 0x4, 0x2, 0x1, 0x6}, /* S */
    {0x7, 0x2, 0x2, 0x2, 0x2}, /* T */
    {0x5, 0x5, 0x5, 0x5, 0x7}, /* U */
    {0x5, 0x5, 0x5, 0x5, 0x2}, /* V */
    {0x5, 0x5, 0x7, 0x7, 0x5}, /* W */
    {0x5, 0x5, 0x2, 0x5, 0x5}, /* X */
    {0x5, 0x5, 0x2, 0x2, 0x2}, /* Y */
    {0x7, 0x1, 0x2, 0x4, 0x7}  /* Z */
};

static const uint8_t INFO_V[INFO_GLYPH_HEIGHT] = {
    0x5, 0x5, 0x5, 0x5, 0x2
};

static const uint8_t *glyph_rows(char character) {
    switch (character) {
        case 'R':
            return GLYPH_R;
        case 'E':
            return GLYPH_E;
        case 'L':
            return GLYPH_L;
        case 'A':
            return GLYPH_A;
        case 'Y':
            return GLYPH_Y;
        case 'C':
            return GLYPH_C;
        default:
            return NULL;
    }
}

static void set_pixel(
        MatrixFramebuffer *framebuffer,
        int32_t x,
        int32_t y,
        MatrixPixel color) {
    if (x < 0
            || y < 0
            || x >= (int32_t)MATRIX_DISPLAY_WIDTH
            || y >= (int32_t)MATRIX_DISPLAY_HEIGHT) {
        return;
    }

    matrix_framebuffer_set(
            framebuffer,
            (uint16_t)x,
            (uint16_t)y,
            color);
}

static void draw_line(
        MatrixFramebuffer *framebuffer,
        int32_t x0,
        int32_t y0,
        int32_t x1,
        int32_t y1,
        MatrixPixel color) {
    const int32_t dx =
            x1 >= x0
                    ? x1 - x0
                    : x0 - x1;
    const int32_t sx =
            x0 < x1 ? 1 : -1;
    const int32_t dy =
            -(y1 >= y0
                    ? y1 - y0
                    : y0 - y1);
    const int32_t sy =
            y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;

    for (;;) {
        set_pixel(
                framebuffer,
                x0,
                y0,
                color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int32_t doubled =
                2 * error;

        if (doubled >= dy) {
            error += dy;
            x0 += sx;
        }

        if (doubled <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

static void draw_streak(
        MatrixFramebuffer *framebuffer,
        int32_t x0,
        int32_t y0,
        int32_t x1,
        int32_t y1,
        uint8_t thickness,
        MatrixPixel color) {
    for (uint8_t offset = 0u;
            offset < thickness;
            offset++) {
        draw_line(
                framebuffer,
                x0,
                y0 + offset,
                x1,
                y1 + offset,
                color);
    }
}

static uint8_t info_character_width(
        char character) {
    if (character == '.'
            || character == ':') {
        return 1u;
    }

    if (character == ' ') {
        return 2u;
    }

    return INFO_GLYPH_WIDTH;
}

static const uint8_t *info_rows(
        char character) {
    if (character == 'v') {
        return INFO_V;
    }

    if (character >= '0'
            && character <= '9') {
        return INFO_DIGITS[
                (uint8_t)(character - '0')];
    }

    if (character >= 'A'
            && character <= 'Z') {
        return INFO_LETTERS[
                (uint8_t)(character - 'A')];
    }

    return NULL;
}

static size_t info_text_width(
        const char *text) {
    if (text == NULL
            || text[0] == '\0') {
        return 0u;
    }

    size_t width = 0u;

    for (size_t index = 0u;
            text[index] != '\0';
            index++) {
        width += info_character_width(
                text[index]);

        if (text[index + 1u] != '\0') {
            width += INFO_GLYPH_GAP;
        }
    }

    return width;
}

static bool draw_info_text_right(
        MatrixFramebuffer *framebuffer,
        const char *text,
        uint16_t y,
        MatrixPixel color) {
    const size_t width =
            info_text_width(text);

    if (width == 0u
            || width
                    > MATRIX_DISPLAY_WIDTH
                            - INFO_RIGHT_MARGIN
            || y + INFO_GLYPH_HEIGHT
                    > MATRIX_DISPLAY_HEIGHT) {
        return false;
    }

    int32_t x =
            (int32_t)(
                    MATRIX_DISPLAY_WIDTH
                    - INFO_RIGHT_MARGIN
                    - width);

    for (size_t index = 0u;
            text[index] != '\0';
            index++) {
        const char character =
                text[index];

        if (character == '.') {
            set_pixel(
                    framebuffer,
                    x,
                    y + INFO_GLYPH_HEIGHT - 1u,
                    color);
        } else if (character == ':') {
            set_pixel(
                    framebuffer,
                    x,
                    y + 1u,
                    color);
            set_pixel(
                    framebuffer,
                    x,
                    y + 3u,
                    color);
        } else if (character != ' ') {
            const uint8_t *rows =
                    info_rows(character);

            if (rows == NULL) {
                return false;
            }

            for (uint8_t row = 0u;
                    row < INFO_GLYPH_HEIGHT;
                    row++) {
                for (uint8_t column = 0u;
                        column < INFO_GLYPH_WIDTH;
                        column++) {
                    const uint8_t mask =
                            (uint8_t)(
                                    1u
                                    << (INFO_GLYPH_WIDTH
                                            - 1u
                                            - column));

                    if ((rows[row] & mask) != 0u) {
                        set_pixel(
                                framebuffer,
                                x + column,
                                y + row,
                                color);
                    }
                }
            }
        }

        x +=
                info_character_width(character)
                + INFO_GLYPH_GAP;
    }

    return true;
}

static bool draw_startup_info(
        MatrixFramebuffer *framebuffer,
        const MatrixStartupInfo *info,
        MatrixPixel color) {
    if (info == NULL
            || info->version == NULL
            || info_rows(info->display_id) == NULL
            || info_rows(info->ssid_id) == NULL) {
        return false;
    }

    char id_text[] = {
        'I', 'D', ':', ' ',
        info->display_id,
        '\0'
    };
    char ssid_text[] = {
        'S', 'S', 'I', 'D', ':', ' ',
        info->ssid_id,
        '\0'
    };
    const uint16_t version_y =
            (uint16_t)(
                    MATRIX_DISPLAY_HEIGHT
                    - VERSION_BOTTOM_MARGIN
                    - INFO_GLYPH_HEIGHT);

    return draw_info_text_right(
            framebuffer,
            id_text,
            INFO_ID_Y,
            color)
            && draw_info_text_right(
                    framebuffer,
                    ssid_text,
                    INFO_SSID_Y,
                    color)
            && draw_info_text_right(
                    framebuffer,
                    info->version,
                    version_y,
                    color);
}

static void draw_block(
        MatrixFramebuffer *framebuffer,
        int32_t x,
        int32_t y,
        MatrixPixel color) {
    for (uint8_t row = 0u;
            row < GLYPH_SCALE;
            row++) {
        for (uint8_t column = 0u;
                column < GLYPH_SCALE;
                column++) {
            set_pixel(
                    framebuffer,
                    x + column,
                    y + row,
                    color);
        }
    }
}

static bool draw_glyph(
        MatrixFramebuffer *framebuffer,
        char character,
        int32_t x,
        int32_t y,
        MatrixPixel color) {
    const uint8_t *rows =
            glyph_rows(character);

    if (rows == NULL) {
        return false;
    }

    for (uint8_t row = 0u;
            row < GLYPH_HEIGHT;
            row++) {
        for (uint8_t column = 0u;
                column < GLYPH_WIDTH;
                column++) {
            const uint8_t mask =
                    (uint8_t)(
                            1u
                            << (GLYPH_WIDTH
                                    - 1u
                                    - column));

            if ((rows[row] & mask) == 0u) {
                continue;
            }

            draw_block(
                    framebuffer,
                    x
                            + ROW_SLANT[row]
                            + column * GLYPH_SCALE,
                    y + row * GLYPH_SCALE,
                    color);
        }
    }

    return true;
}

static bool draw_word(
        MatrixFramebuffer *framebuffer,
        const char *word,
        int32_t x,
        int32_t y,
        MatrixPixel color) {
    if (word == NULL) {
        return false;
    }

    for (size_t index = 0u;
            word[index] != '\0';
            index++) {
        if (!draw_glyph(
                framebuffer,
                word[index],
                x + (int32_t)index * GLYPH_STRIDE,
                y,
                color)) {
            return false;
        }
    }

    return true;
}

static bool draw_startup_base(
        MatrixFramebuffer *framebuffer,
        const MatrixStartupInfo *info) {
    if (framebuffer == NULL
            || info == NULL
            || info->version == NULL) {
        return false;
    }

    const MatrixPixel black =
            matrix_pixel_rgb565_create(
                    0,
                    0,
                    0);
    const MatrixPixel lime =
            matrix_pixel_rgb565_create(
                    205,
                    255,
                    0);
    const MatrixPixel yellow =
            matrix_pixel_rgb565_create(
                    255,
                    216,
                    0);
    const MatrixPixel cyan =
            matrix_pixel_rgb565_create(
                    0,
                    220,
                    255);
    const MatrixPixel blue =
            matrix_pixel_rgb565_create(
                    0,
                    80,
                    255);
    const MatrixPixel magenta =
            matrix_pixel_rgb565_create(
                    255,
                    0,
                    120);
    const MatrixPixel red =
            matrix_pixel_rgb565_create(
                    255,
                    32,
                    48);
    const MatrixPixel version_color =
            matrix_pixel_rgb565_create(
                    128,
                    128,
                    128);

    matrix_framebuffer_clear(framebuffer);

    /*
     * Short opposing color streaks echo the supplied race-logo reference
     * without copying its artwork. Draw them first so the wordmark remains
     * the visual foreground.
     */
    draw_streak(
            framebuffer,
            0,
            12,
            40,
            4,
            2u,
            cyan);
    draw_streak(
            framebuffer,
            0,
            18,
            34,
            12,
            2u,
            blue);
    draw_streak(
            framebuffer,
            118,
            7,
            159,
            0,
            2u,
            cyan);
    draw_streak(
            framebuffer,
            126,
            14,
            159,
            9,
            2u,
            blue);

    draw_streak(
            framebuffer,
            0,
            47,
            40,
            39,
            2u,
            magenta);
    draw_streak(
            framebuffer,
            0,
            55,
            34,
            49,
            2u,
            red);
    draw_streak(
            framebuffer,
            113,
            47,
            159,
            38,
            2u,
            magenta);
    draw_streak(
            framebuffer,
            120,
            57,
            159,
            50,
            2u,
            red);

    /*
     * A two-pixel black drop shadow cuts the wordmark free from any streaks
     * that pass underneath it.
     */
    if (!draw_word(
            framebuffer,
            "RELAY",
            20,
            4,
            black)
            || !draw_word(
                    framebuffer,
                    "RACE",
                    34,
                    36,
                    black)
            || !draw_word(
                    framebuffer,
                    "RELAY",
                    18,
                    2,
                    lime)
            || !draw_word(
                    framebuffer,
                    "RACE",
                    32,
                    34,
                    yellow)
            || !draw_startup_info(
                    framebuffer,
                    info,
                    version_color)) {
        return false;
    }

    return true;
}


bool matrix_startup_screen_render(
        MatrixFramebuffer *framebuffer,
        const MatrixStartupInfo *info) {
    return draw_startup_base(
            framebuffer,
            info);
}

bool matrix_startup_sequence_render(
        MatrixFramebuffer *framebuffer,
        const MatrixStartupInfo *info,
        MatrixStartupState state,
        uint8_t animation_frame) {
    if (state > MATRIX_STARTUP_READY
            || animation_frame
                    >= MATRIX_STARTUP_SEQUENCE_FRAME_COUNT
            || !draw_startup_base(
                    framebuffer,
                    info)) {
        return false;
    }

    const MatrixPixel warning_background =
            matrix_pixel_rgb565_create(
                    0,
                    96,
                    255);
    const MatrixPixel warning_glyph =
            matrix_pixel_rgb565_create(
                    0xee,
                    0xee,
                    0xee);
    const MatrixPixel warning_strike =
            matrix_pixel_rgb565_create(
                    255,
                    0,
                    0);
    const uint16_t one_status_x =
            (uint16_t)(
                    MATRIX_DISPLAY_WIDTH
                    - MATRIX_STATUS_ICON_WIDTH);
    const uint16_t two_status_x =
            (uint16_t)(
                    MATRIX_DISPLAY_WIDTH
                    - 2u * MATRIX_STATUS_ICON_WIDTH
                    - MATRIX_STATUS_ICON_GAP);
    const uint16_t second_status_x =
            (uint16_t)(
                    two_status_x
                    + MATRIX_STATUS_ICON_WIDTH
                    + MATRIX_STATUS_ICON_GAP);

    switch (state) {
        case MATRIX_STARTUP_BOOT:
            matrix_status_icon_draw(
                    framebuffer,
                    two_status_x,
                    0u,
                    MATRIX_STATUS_ICON_WIFI,
                    warning_background,
                    warning_glyph,
                    true,
                    warning_strike);
            matrix_status_icon_draw(
                    framebuffer,
                    second_status_x,
                    0u,
                    MATRIX_STATUS_ICON_LINK,
                    warning_background,
                    warning_glyph,
                    true,
                    warning_strike);
            break;

        case MATRIX_STARTUP_WIFI_SCAN:
            matrix_status_icon_draw_progress(
                    framebuffer,
                    two_status_x,
                    0u,
                    MATRIX_STATUS_ICON_WIFI,
                    animation_frame,
                    warning_background,
                    warning_glyph);
            matrix_status_icon_draw(
                    framebuffer,
                    second_status_x,
                    0u,
                    MATRIX_STATUS_ICON_LINK,
                    warning_background,
                    warning_glyph,
                    true,
                    warning_strike);
            break;

        case MATRIX_STARTUP_WIFI_CONNECTED:
            matrix_status_icon_draw(
                    framebuffer,
                    one_status_x,
                    0u,
                    MATRIX_STATUS_ICON_LINK,
                    warning_background,
                    warning_glyph,
                    true,
                    warning_strike);
            break;

        case MATRIX_STARTUP_LINK_CONNECTING:
            matrix_status_icon_draw_progress(
                    framebuffer,
                    one_status_x,
                    0u,
                    MATRIX_STATUS_ICON_LINK,
                    animation_frame,
                    warning_background,
                    warning_glyph);
            break;

        case MATRIX_STARTUP_READY:
            break;
    }

    return true;
}
