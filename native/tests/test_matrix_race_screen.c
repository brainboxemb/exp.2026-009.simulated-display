#include "matrix_race_screen.h"
#include "matrix_sprite.h"
#include "matrix_numeric_font.h"
#include "matrix_vbz_font.h"

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
    .footer = "520 234 346 12"
};


static void test_default_theme_uses_vbz_and_independent_sprite_color(void) {
    MatrixTheme theme =
            matrix_theme_default_create();

    assert(theme.header.font.family == MATRIX_FONT_VBZ);
    assert(theme.header.font.size == 9);
    assert(theme.row_number.font.family == MATRIX_FONT_VBZ);
    assert(theme.row_number.font.size == 9);
    assert(theme.row_time.font.family == MATRIX_FONT_VBZ);
    assert(theme.row_time.font.size == 9);
    assert(theme.large_value.font.family == MATRIX_FONT_VBZ);
    assert(theme.large_value.font.size == 12);
    assert(theme.footer.font.family == MATRIX_FONT_VBZ);
    assert(theme.footer.font.size == 9);

    assert(
            theme.sprite_color
            == matrix_pixel_rgb565_create(
                    255,
                    0,
                    0));
}

static void test_default_screen_renders_lines(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme,
                    0));

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
                    52)
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
                    &theme,
                    0));
}

static void test_vbz_9_and_numeric_12_render(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    theme.header.font.family =
            MATRIX_FONT_VBZ;
    theme.header.font.size = 9;
    theme.row_number.font.family =
            MATRIX_FONT_VBZ;
    theme.row_number.font.size = 9;
    theme.row_time.font.family =
            MATRIX_FONT_VBZ;
    theme.row_time.font.size = 9;
    theme.large_value.font.family =
            MATRIX_FONT_VBZ;
    theme.large_value.font.size = 12;
    theme.footer.font.family =
            MATRIX_FONT_VBZ;
    theme.footer.font.size = 9;

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme,
                    0));
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
                    &theme,
                    0));

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
            || pixel == theme->footer.color
            || pixel == theme->sprite_color;
}

static void test_pixel_fonts_render_only_theme_colors(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme,
                    0));

    for (size_t index = 0;
            index < MATRIX_DISPLAY_PIXEL_COUNT;
            index++) {
        assert(
                is_theme_color(
                        framebuffer.pixels[index],
                        &theme));
    }
}

static void test_vbz_metrics_match_source_font(void) {
    lv_font_glyph_dsc_t digit = {0};
    lv_font_glyph_dsc_t colon = {0};

    assert(
            matrix_font_vbz_9.get_glyph_dsc(
                    &matrix_font_vbz_9,
                    &digit,
                    '0',
                    0));
    assert(digit.box_w == 16);
    assert(digit.box_h == 9);
    assert(digit.adv_w == 7);
    assert(digit.ofs_x == 1);

    assert(
            matrix_font_vbz_9.get_glyph_dsc(
                    &matrix_font_vbz_9,
                    &colon,
                    ':',
                    0));
    assert(colon.box_w == 16);
    assert(colon.box_h == 6);
    assert(colon.adv_w == 3);
    assert(colon.ofs_x == 1);
}

static void test_vbz_zero_bitmap_matches_source_rows(void) {
    lv_font_glyph_dsc_t glyph = {0};

    assert(
            matrix_font_vbz_9.get_glyph_dsc(
                    &matrix_font_vbz_9,
                    &glyph,
                    '0',
                    0));

    lv_draw_buf_t *draw_buffer =
            lv_draw_buf_create(
                    glyph.box_w,
                    glyph.box_h,
                    LV_COLOR_FORMAT_A8,
                    LV_STRIDE_AUTO);

    assert(draw_buffer != NULL);

    const void *bitmap =
            matrix_font_vbz_9.get_glyph_bitmap(
                    &glyph,
                    draw_buffer);

    assert(bitmap == draw_buffer);

    /*
     * Source rows for '0':
     *   0x78 = 01111000
     *   0xCC = 11001100
     */
    const uint8_t *row0 =
            &draw_buffer->data[0];
    const uint8_t *row1 =
            &draw_buffer->data[
                    draw_buffer->header.stride];

    assert(row0[0] == 0x00);
    assert(row0[1] == 0xff);
    assert(row0[2] == 0xff);
    assert(row0[3] == 0xff);
    assert(row0[4] == 0xff);
    assert(row0[5] == 0x00);

    assert(row1[0] == 0xff);
    assert(row1[1] == 0xff);
    assert(row1[2] == 0x00);
    assert(row1[3] == 0x00);
    assert(row1[4] == 0xff);
    assert(row1[5] == 0xff);

    lv_draw_buf_destroy(draw_buffer);
}

