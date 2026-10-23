#include "matrix_race_screen.h"
#include "matrix_numeric_font.h"
#include "matrix_vbz_font.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const MatrixRaceScreenData SAMPLE = {
    .location = "WP-99",
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
        {"999", "99:99", "99", "4"},
        {"999", "99:99", "99", "5"}
    },
    .right_rows = {
        {"999", "999"},
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
    assert(theme.large_value.font.size == 14);
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
                    81,
                    30)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    52)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    159,
                    52)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    81,
                    53)
            != theme.line_color);
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
            || pixel == theme->sprite_color
            || pixel == theme->flag_color
            || pixel == theme->warning_background_color
            || pixel == theme->warning_text_color
            || pixel == theme->warning_strike_color
            || pixel == theme->sync_background_color
            || pixel == theme->sync_icon_color;
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
    lv_font_glyph_dsc_t rank_suffix = {0};

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

    assert(
            matrix_font_vbz_9.get_glyph_dsc(
                    &matrix_font_vbz_9,
                    &rank_suffix,
                    'e',
                    0));
    assert(rank_suffix.box_h == 7);
    assert(rank_suffix.adv_w == 7);
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
                    81));
    assert(
            !row_contains_color(
                    &framebuffer,
                    54,
                    footer_color,
                    81));
    assert(
            row_contains_color(
                    &framebuffer,
                    55,
                    footer_color,
                    81));
    assert(
            row_contains_color(
                    &framebuffer,
                    63,
                    footer_color,
                    81));
}

static void test_numeric_14_uses_two_pixel_horizontal_strokes(void) {
    lv_font_glyph_dsc_t glyph = {0};

    assert(
            matrix_font_numeric_14.get_glyph_dsc(
                    &matrix_font_numeric_14,
                    &glyph,
                    '0',
                    0));
    assert(glyph.box_w == 9);
    assert(glyph.box_h == 14);
    assert(glyph.adv_w == 10);

    lv_draw_buf_t *draw_buffer =
            lv_draw_buf_create(
                    glyph.box_w,
                    glyph.box_h,
                    LV_COLOR_FORMAT_A8,
                    LV_STRIDE_AUTO);

    assert(draw_buffer != NULL);
    assert(
            matrix_font_numeric_14.get_glyph_bitmap(
                    &glyph,
                    draw_buffer)
            == draw_buffer);

    for (uint32_t x = 1;
            x <= 7;
            x++) {
        assert(draw_buffer->data[x] == 0xffu);
        assert(
                draw_buffer->data[
                        draw_buffer->header.stride
                        + x]
                == 0xffu);
        assert(
                draw_buffer->data[
                        12u * draw_buffer->header.stride
                        + x]
                == 0xffu);
        assert(
                draw_buffer->data[
                        13u * draw_buffer->header.stride
                        + x]
                == 0xffu);
    }

    lv_draw_buf_destroy(draw_buffer);
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


static int first_header_pixel_x(
        const MatrixFramebuffer *framebuffer,
        MatrixPixel color) {
    for (uint16_t x = 40u;
            x < 97u;
            x++) {
        for (uint16_t y = 0;
                y < 11u;
                y++) {
            if (matrix_framebuffer_get(
                    framebuffer,
                    x,
                    y)
                    == color) {
                return (int)x;
            }
        }
    }

    return -1;
}

static void test_header_statuses_shift_content_and_clock(void) {
    MatrixFramebuffer connected_frame;
    MatrixFramebuffer wifi_frame;
    MatrixFramebuffer disconnected_frame;
    MatrixFramebuffer all_status_frame;
    MatrixTheme theme =
            matrix_theme_default_create();
    MatrixRaceScreenData wifi_warning = SAMPLE;
    MatrixRaceScreenData disconnected = SAMPLE;
    MatrixRaceScreenData all_status = SAMPLE;

    theme.header.color =
            matrix_pixel_rgb565_create(
                    255,
                    128,
                    0);
    theme.flag_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);
    theme.warning_background_color =
            matrix_pixel_rgb565_create(
                    0,
                    0,
                    255);
    theme.warning_text_color =
            matrix_pixel_rgb565_create(
                    255,
                    255,
                    255);
    theme.warning_strike_color =
            matrix_pixel_rgb565_create(
                    255,
                    0,
                    0);
    theme.sync_background_color =
            matrix_pixel_rgb565_create(
                    0,
                    160,
                    72);
    theme.sync_icon_color =
            matrix_pixel_rgb565_create(
                    255,
                    255,
                    255);

    wifi_warning.wifi_connected = false;

    disconnected.wifi_connected = false;
    disconnected.registration_server_connected = false;

    all_status.wifi_connected = false;
    all_status.registration_server_connected = false;
    all_status.synchronizing = true;

    assert(
            matrix_race_screen_render(
                    &connected_frame,
                    &SAMPLE,
                    &theme,
                    0));
    assert(
            matrix_race_screen_render(
                    &wifi_frame,
                    &wifi_warning,
                    &theme,
                    0));
    assert(
            matrix_race_screen_render(
                    &disconnected_frame,
                    &disconnected,
                    &theme,
                    0));
    assert(
            matrix_race_screen_render(
                    &all_status_frame,
                    &all_status,
                    &theme,
                    0));

    /*
     * No status: flag x=132.
     * One 11px status: x=121.
     * Two statuses plus separator: x=109.
     * Three statuses plus separators: x=97.
     */
    assert(
            matrix_framebuffer_get(
                    &connected_frame,
                    132,
                    1)
            == theme.flag_color);
    assert(
            matrix_framebuffer_get(
                    &wifi_frame,
                    121,
                    1)
            == theme.flag_color);
    assert(
            matrix_framebuffer_get(
                    &disconnected_frame,
                    109,
                    1)
            == theme.flag_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    97,
                    1)
            == theme.flag_color);

    /*
     * With all three statuses, WiFi, broken link and sync occupy x=125, 137
     * and 149. Sync is green/white and deliberately has no red strike.
     */
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    125,
                    10)
            == theme.warning_background_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    137,
                    10)
            == theme.warning_background_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    149,
                    0)
            == theme.sync_background_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    151,
                    2)
            == theme.sync_icon_color);

    /*
     * Warning strikes cover both outer corners and are two pixels thick.
     */
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    125,
                    0)
            == theme.warning_strike_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    126,
                    0)
            == theme.warning_strike_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    135,
                    10)
            == theme.warning_strike_color);

    /*
     * The first status does not reduce the normal 80px clock field. Once the
     * right-side group grows further, the field shrinks and the centered clock
     * therefore moves left.
     */
    const int connected_clock_x =
            first_header_pixel_x(
                    &connected_frame,
                    theme.header.color);
    const int wifi_clock_x =
            first_header_pixel_x(
                    &wifi_frame,
                    theme.header.color);
    const int all_status_clock_x =
            first_header_pixel_x(
                    &all_status_frame,
                    theme.header.color);

    assert(connected_clock_x >= 0);
    assert(wifi_clock_x == connected_clock_x);
    assert(all_status_clock_x < connected_clock_x);
}

