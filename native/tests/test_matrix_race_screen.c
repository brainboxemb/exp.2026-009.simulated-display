#include "matrix_race_screen.h"

#include <assert.h>
#include <stdio.h>

static const MatrixRaceScreenData SAMPLE = {
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
                    10)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    90,
                    30)
            == theme.line_color);
}

static void test_font_family_and_size_are_configurable(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    theme.header.font.family =
            MATRIX_FONT_MONTSERRAT;
    theme.header.font.size = 10;
    theme.large_value.font.family =
            MATRIX_FONT_MONTSERRAT;
    theme.large_value.font.size = 20;

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme));
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
    test_unsupported_font_size_is_rejected();

    puts("matrix_race_screen tests passed");
    return 0;
}
