#include "matrix_race_screen.h"

#include "matrix_font.h"
#include "matrix_sprite.h"
#include "lvgl.h"

#include <stddef.h>
#include <stdint.h>

#define HEADER_LINE_Y 11
#define HEADER_TEXT_Y -1
#define MAIN_COLUMN_X 81
#define FOOTER_LINE_Y 52

#define HEADER_LOCATION_X 1
#define HEADER_LOCATION_WIDTH 38
#define HEADER_CLOCK_X 40
#define HEADER_CLOCK_WIDTH 80
#define HEADER_CLOCK_GAP 1u
#define FINISH_FLAG_Y 1
#define FINISH_FLAG_WIDTH 7u
#define FINISH_FLAG_HEIGHT 7u

#define HEADER_COUNTER_WIDTH 21u

#define WARNING_Y 0
#define WARNING_WIDTH 11u
#define WARNING_GAP 1u
#define WARNING_HEIGHT 11u
#define WARNING_GLYPH_WIDTH 11u
#define WARNING_GLYPH_HEIGHT 11u

#define LEFT_SPRITE_X 1
#define LEFT_NUMBER_X 12
#define LEFT_NUMBER_WIDTH 21
#define LEFT_TIME_X 33
#define LEFT_TIME_WIDTH 55

#define RIGHT_SPRITE_X 92
#define RIGHT_VALUE_X 103
#define RIGHT_VALUE_WIDTH 27
#define RIGHT_READY_X 133
#define RIGHT_READY_WIDTH 27

#define FOOTER_Y 54

static lv_display_t *display;
static MatrixFramebuffer *active_framebuffer;
static bool lvgl_initialized;

/*
 * Static one-pixel checker flag. At this resolution a literal 1x1 checker
 * remains clearer than a simulated wave.
 */
static const uint8_t FINISH_FLAG[FINISH_FLAG_HEIGHT] = {
    0x55,
    0x2a,
    0x55,
    0x2a,
    0x55,
    0x2a,
    0x55
};

/*
 * WiFi unavailable: a broad signal bar, a narrower bar below it and one
 * diagonal strike-through.
 */