static void test_registration_layout_uses_full_width_without_sprites(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();
    MatrixRaceScreenData registrations = SAMPLE;

    registrations.layout =
            MATRIX_RACE_LAYOUT_REGISTRATIONS;

    theme.line_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);
    theme.sprite_color =
            matrix_pixel_rgb565_create(
                    0,
                    0,
                    255);
    theme.row_number.color =
            matrix_pixel_rgb565_create(
                    255,
                    255,
                    0);
    theme.row_time.color =
            theme.row_number.color;

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &registrations,
                    &theme,
                    0));

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    81,
                    30)
            != theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    159,
                    52)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    4,
                    13)
            != theme.sprite_color);

    /*
     * The normal registration preview shows three rows. The fourth row at
     * y=42 is reserved for real data later and is not part of the default
     * preview.
     */
    assert(
            !row_contains_color(
                    &framebuffer,
                    45,
                    theme.row_number.color,
                    MATRIX_DISPLAY_WIDTH));
}

static void test_registration_overflow_removes_footer(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();
    MatrixRaceScreenData overflow = SAMPLE;

    overflow.layout =
            MATRIX_RACE_LAYOUT_REGISTRATIONS_OVERFLOW;

    theme.line_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);
    theme.footer.color =
            matrix_pixel_rgb565_create(
                    0,
                    0,
                    255);

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &overflow,
                    &theme,
                    0));

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    52)
            != theme.line_color);

    for (size_t index = 0;
            index < MATRIX_DISPLAY_PIXEL_COUNT;
            index++) {
        assert(
                framebuffer.pixels[index]
                != theme.footer.color);
    }
}

static void test_ready_overflow_keeps_left_half_footer(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();
    MatrixRaceScreenData overflow = SAMPLE;

    overflow.layout =
            MATRIX_RACE_LAYOUT_MIXED_READY_OVERFLOW;

    theme.line_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &overflow,
                    &theme,
                    0));

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    52)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    80,
                    52)
            == theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    100,
                    52)
            != theme.line_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    81,
                    53)
            == theme.line_color);
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

    /*
     * Runner is in the passed-runner column at the far left.
     * Warmup is in the ready-runner column after the vertical divider.
     */
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    4,
                    13)
            == runner_color);
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    86,
                    14)
            == runner_color);

    /*
     * Frame 2 changes both animations without moving either body to another
     * screen location.
     */
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    1,
                    22)
            != runner_color);
    assert(
            matrix_framebuffer_get(
                    &frame2,
                    1,
                    22)
            == runner_color);

    assert(
            matrix_framebuffer_get(
                    &frame0,
                    84,
                    18)
            != runner_color);
    assert(
            matrix_framebuffer_get(
                    &frame2,
                    84,
                    18)
            == runner_color);
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
    test_numeric_14_uses_two_pixel_horizontal_strokes();
    test_numeric_12_uses_two_pixel_horizontal_strokes();
    test_header_statuses_shift_content_and_clock();
    test_registration_layout_uses_full_width_without_sprites();
    test_registration_overflow_removes_footer();
    test_ready_overflow_keeps_left_half_footer();
    test_runner_and_warmup_animate_in_their_status_columns();
    test_unsupported_font_size_is_rejected();

    puts("matrix_race_screen tests passed");
    return 0;
}
