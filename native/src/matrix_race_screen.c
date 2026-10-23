#include "matrix_race_screen.h"

#include "matrix_font.h"
#include "lvgl.h"

#include <stddef.h>
#include <stdint.h>

#define HEADER_LINE_Y 11
#define HEADER_TEXT_Y -1
#define MAIN_COLUMN_X 90
#define FOOTER_LINE_Y 52

#define HEADER_LOCATION_X 1
#define HEADER_LOCATION_WIDTH 38
#define HEADER_CLOCK_X 40
#define HEADER_CLOCK_WIDTH 80
#define HEADER_COUNTER_X 121
#define HEADER_COUNTER_WIDTH 38

#define LEFT_NUMBER_X 1
#define LEFT_NUMBER_WIDTH 28
#define LEFT_TIME_X 33
#define LEFT_TIME_WIDTH 55

#define RIGHT_X 92
#define RIGHT_LEFT_WIDTH 32
#define RIGHT_RIGHT_X 126
#define RIGHT_RIGHT_WIDTH 33

#define FOOTER_Y 53

static lv_display_t *display;
static MatrixFramebuffer *active_framebuffer;
static bool lvgl_initialized;

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
        const MatrixTheme *theme) {
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
    create_label(
            screen,
            data->clock,
            HEADER_CLOCK_X,
            HEADER_TEXT_Y,
            HEADER_CLOCK_WIDTH,
            LV_TEXT_ALIGN_CENTER,
            &theme->header,
            header_font);
    create_label(
            screen,
            data->counter,
            HEADER_COUNTER_X,
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
                RIGHT_X,
                right_row_y[index],
                RIGHT_LEFT_WIDTH,
                LV_TEXT_ALIGN_CENTER,
                &theme->large_value,
                large_value_font);

        create_label(
                screen,
                data->right_rows[index].right,
                RIGHT_RIGHT_X,
                right_row_y[index],
                RIGHT_RIGHT_WIDTH,
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
    return true;
}
