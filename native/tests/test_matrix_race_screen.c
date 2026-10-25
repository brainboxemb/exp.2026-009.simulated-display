#include "matrix_race_screen.h"
#include "matrix_numeric_font.h"
#include "matrix_vbz_font.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const MatrixRaceScreenData SAMPLE = {
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


static void test_default_theme_uses_vbz_and_soft_white_accents(void) {
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

    const MatrixPixel soft_white =
            matrix_pixel_rgb565_create(
                    0xee,
                    0xee,
                    0xee);

    assert(theme.sprite_color == soft_white);
    assert(theme.exchange_point_color == soft_white);
    assert(theme.flag_color == soft_white);
    assert(theme.warning_text_color == soft_white);
    assert(theme.sync_icon_color == soft_white);
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
            || pixel == theme->exchange_point_color
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
            !row_contains_color(
                    &framebuffer,
                    0,
                    header_color,
                    MATRIX_DISPLAY_WIDTH));
    assert(
            row_contains_color(
                    &framebuffer,
                    1,
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
                    2)
            == theme.flag_color);
    assert(
            matrix_framebuffer_get(
                    &wifi_frame,
                    121,
                    2)
            == theme.flag_color);
    assert(
            matrix_framebuffer_get(
                    &disconnected_frame,
                    109,
                    2)
            == theme.flag_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    97,
                    2)
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
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    150,
                    8)
            == theme.sync_icon_color);
    assert(
            matrix_framebuffer_get(
                    &all_status_frame,
                    154,
                    5)
            == theme.sync_background_color);

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

static void test_font_test_layout_aligns_all_three_font_sizes(void) {
    MatrixFramebuffer framebuffer;
    MatrixTheme theme =
            matrix_theme_default_create();
    MatrixRaceScreenData font_test = SAMPLE;

    font_test.layout =
            MATRIX_RACE_LAYOUT_FONT_TEST;

    theme.line_color =
            matrix_pixel_rgb565_create(
                    0,
                    255,
                    0);
    theme.large_value.color =
            matrix_pixel_rgb565_create(
                    255,
                    0,
                    255);

    assert(
            matrix_race_screen_render(
                    &framebuffer,
                    &font_test,
                    &theme,
                    0));

    assert(
            row_contains_color(
                    &framebuffer,
                    1,
                    theme.large_value.color,
                    MATRIX_DISPLAY_WIDTH));
    assert(
            row_contains_color(
                    &framebuffer,
                    14,
                    theme.large_value.color,
                    MATRIX_DISPLAY_WIDTH));
    assert(
            row_contains_color(
                    &framebuffer,
                    30,
                    theme.large_value.color,
                    MATRIX_DISPLAY_WIDTH));

    /*
     * Every font row uses ten fixed 14-pixel digit cells starting at x=18.
     * Each cell must contain its corresponding glyph, so 0..9 stay vertically
     * aligned while still retaining visible spacing between cells.
     */
    for (uint16_t digit = 0;
            digit < 10u;
            digit++) {
        const uint16_t cell_x =
                (uint16_t)(18u + digit * 14u);
        bool row9_has_digit = false;
        bool row12_has_digit = false;
        bool row14_has_digit = false;

        for (uint16_t x = cell_x;
                x < cell_x + 14u;
                x++) {
            for (uint16_t y = 1u;
                    y < 10u;
                    y++) {
                row9_has_digit |=
                        matrix_framebuffer_get(
                                &framebuffer,
                                x,
                                y)
                        == theme.large_value.color;
            }
            for (uint16_t y = 14u;
                    y < 26u;
                    y++) {
                row12_has_digit |=
                        matrix_framebuffer_get(
                                &framebuffer,
                                x,
                                y)
                        == theme.large_value.color;
            }
            for (uint16_t y = 30u;
                    y < 44u;
                    y++) {
                row14_has_digit |=
                        matrix_framebuffer_get(
                                &framebuffer,
                                x,
                                y)
                        == theme.large_value.color;
            }
        }

        assert(row9_has_digit);
        assert(row12_has_digit);
        assert(row14_has_digit);
    }
    assert(
            !row_contains_color(
                    &framebuffer,
                    11u,
                    theme.line_color,
                    MATRIX_DISPLAY_WIDTH));
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
     * The normal registration preview shows three rows. The fourth row only
     * appears in registration-overflow mode after the footer disappears.
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

    /*
     * Overflow means exactly one extra registration: the fourth row uses the
     * former footer space while keeping the normal 13-pixel cadence.
     */
    assert(
            row_contains_color(
                    &framebuffer,
                    55,
                    theme.row_number.color,
                    MATRIX_DISPLAY_WIDTH));

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

    /*
     * Ready overflow keeps the normal first-row baseline and uses the free
     * pixels at the bottom for the third row.
     */
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    86,
                    14)
            == theme.sprite_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    86,
                    13)
            != theme.sprite_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    95,
                    48)
            == theme.large_value.color);
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
     * The ready-team number starts after a deliberate two-pixel gap following
     * the 10-pixel warmup sprite.
     */
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    92,
                    14)
            != theme.large_value.color);
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    93,
                    14)
            != theme.large_value.color);
    assert(
            matrix_framebuffer_get(
                    &frame0,
                    95,
                    14)
            == theme.large_value.color);

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