static const uint16_t WARNING_WIFI[WARNING_GLYPH_HEIGHT] = {
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

/*
 * Registration server unavailable: two diagonal chain links, based on the
 * conventional link icon. The links are offset from each other instead of
 * forming two vertically stacked loops.
 */
static const uint16_t WARNING_LINK[WARNING_GLYPH_HEIGHT] = {
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

/*
 * Two-pixel diagonal strike-through across the complete warning block.
 * Both outer corners are part of the strike.
 */
static const uint16_t WARNING_STRIKE[WARNING_GLYPH_HEIGHT] = {
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

/*
 * Synchronizing: two compact curved arrows centered inside the 11x11 status
 * block. The surrounding green margin keeps the symbol from reading as one
 * large circle. The upper arc points right and the lower arc points left.
 * This status is positive/active rather than a warning, so it uses the sync
 * colors and no strike-through.
 */
static const uint16_t STATUS_SYNC[WARNING_GLYPH_HEIGHT] = {
    0x000,
    0x000,
    0x1c0,
    0x220,
    0x070,
    0x038,
    0x180,
    0x110,
    0x0e0,
    0x000,
    0x000
};

static void draw_finish_flag(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        MatrixPixel color) {
    for (uint16_t row = 0;
            row < FINISH_FLAG_HEIGHT;
            row++) {
        for (uint16_t column = 0;
                column < FINISH_FLAG_WIDTH;
                column++) {
            const uint16_t mask =
                    (uint16_t)(
                            1u
                            << (FINISH_FLAG_WIDTH
                                    - 1u
                                    - column));

            if ((FINISH_FLAG[row] & mask) == 0) {
                continue;
            }

            matrix_framebuffer_set(
                    framebuffer,
                    (uint16_t)(x + column),
                    (uint16_t)(FINISH_FLAG_Y + row),
                    color);
        }
    }
}

static void draw_status_block(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        const uint16_t glyph[WARNING_GLYPH_HEIGHT],
        MatrixPixel background_color,
        MatrixPixel icon_color,
        bool strike,
        MatrixPixel strike_color) {
    for (uint16_t row = 0;
            row < WARNING_HEIGHT;
            row++) {
        for (uint16_t column = 0;
                column < WARNING_WIDTH;
                column++) {
            matrix_framebuffer_set(
                    framebuffer,
                    (uint16_t)(x + column),
                    (uint16_t)(WARNING_Y + row),
                    background_color);
        }
    }

    for (uint16_t row = 0;
            row < WARNING_GLYPH_HEIGHT;
            row++) {
        for (uint16_t column = 0;
                column < WARNING_GLYPH_WIDTH;
                column++) {
            const uint16_t mask =
                    (uint16_t)(
                            1u
                            << (WARNING_GLYPH_WIDTH
                                    - 1u
                                    - column));

            if ((glyph[row] & mask) != 0) {
                matrix_framebuffer_set(
                        framebuffer,
                        (uint16_t)(x + column),
                        (uint16_t)(WARNING_Y + row),
                        icon_color);
            }

            if (strike
                    && (WARNING_STRIKE[row] & mask) != 0) {
                matrix_framebuffer_set(
                        framebuffer,
                        (uint16_t)(x + column),
                        (uint16_t)(WARNING_Y + row),
                        strike_color);
            }
        }
    }
}

static lv_color_t lv_color_from_pixel(MatrixPixel pixel) {
    const uint8_t red =
            (uint8_t)((((pixel >> 11u) & 0x1fu) * 255u) / 31u);
    const uint8_t green =
            (uint8_t)((((pixel >> 5u) & 0x3fu) * 255u) / 63u);
    const uint8_t blue =
            (uint8_t)(((pixel & 0x1fu) * 255u) / 31u);

    return lv_color_make(red, green, blue);
}

static void display_flush(
        lv_display_t *display_to_flush,
        const lv_area_t *area,
        uint8_t *pixels) {
    LV_UNUSED(area);
    LV_UNUSED(pixels);
    lv_display_flush_ready(display_to_flush);
}

static void ensure_display(MatrixFramebuffer *framebuffer) {
    if (!lvgl_initialized) {
        lv_init();
        lvgl_initialized = true;

        display =
                lv_display_create(
                        MATRIX_DISPLAY_WIDTH,
                        MATRIX_DISPLAY_HEIGHT);
        lv_display_set_color_format(
                display,
                LV_COLOR_FORMAT_RGB565);
        lv_display_set_flush_cb(
                display,
                display_flush);
    }

    if (active_framebuffer != framebuffer) {
        active_framebuffer = framebuffer;
        lv_display_set_buffers(
                display,
                framebuffer->pixels,
                NULL,
                sizeof(framebuffer->pixels),
                LV_DISPLAY_RENDER_MODE_DIRECT);
    }

    lv_display_set_default(display);
}

static void style_screen(
        lv_obj_t *screen,
        const MatrixTheme *theme) {
    lv_obj_remove_style_all(screen);
    lv_obj_set_style_bg_color(
            screen,
            lv_color_from_pixel(theme->background_color),
            0);
    lv_obj_set_style_bg_opa(
            screen,
            LV_OPA_COVER,
            0);
    lv_obj_set_style_pad_all(
            screen,
            0,
            0);
    lv_obj_set_scrollable(
            screen,
            false);
}

static void create_line(
        lv_obj_t *screen,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height,
        MatrixPixel color) {
    lv_obj_t *line = lv_obj_create(screen);

    lv_obj_remove_style_all(line);
    lv_obj_set_pos(line, x, y);
    lv_obj_set_size(line, width, height);
    lv_obj_set_style_bg_color(
            line,
            lv_color_from_pixel(color),
            0);
    lv_obj_set_style_bg_opa(
            line,
            LV_OPA_COVER,
            0);
}

static lv_obj_t *create_label(
        lv_obj_t *screen,
        const char *text,
        int32_t x,
        int32_t y,
        int32_t width,
        lv_text_align_t alignment,
        const MatrixTextStyle *style,
        const lv_font_t *font) {
    lv_obj_t *label = lv_label_create(screen);

    lv_obj_remove_style_all(label);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(
            label,
            lv_color_from_pixel(style->color),
            0);
    lv_obj_set_style_text_align(
            label,
            alignment,
            0);
    lv_label_set_text(
            label,
            text != NULL ? text : "");

    return label;
}

bool matrix_race_screen_render(
        MatrixFramebuffer *framebuffer,
        const MatrixRaceScreenData *data,
        const MatrixTheme *theme,
        uint8_t animation_frame) {
    if (framebuffer == NULL
            || data == NULL
            || theme == NULL) {
        return false;
    }

    const lv_font_t *header_font =
            matrix_font_resolve(theme->header.font);
    const lv_font_t *row_number_font =
            matrix_font_resolve(theme->row_number.font);
    const lv_font_t *row_time_font =
            matrix_font_resolve(theme->row_time.font);
    const lv_font_t *large_value_font =
            matrix_large_font_resolve(theme->large_value.font);
    const lv_font_t *footer_font =
            matrix_font_resolve(theme->footer.font);

    if (header_font == NULL
            || row_number_font == NULL
            || row_time_font == NULL
            || large_value_font == NULL
            || footer_font == NULL) {
        return false;
    }

    ensure_display(framebuffer);

    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    style_screen(screen, theme);

    create_line(
            screen,
            0,
            HEADER_LINE_Y,
            MATRIX_DISPLAY_WIDTH,
            1,
            theme->line_color);
    create_line(
            screen,
            MAIN_COLUMN_X,
            HEADER_LINE_Y,
            1,
            MATRIX_DISPLAY_HEIGHT - HEADER_LINE_Y,
            theme->line_color);
    create_line(
            screen,
            0,
            FOOTER_LINE_Y,
            MAIN_COLUMN_X,
            1,
            theme->line_color);

    create_label(
            screen,
            data->location,
            HEADER_LOCATION_X,
            HEADER_TEXT_Y,
            HEADER_LOCATION_WIDTH,
            LV_TEXT_ALIGN_LEFT,
            &theme->header,
            header_font);
    uint16_t status_count = 0;

    if (!data->wifi_connected) {
        status_count++;
    }

    if (!data->registration_server_connected) {
        status_count++;
    }

    if (data->synchronizing) {
        status_count++;
    }

    const uint16_t status_group_width =
            status_count == 0u
                    ? 0u
                    : (uint16_t)(
                            status_count * WARNING_WIDTH
                            + (status_count - 1u) * WARNING_GAP);
    const uint16_t counter_right =
            (uint16_t)(
                    MATRIX_DISPLAY_WIDTH
                    - status_group_width);
    const uint16_t counter_x =
            (uint16_t)(
                    counter_right
                    - HEADER_COUNTER_WIDTH);
    const uint16_t finish_flag_x =
            (uint16_t)(
                    counter_x
                    - FINISH_FLAG_WIDTH);
    const uint16_t clock_available_width =
            finish_flag_x > HEADER_CLOCK_X + HEADER_CLOCK_GAP
                    ? (uint16_t)(
                            finish_flag_x
                            - HEADER_CLOCK_X
                            - HEADER_CLOCK_GAP)
                    : 0u;
    const uint16_t clock_width =
            clock_available_width < HEADER_CLOCK_WIDTH
                    ? clock_available_width
                    : HEADER_CLOCK_WIDTH;

    create_label(
            screen,
            data->clock,
            HEADER_CLOCK_X,
            HEADER_TEXT_Y,
            clock_width,
            LV_TEXT_ALIGN_CENTER,
            &theme->header,
            header_font);
    create_label(
            screen,
            data->counter,
            counter_x,
            HEADER_TEXT_Y,
            HEADER_COUNTER_WIDTH,
            LV_TEXT_ALIGN_RIGHT,
            &theme->header,
            header_font);

    static const int32_t left_row_y[] = {
        13,
        26,
        39
    };

    for (size_t index = 0;
            index < MATRIX_RACE_LEFT_ROW_COUNT;
            index++) {
        create_label(
                screen,
                data->left_rows[index].number,
                LEFT_NUMBER_X,
                left_row_y[index],
                LEFT_NUMBER_WIDTH,
                LV_TEXT_ALIGN_RIGHT,
                &theme->row_number,
                row_number_font);

        create_label(
                screen,
                data->left_rows[index].time,
                LEFT_TIME_X,
                left_row_y[index],
                LEFT_TIME_WIDTH,
                LV_TEXT_ALIGN_RIGHT,
                &theme->row_time,
                row_time_font);
    }

    static const int32_t right_row_y[] = {
        14,
        36
    };

    for (size_t index = 0;
            index < MATRIX_RACE_RIGHT_ROW_COUNT;
            index++) {
        create_label(
                screen,
                data->right_rows[index].left,
                RIGHT_VALUE_X,
                right_row_y[index],
                RIGHT_VALUE_WIDTH,
                LV_TEXT_ALIGN_CENTER,
                &theme->large_value,
                large_value_font);

        create_label(
                screen,
                data->right_rows[index].right,
                RIGHT_READY_X,
                right_row_y[index],
                RIGHT_READY_WIDTH,
                LV_TEXT_ALIGN_CENTER,
                &theme->large_value,
                large_value_font);
    }

    lv_obj_t *footer =
            create_label(
                    screen,
                    data->footer,
                    1,
                    FOOTER_Y,
                    MAIN_COLUMN_X - 2,
                    LV_TEXT_ALIGN_LEFT,
                    &theme->footer,
                    footer_font);

    lv_obj_set_style_text_letter_space(
            footer,
            -1,
            0);
    lv_label_set_long_mode(
            footer,
            LV_LABEL_LONG_CLIP);

    lv_obj_invalidate(screen);
    lv_refr_now(display);

    draw_finish_flag(
            framebuffer,
            finish_flag_x,
            theme->flag_color);

    uint16_t status_x =
            (uint16_t)(
                    MATRIX_DISPLAY_WIDTH
                    - status_group_width);

    if (!data->wifi_connected) {
        draw_status_block(
                framebuffer,
                status_x,
                WARNING_WIFI,
                theme->warning_background_color,
                theme->warning_text_color,
                true,
                theme->warning_strike_color);
        status_x =
                (uint16_t)(
                        status_x
                        + WARNING_WIDTH
                        + WARNING_GAP);
    }

    if (!data->registration_server_connected) {
        draw_status_block(
                framebuffer,
                status_x,
                WARNING_LINK,
                theme->warning_background_color,
                theme->warning_text_color,
                true,
                theme->warning_strike_color);
        status_x =
                (uint16_t)(
                        status_x
                        + WARNING_WIDTH
                        + WARNING_GAP);
    }

    if (data->synchronizing) {
        draw_status_block(
                framebuffer,
                status_x,
                STATUS_SYNC,
                theme->sync_background_color,
                theme->sync_icon_color,
                false,
                theme->sync_icon_color);
    }

    matrix_sprite_draw(
            framebuffer,
            MATRIX_SPRITE_RUNNER,
            animation_frame,
            LEFT_SPRITE_X,
            left_row_y[0],
            theme->sprite_color);
    matrix_sprite_draw(
            framebuffer,
            MATRIX_SPRITE_WARMUP,
            (uint8_t)(animation_frame / 2u),
            RIGHT_SPRITE_X,
            right_row_y[0],
            theme->sprite_color);

    return true;
}
