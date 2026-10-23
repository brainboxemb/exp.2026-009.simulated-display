#include "matrix_framebuffer.h"
#include "matrix_race_screen.h"
#include "matrix_theme.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define MATRIX_HOST_REQUEST_DEMO_SCREEN 0u
#define MATRIX_HOST_REQUEST_CLEAR 1u

static const MatrixRaceScreenData DEMO_SCREEN = {
    .location = "WP-50",
    .clock = "02:05:67",
    .counter = "200",
    .left_rows = {
        {"1", "99:99:99"},
        {"520", "99:99:99"},
        {"65", "99:99:99"}
    },
    .right_rows = {
        {"527", "527"},
        {"527", "527"}
    },
    .footer = {"520", "234", "346", "12"}
};

static void write_u16_be(uint16_t value) {
    fputc((value >> 8) & 0xff, stdout);
    fputc(value & 0xff, stdout);
}

static void write_u32_be(uint32_t value) {
    fputc((value >> 24) & 0xff, stdout);
    fputc((value >> 16) & 0xff, stdout);
    fputc((value >> 8) & 0xff, stdout);
    fputc(value & 0xff, stdout);
}

static bool read_u32_be(uint32_t *value) {
    uint8_t bytes[4];

    if (fread(bytes, 1, sizeof(bytes), stdin)
            != sizeof(bytes)) {
        return false;
    }

    *value =
            ((uint32_t)bytes[0] << 24)
            | ((uint32_t)bytes[1] << 16)
            | ((uint32_t)bytes[2] << 8)
            | (uint32_t)bytes[3];
    return true;
}

static void write_frame(
        const MatrixFramebuffer *framebuffer) {
    static const uint8_t magic[] = {'S', 'D', 'F', '2'};

    fwrite(magic, 1, sizeof(magic), stdout);
    write_u16_be(MATRIX_DISPLAY_WIDTH);
    write_u16_be(MATRIX_DISPLAY_HEIGHT);
    write_u32_be(MATRIX_DISPLAY_PIXEL_COUNT * 2u);

    for (uint16_t y = 0;
            y < MATRIX_DISPLAY_HEIGHT;
            y++) {
        for (uint16_t x = 0;
                x < MATRIX_DISPLAY_WIDTH;
                x++) {
            write_u16_be(
                    matrix_framebuffer_get(
                            framebuffer,
                            x,
                            y));
        }
    }

    fflush(stdout);
}

static bool draw_demo_screen(
        MatrixFramebuffer *framebuffer) {
    MatrixTheme theme =
            matrix_theme_default_create();

    return matrix_race_screen_render(
            framebuffer,
            &DEMO_SCREEN,
            &theme);
}

int main(void) {
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1) {
        return 4;
    }

    if (_setmode(_fileno(stdout), _O_BINARY) == -1) {
        return 5;
    }
#endif

    MatrixFramebuffer framebuffer;

    if (!draw_demo_screen(&framebuffer)) {
        return 6;
    }

    write_frame(&framebuffer);

    for (;;) {
        uint32_t request = 0;

        if (!read_u32_be(&request)) {
            break;
        }

        switch (request) {
            case MATRIX_HOST_REQUEST_DEMO_SCREEN:
                if (!draw_demo_screen(&framebuffer)) {
                    return 6;
                }
                write_frame(&framebuffer);
                break;

            case MATRIX_HOST_REQUEST_CLEAR:
                matrix_framebuffer_clear(&framebuffer);
                write_frame(&framebuffer);
                break;

            default:
                return 2;
        }
    }

    return 0;
}