static bool row_contains_color(
        const MatrixFramebuffer *framebuffer,
        uint16_t y,
        MatrixPixel color,
        uint16_t x_limit) {
    for (uint16_t x = 0;
            x < x_limit;
            x++) {
        if (matrix_framebuffer_get(
                framebuffer,
                x,
                y)
                == color) {
            return true;
        }
    }

    return false;
}

static void test_vbz_header_and_footer_use_vertical_edges(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    const MatrixPixel header_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);
    const MatrixPixel footer_color =
            matrix_pixel_rgb565_create(
                    0,
                    0,
                    255);

    theme.header.font.family = MATRIX_FONT_VBZ;
    theme.header.font.size = 9;
    theme.footer.font.family = MATRIX_FONT_VBZ;
    theme.footer.font.size = 9;
    theme.header.color = header_color;
    theme.footer.color = footer_color;

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme,
                    0));

    assert(
            row_contains_color(
                    &framebuffer,
                    0,
                    header_color,
                    MATRIX_DISPLAY_WIDTH));

    assert(
            !row_contains_color(
                    &framebuffer,
                    53,
                    footer_color,
                    90));
    assert(
            !row_contains_color(
                    &framebuffer,
                    54,
                    footer_color,
                    90));
    assert(
            row_contains_color(
                    &framebuffer,
                    55,
                    footer_color,
                    90));
    assert(
            row_contains_color(
                    &framebuffer,
                    63,
                    footer_color,
                    90));
}

static void test_numeric_12_uses_two_pixel_horizontal_strokes(void) {
    lv_font_glyph_dsc_t glyph = {0};

    assert(
            matrix_font_numeric_12.get_glyph_dsc(
                    &matrix_font_numeric_12,
                    &glyph,
                    '0',
                    0));

    lv_draw_buf_t *draw_buffer =
            lv_draw_buf_create(
                    glyph.box_w,
                    glyph.box_h,
                    LV_COLOR_FORMAT_A8,
                    LV_STRIDE_AUTO);

    assert(draw_buffer != NULL);

    assert(
            matrix_font_numeric_12.get_glyph_bitmap(
                    &glyph,
                    draw_buffer)
            == draw_buffer);

    for (uint32_t x = 1;
            x <= 6;
            x++) {
        assert(
                draw_buffer->data[x]
                == 0xffu);
        assert(
                draw_buffer->data[
                        draw_buffer->header.stride
                        + x]
                == 0xffu);
        assert(
                draw_buffer->data[
                        10u * draw_buffer->header.stride
                        + x]
                == 0xffu);
        assert(
                draw_buffer->data[
                        11u * draw_buffer->header.stride
                        + x]
                == 0xffu);
    }

    lv_draw_buf_destroy(draw_buffer);
}




static void test_runner_legs_do_not_form_thick_blocks(void) {
    const MatrixPixel color =
            matrix_pixel_rgb565_create(
                    255,
                    255,
                    255);

    for (uint8_t frame = 0;
            frame < MATRIX_SPRITE_FRAME_COUNT;
            frame++) {
        MatrixFramebuffer framebuffer;
        matrix_framebuffer_clear(&framebuffer);

        matrix_sprite_draw(
                &framebuffer,
                MATRIX_SPRITE_RUNNER,
                frame,
                0,
                0,
                color);

        for (uint16_t y = 8;
                y + 1u < MATRIX_SPRITE_HEIGHT;
                y++) {
            for (uint16_t x = 0;
                    x + 1u < MATRIX_SPRITE_WIDTH;
                    x++) {
                const bool top_left =
                        matrix_framebuffer_get(
                                &framebuffer,
                                x,
                                y)
                        == color;
                const bool top_right =
                        matrix_framebuffer_get(
                                &framebuffer,
                                (uint16_t)(x + 1u),
                                y)
                        == color;
                const bool bottom_left =
                        matrix_framebuffer_get(
                                &framebuffer,
                                x,
                                (uint16_t)(y + 1u))
                        == color;
                const bool bottom_right =
                        matrix_framebuffer_get(
                                &framebuffer,
                                (uint16_t)(x + 1u),
                                (uint16_t)(y + 1u))
                        == color;

                assert(
                        !(top_left
                                && top_right
                                && bottom_left
                                && bottom_right));
            }
        }
    }
}

