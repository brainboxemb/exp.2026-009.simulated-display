#include "matrix_race_screen.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const MatrixRaceScreenData SAMPLE = {
    .location = "WP-99",
    .clock = "99:99:99",
    .counter = "999",
    .left_rows = {
        {"999", "99:99"},
        {"999", "99:99"},
        {"999", "99:99"}
    },
    .right_rows = {
        {"999", "999"},
        {"999", "999"}
    },
    .footer = {"520", "234", "346", "12"}
};

static void test_default_screen_renders_lines(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme));

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    11)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    90,
                    30)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    50)
            == theme.line_color);
}

static void test_font_family_and_size_are_configurable(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    theme.header.font.family =
            MATRIX_FONT_UNSCII;
    theme.header.font.size = 8;
    theme.large_value.font.family =
            MATRIX_FONT_UNSCII;
    theme.large_value.font.size = 16;

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme));
}

static void test_vbz_9_and_10_render(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    theme.header.font.family =
            MATRIX_FONT_VBZ;
    theme.header.font.size = 9;
    theme.row_number.font.family =
            MATRIX_FONT_VBZ;
    theme.row_number.font.size = 10;
    theme.row_time.font.family =
            MATRIX_FONT_VBZ;
    theme.row_time.font.size = 10;
    theme.large_value.font.family =
            MATRIX_FONT_VBZ;
    theme.large_value.font.size = 10;
    theme.footer.font.family =
            MATRIX_FONT_VBZ;
    theme.footer.font.size = 9;

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme));
}

static void test_independent_line_and_text_colors_render(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    theme.line_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);
    theme.header.color =
            matrix_pixel_rgb565_create(
                    255,
                    255,
                    255);

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme));

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    11)
            == theme.line_color);
}

static bool is_theme_color(
        MatrixPixel pixel,
        const MatrixTheme *theme) {
    return pixel == theme->background_color
            || pixel == theme->line_color
            || pixel == theme->header.color
            || pixel == theme->row_number.color
            || pixel == theme->row_time.color
            || pixel == theme->large_value.color
            || pixel == theme->footer.color;
}

static void test_pixel_fonts_render_only_theme_colors(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme));

    for (size_t index = 0;
            index < MATRIX_DISPLAY_PIXEL_COUNT;
            index++) {
        assert(
                is_theme_color(
                        framebuffer.pixels[index],
                        &theme));
    }
}

static void test_vbz_9_and_10_render_as_distinct_pixel_fonts(void) {
    MatrixFramebuffer frame9;
    MatrixFramebuffer frame10;

    MatrixTheme theme9 =
            matrix_theme_default_create();
    MatrixTheme theme10 =
            matrix_theme_default_create();

    theme9.header.font.family = MATRIX_FONT_VBZ;
    theme9.header.font.size = 9;
    theme9.row_number.font.family = MATRIX_FONT_VBZ;
    theme9.row_number.font.size = 9;
    theme9.row_time.font.family = MATRIX_FONT_VBZ;
    theme9.row_time.font.size = 9;
    theme9.large_value.font.family = MATRIX_FONT_VBZ;
    theme9.large_value.font.size = 9;
    theme9.footer.font.family = MATRIX_FONT_VBZ;
    theme9.footer.font.size = 9;

    theme10.header.font.family = MATRIX_FONT_VBZ;
    theme10.header.font.size = 10;
    theme10.row_number.font.family = MATRIX_FONT_VBZ;
    theme10.row_number.font.size = 10;
    theme10.row_time.font.family = MATRIX_FONT_VBZ;
    theme10.row_time.font.size = 10;
    theme10.large_value.font.family = MATRIX_FONT_VBZ;
    theme10.large_value.font.size = 10;
    theme10.footer.font.family = MATRIX_FONT_VBZ;
    theme10.footer.font.size = 10;

    assert(
            matrix_race_screen_render(
                    &frame9,
                    &SAMPLE,
                    &theme9));

    assert(
            matrix_race_screen_render(
                    &frame10,
                    &SAMPLE,
                    &theme10));

    assert(
            memcmp(
                    frame9.pixels,
                    frame10.pixels,
                    sizeof(frame9.pixels))
            != 0);
}

static void test_unsupported_font_size_is_rejected(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    theme.header.font.size = 9;

    assert(
            !matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme));
}

int main(void) {
    test_default_screen_renders_lines();
    test_font_family_and_size_are_configurable();
    test_vbz_9_and_10_render();
    test_independent_line_and_text_colors_render();
    test_pixel_fonts_render_only_theme_colors();
    test_vbz_9_and_10_render_as_distinct_pixel_fonts();
    test_unsupported_font_size_is_rejected();

    puts("matrix_race_screen tests passed");
    return 0;
}