static void test_normal_footer_keeps_vbz_spacing(void) {
    MatrixFramebuffer normal_frame;
    MatrixFramebuffer overflow_frame;
    MatrixTheme theme = matrix_theme_default_create();
    MatrixRaceScreenData normal = SAMPLE;
    MatrixRaceScreenData overflow = SAMPLE;

    const MatrixPixel footer_color =
            matrix_pixel_rgb565_create(0, 255, 0);
    theme.footer.color = footer_color;
    normal.layout = MATRIX_RACE_LAYOUT_MIXED;
    overflow.layout = MATRIX_RACE_LAYOUT_MIXED_READY_OVERFLOW;

    assert(matrix_race_screen_render(&normal_frame, &normal, &theme, 0));
    assert(matrix_race_screen_render(&overflow_frame, &overflow, &theme, 0));

    int normal_right = -1;
    int overflow_right = -1;
    for (uint16_t y = 54; y < MATRIX_DISPLAY_HEIGHT; y++) {
        for (uint16_t x = 0; x < MATRIX_DISPLAY_WIDTH; x++) {
            if (matrix_framebuffer_get(&normal_frame, x, y) == footer_color) {
                normal_right = (int)x;
            }
            if (matrix_framebuffer_get(&overflow_frame, x, y) == footer_color) {
                overflow_right = (int)x;
            }
        }
    }

    assert(normal_right >= 80);
    assert(overflow_right < 81);
    assert(normal_right > overflow_right);
}

static void test_exchange_point_is_inverted_without_prefix(void) {
    MatrixFramebuffer frame;
    MatrixTheme theme = matrix_theme_default_create();
    MatrixRaceScreenData data = SAMPLE;
    theme.header.color = matrix_pixel_rgb565_create(255, 0, 0);
    theme.exchange_point_color = matrix_pixel_rgb565_create(0xee, 0xee, 0xee);
    theme.background_color = matrix_pixel_rgb565_create(0, 0, 0);

    data.location = "99";
    assert(matrix_race_screen_render(&frame, &data, &theme, 0));
    /* One pixel of horizontal padding; the badge fills the 11px header band. */
    assert(matrix_framebuffer_get(&frame, 0, 0) == theme.exchange_point_color);
    assert(matrix_framebuffer_get(&frame, 0, 10) == theme.exchange_point_color);
    assert(matrix_framebuffer_get(&frame, 0, 11) == theme.line_color);
    assert(matrix_framebuffer_get(&frame, 2, 3) == theme.background_color);
    assert(matrix_framebuffer_get(&frame, 16, 5) == theme.background_color);
    assert(matrix_framebuffer_get(&frame, 40, 11) == theme.line_color);

    data.location = "9";
    assert(matrix_race_screen_render(&frame, &data, &theme, 0));
    assert(matrix_framebuffer_get(&frame, 0, 0) == theme.exchange_point_color);
    assert(matrix_framebuffer_get(&frame, 9, 5) == theme.background_color);

    data.location = "999";
    assert(matrix_race_screen_render(&frame, &data, &theme, 0));
    assert(matrix_framebuffer_get(&frame, 0, 0) == theme.exchange_point_color);
    assert(matrix_framebuffer_get(&frame, 22, 0) == theme.exchange_point_color);
    assert(matrix_framebuffer_get(&frame, 23, 5) == theme.background_color);
}

int main(void) {
    test_default_theme_uses_vbz_and_soft_white_accents();
    test_default_screen_renders_lines();
    test_exchange_point_is_inverted_without_prefix();
    test_normal_footer_keeps_vbz_spacing();
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
    test_font_test_layout_aligns_all_three_font_sizes();
    test_registration_layout_uses_full_width_without_sprites();
    test_registration_overflow_removes_footer();
    test_ready_overflow_keeps_left_half_footer();
    test_runner_and_warmup_animate_in_their_status_columns();
    test_unsupported_font_size_is_rejected();

    puts("matrix_race_screen tests passed");
    return 0;
}