static void test_runner_has_stable_three_by_three_head(void) {
    const MatrixPixel color =
            matrix_pixel_rgb565_create(
                    255,
                    255,
                    255);

    for (uint8_t frame = 0;
            frame < MATRIX_SPRITE_FRAME_COUNT;
            frame++) {
        MatrixFramebuffer framebuffer;
        matrix_framebuffer_clear(&framebuffer);

        matrix_sprite_draw(
                &framebuffer,
                MATRIX_SPRITE_RUNNER,
                frame,
                0,
                0,
                color);

        for (uint16_t y = 0;
                y < 3;
                y++) {
            for (uint16_t x = 5;
                    x <= 7;
                    x++) {
                assert(
                        matrix_framebuffer_get(
                                &framebuffer,
                                x,
                                y)
                        == color);
            }

            assert(
                    matrix_framebuffer_get(
                            &framebuffer,
                            4,
                            y)
                    != color);
            assert(
                    matrix_framebuffer_get(
                            &framebuffer,
                            8,
                            y)
                    != color);
        }

        assert(
                matrix_framebuffer_get(
                        &framebuffer,
                        6,
                        3)
                == color);
    }
}

static void test_runner_opposite_contact_phases_reverse_limb_sides(void) {
    MatrixFramebuffer frame0;
    MatrixFramebuffer frame3;
    const MatrixPixel color =
            matrix_pixel_rgb565_create(
                    255,
                    255,
                    255);

    matrix_framebuffer_clear(&frame0);
    matrix_framebuffer_clear(&frame3);

    matrix_sprite_draw(
            &frame0,
            MATRIX_SPRITE_RUNNER,
            0,
            0,
            0,
            color);
    matrix_sprite_draw(
            &frame3,
            MATRIX_SPRITE_RUNNER,
            3,
            0,
            0,
            color);

    assert(
            matrix_framebuffer_get(
                    &frame0,
                    8,
                    4)
            == color);
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    2,
                    9)
            == color);
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    9,
                    10)
            == color);

    assert(
            matrix_framebuffer_get(
                    &frame3,
                    2,
                    4)
            == color);
    assert(
            matrix_framebuffer_get(
                    &frame3,
                    8,
                    9)
            == color);
    assert(
            matrix_framebuffer_get(
                    &frame3,
                    1,
                    10)
            == color);
}

static void test_runner_and_warmup_animate_in_their_status_columns(void) {
    MatrixFramebuffer frame0;
    MatrixFramebuffer frame2;
    MatrixTheme theme =
            matrix_theme_default_create();

    const MatrixPixel runner_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);
    theme.sprite_color = runner_color;

    assert(
            matrix_race_screen_render(
                    &frame0,
                    &SAMPLE,
                    &theme,
                    0));
    assert(
            matrix_race_screen_render(
                    &frame2,
                    &SAMPLE,
                    &theme,
                    2));

    assert(
            matrix_framebuffer_get(
                    &frame0,
                    6,
                    13)
            == runner_color);
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    96,
                    14)
            == runner_color);

    assert(
            memcmp(
                    frame0.pixels,
                    frame2.pixels,
                    sizeof(frame0.pixels))
            != 0);
}

static void test_unsupported_font_size_is_rejected(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();

    theme.header.font.size = 10;

    assert(
            !matrix_race_screen_render(
                    &framebuffer,
                    &SAMPLE,
                    &theme,
                    0));
}

int main(void) {
    test_default_theme_uses_vbz_and_independent_sprite_color();
    test_default_screen_renders_lines();
    test_font_family_and_size_are_configurable();
    test_vbz_9_and_numeric_12_render();
    test_independent_line_and_text_colors_render();
    test_pixel_fonts_render_only_theme_colors();
    test_vbz_metrics_match_source_font();
    test_vbz_zero_bitmap_matches_source_rows();
    test_vbz_header_and_footer_use_vertical_edges();
    test_numeric_12_uses_two_pixel_horizontal_strokes();
    test_runner_legs_do_not_form_thick_blocks();
    test_runner_has_stable_three_by_three_head();
    test_runner_opposite_contact_phases_reverse_limb_sides();
    test_runner_and_warmup_animate_in_their_status_columns();
    test_unsupported_font_size_is_rejected();

    puts("matrix_race_screen tests passed");
    return 0;
}
