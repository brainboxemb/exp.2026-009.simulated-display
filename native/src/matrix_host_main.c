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
#define MATRIX_HOST_REQUEST_THEME 2u
#define MATRIX_HOST_REQUEST_ANIMATION_FRAME 3u
#define MATRIX_HOST_REQUEST_CONNECTION_STATUS 4u
#define MATRIX_HOST_REQUEST_LAYOUT 5u

static MatrixRaceScreenData DEMO_SCREEN = {
    .location = "99",
    .clock = "99:99:99",
    .counter = "999",
    .layout = MATRIX_RACE_LAYOUT_MIXED,
    .wifi_connected = true,
    .registration_server_connected = true,
    .synchronizing = false,
    .left_rows = {
        {"999", "99:99", "99", "1"},
        {"999", "99:99", "99", "2"},
        {"999", "99:99", "99", "3"},
        {"999", "99:99", "99", "4"}
    },
    .right_rows = {
        {"999", "999"},
        {"999", "999"},
        {"999", "999"}
    },
    .footer = "520 234 346 12"
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

static bool read_u8(uint8_t *value) {
    const int byte = fgetc(stdin);

    if (byte == EOF) {
        return false;
    }

    *value = (uint8_t)byte;
    return true;
}

static bool read_u16_be(uint16_t *value) {
    uint8_t high = 0;
    uint8_t low = 0;

    if (!read_u8(&high)
            || !read_u8(&low)) {
        return false;
    }

    *value =
            ((uint16_t)high << 8)
            | (uint16_t)low;
    return true;
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
        MatrixFramebuffer *framebuffer,
        const MatrixTheme *theme,
        uint8_t animation_frame) {
    return matrix_race_screen_render(
            framebuffer,
            &DEMO_SCREEN,
            theme,
            animation_frame);
}

static bool read_theme(MatrixTheme *theme) {
    uint16_t background_color = 0;
    uint16_t line_color = 0;
    uint16_t header_color = 0;
    uint16_t row_color = 0;
    uint16_t large_color = 0;
    uint16_t footer_color = 0;
    uint16_t sprite_color = 0;
    uint16_t exchange_point_background_color = 0;
    uint16_t exchange_point_text_color = 0;
    uint16_t flag_color = 0;
    uint16_t warning_background_color = 0;
    uint16_t warning_text_color = 0;
    uint16_t warning_strike_color = 0;
    uint16_t sync_background_color = 0;
    uint16_t sync_icon_color = 0;
    uint8_t family = 0;
    uint8_t header_size = 0;
    uint8_t row_size = 0;
    uint8_t large_size = 0;
    uint8_t footer_size = 0;

    if (!read_u16_be(&background_color)
            || !read_u16_be(&line_color)
            || !read_u16_be(&header_color)
            || !read_u16_be(&row_color)
            || !read_u16_be(&large_color)
            || !read_u16_be(&footer_color)
            || !read_u16_be(&sprite_color)
            || !read_u16_be(&exchange_point_background_color)
            || !read_u16_be(&exchange_point_text_color)
            || !read_u16_be(&flag_color)
            || !read_u16_be(&warning_background_color)
            || !read_u16_be(&warning_text_color)
            || !read_u16_be(&warning_strike_color)
            || !read_u16_be(&sync_background_color)
            || !read_u16_be(&sync_icon_color)
            || !read_u8(&family)
            || !read_u8(&header_size)
            || !read_u8(&row_size)
            || !read_u8(&large_size)
            || !read_u8(&footer_size)) {
        return false;
    }

    if (family > MATRIX_FONT_VBZ) {
        return false;
    }

    theme->background_color = background_color;
    theme->line_color = line_color;
    theme->header.color = header_color;
    theme->row_number.color = row_color;
    theme->row_time.color = row_color;
    theme->large_value.color = large_color;
    theme->footer.color = footer_color;
    theme->sprite_color = sprite_color;
    theme->exchange_point_background_color =
            exchange_point_background_color;
    theme->exchange_point_text_color =
            exchange_point_text_color;
    theme->flag_color = flag_color;
    theme->warning_background_color = warning_background_color;
    theme->warning_text_color = warning_text_color;
    theme->warning_strike_color = warning_strike_color;
    theme->sync_background_color = sync_background_color;
    theme->sync_icon_color = sync_icon_color;

    theme->header.font.family =
            (MatrixFontFamily)family;
    theme->row_number.font.family =
            (MatrixFontFamily)family;
    theme->row_time.font.family =
            (MatrixFontFamily)family;
    theme->large_value.font.family =
            (MatrixFontFamily)family;
    theme->footer.font.family =
            (MatrixFontFamily)family;

    theme->header.font.size = header_size;
    theme->row_number.font.size = row_size;
    theme->row_time.font.size = row_size;
    theme->large_value.font.size = large_size;
    theme->footer.font.size = footer_size;

    return true;
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
    MatrixTheme theme =
            matrix_theme_default_create();
    uint8_t animation_frame = 0;
    bool demo_visible = true;

    if (!draw_demo_screen(
            &framebuffer,
            &theme,
            animation_frame)) {
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
                demo_visible = true;
                if (!draw_demo_screen(
                        &framebuffer,
                        &theme,
                        animation_frame)) {
                    return 6;
                }
                write_frame(&framebuffer);
                break;

            case MATRIX_HOST_REQUEST_CLEAR:
                demo_visible = false;
                matrix_framebuffer_clear(&framebuffer);
                write_frame(&framebuffer);
                break;

            case MATRIX_HOST_REQUEST_THEME:
                if (!read_theme(&theme)) {
                    return 6;
                }

                if (demo_visible) {
                    if (!draw_demo_screen(
                            &framebuffer,
                            &theme,
                            animation_frame)) {
                        return 6;
                    }
                } else {
                    matrix_framebuffer_clear(&framebuffer);
                }

                write_frame(&framebuffer);
                break;

            case MATRIX_HOST_REQUEST_ANIMATION_FRAME: {
                uint8_t requested_frame = 0;

                if (!read_u8(&requested_frame)
                        || requested_frame >= 12u) {
                    return 6;
                }

                animation_frame = requested_frame;

                if (!demo_visible) {
                    break;
                }

                if (!draw_demo_screen(
                        &framebuffer,
                        &theme,
                        animation_frame)) {
                    return 6;
                }

                write_frame(&framebuffer);
                break;
            }

            case MATRIX_HOST_REQUEST_LAYOUT: {
                uint8_t layout = 0;

                if (!read_u8(&layout)
                        || layout > MATRIX_RACE_LAYOUT_FONT_TEST) {
                    return 6;
                }

                DEMO_SCREEN.layout =
                        (MatrixRaceLayout)layout;

                if (demo_visible) {
                    if (!draw_demo_screen(
                            &framebuffer,
                            &theme,
                            animation_frame)) {
                        return 6;
                    }

                    write_frame(&framebuffer);
                }
                break;
            }

            case MATRIX_HOST_REQUEST_CONNECTION_STATUS: {
                uint8_t wifi_connected = 0;
                uint8_t registration_server_connected = 0;
                uint8_t synchronizing = 0;

                if (!read_u8(&wifi_connected)
                        || !read_u8(&registration_server_connected)
                        || !read_u8(&synchronizing)
                        || wifi_connected > 1u
                        || registration_server_connected > 1u
                        || synchronizing > 1u) {
                    return 6;
                }

                DEMO_SCREEN.wifi_connected =
                        wifi_connected != 0;
                DEMO_SCREEN.registration_server_connected =
                        registration_server_connected != 0;
                DEMO_SCREEN.synchronizing =
                        synchronizing != 0;

                if (demo_visible) {
                    if (!draw_demo_screen(
                            &framebuffer,
                            &theme,
                            animation_frame)) {
                        return 6;
                    }

                    write_frame(&framebuffer);
                }
                break;
            }

            default:
                return 2;
        }
    }

    return 0;
}
