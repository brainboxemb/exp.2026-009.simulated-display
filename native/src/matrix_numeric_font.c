#include "matrix_numeric_font.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SOURCE_WIDTH 7u
#define SOURCE_HEIGHT 9u

#define NUMERIC_12_WIDTH 9u
#define NUMERIC_12_HEIGHT 12u

#define NUMERIC_14_WIDTH 10u
#define NUMERIC_14_HEIGHT 14u

typedef struct {
    uint32_t codepoint;
    uint8_t rows[SOURCE_HEIGHT];
} NumericSourceGlyph;

/*
 * Literal 7x9 digit topology from the original VBZ font.
 *
 * The larger numeric fonts are generated from this one source in both axes.
 * That preserves the characteristic VBZ shapes instead of independently
 * redesigning the 12px and 14px variants.
 */
static const NumericSourceGlyph source_glyphs[] = {
    {' ', {0}},
    {'-', {
        0x00, 0x00, 0x00,
        0x00, 0x7e, 0x00,
        0x00, 0x00, 0x00
    }},
    {'0', {
        0x3c, 0x66, 0x66,
        0x66, 0x66, 0x66,
        0x66, 0x66, 0x3c
    }},
    {'1', {
        0x18, 0x38, 0x78,
        0x18, 0x18, 0x18,
        0x18, 0x18, 0x18
    }},
    {'2', {
        0x3c, 0x66, 0x66,
        0x06, 0x0c, 0x18,
        0x30, 0x66, 0x7e
    }},
    {'3', {
        0x3c, 0x66, 0x66,
        0x06, 0x0c, 0x06,
        0x66, 0x66, 0x3c
    }},
    {'4', {
        0x60, 0x60, 0x60,
        0x6c, 0x6c, 0x7e,
        0x0c, 0x0c, 0x0c
    }},
    {'5', {
        0x7e, 0x66, 0x60,
        0x60, 0x7c, 0x06,
        0x06, 0x66, 0x3c
    }},
    {'6', {
        0x3c, 0x66, 0x60,
        0x60, 0x7c, 0x66,
        0x66, 0x66, 0x3c
    }},
    {'7', {
        0x7e, 0x66, 0x06,
        0x0c, 0x18, 0x30,
        0x60, 0x60, 0x60
    }},
    {'8', {
        0x3c, 0x66, 0x66,
        0x66, 0x3c, 0x66,
        0x66, 0x66, 0x3c
    }},
    {'9', {
        0x3c, 0x66, 0x66,
        0x66, 0x3e, 0x06,
        0x06, 0x66, 0x3c
    }}
};

#define SOURCE_GLYPH_COUNT \
        (sizeof(source_glyphs) / sizeof(source_glyphs[0]))

static int32_t source_glyph_index_find(uint32_t letter) {
    for (uint32_t index = 0;
            index < SOURCE_GLYPH_COUNT;
            index++) {
        if (source_glyphs[index].codepoint == letter) {
            return (int32_t)index;
        }
    }

    return -1;
}

static bool glyph_descriptor_get(
        lv_font_glyph_dsc_t *descriptor,
        uint32_t letter,
        uint8_t width,
        uint8_t height) {
    const int32_t index =
            source_glyph_index_find(letter);
    if (index < 0) {
        return false;
    }

    const bool blank =
            letter == ' ';

    descriptor->adv_w =
            blank
                    ? (uint16_t)(width / 2u)
                    : width;
    descriptor->box_w =
            blank
                    ? 0
                    : width;
    descriptor->box_h =
            blank
                    ? 0
                    : height;
    descriptor->ofs_x = 0;
    descriptor->ofs_y = 0;
    descriptor->stride =
            blank
                    ? 0
                    : width;
    descriptor->format = LV_FONT_GLYPH_FORMAT_A8;
    descriptor->is_placeholder = false;
    descriptor->gid.index = (uint32_t)index;

    return true;
}

static const void *glyph_bitmap_get(
        lv_font_glyph_dsc_t *descriptor,
        lv_draw_buf_t *draw_buffer,
        uint8_t target_width,
        uint8_t target_height) {
    if (descriptor->gid.index >= SOURCE_GLYPH_COUNT
            || draw_buffer == NULL) {
        return NULL;
    }

    const NumericSourceGlyph *glyph =
            &source_glyphs[descriptor->gid.index];

    memset(
            draw_buffer->data,
            0,
            draw_buffer->data_size);

    for (uint32_t y = 0;
            y < target_height;
            y++) {
        const uint32_t source_y =
                (y * SOURCE_HEIGHT)
                / target_height;

        for (uint32_t x = 0;
                x < target_width;
                x++) {
            const uint32_t source_x =
                    (x * SOURCE_WIDTH)
                    / target_width;
            const uint8_t source_mask =
                    (uint8_t)(
                            1u
                            << (SOURCE_WIDTH
                                    - 1u
                                    - source_x));

            draw_buffer->data[
                    y * draw_buffer->header.stride
                    + x] =
                    (glyph->rows[source_y]
                            & source_mask)
                    != 0
                    ? 0xffu
                    : 0x00u;
        }
    }

    return draw_buffer;
}

static bool glyph_12_descriptor_get(
        const lv_font_t *font,
        lv_font_glyph_dsc_t *descriptor,
        uint32_t letter,
        uint32_t letter_next) {
    LV_UNUSED(font);
    LV_UNUSED(letter_next);

    return glyph_descriptor_get(
            descriptor,
            letter,
            NUMERIC_12_WIDTH,
            NUMERIC_12_HEIGHT);
}

static const void *glyph_12_bitmap_get(
        lv_font_glyph_dsc_t *descriptor,
        lv_draw_buf_t *draw_buffer) {
    return glyph_bitmap_get(
            descriptor,
            draw_buffer,
            NUMERIC_12_WIDTH,
            NUMERIC_12_HEIGHT);
}

static bool glyph_14_descriptor_get(
        const lv_font_t *font,
        lv_font_glyph_dsc_t *descriptor,
        uint32_t letter,
        uint32_t letter_next) {
    LV_UNUSED(font);
    LV_UNUSED(letter_next);

    return glyph_descriptor_get(
            descriptor,
            letter,
            NUMERIC_14_WIDTH,
            NUMERIC_14_HEIGHT);
}

static const void *glyph_14_bitmap_get(
        lv_font_glyph_dsc_t *descriptor,
        lv_draw_buf_t *draw_buffer) {
    return glyph_bitmap_get(
            descriptor,
            draw_buffer,
            NUMERIC_14_WIDTH,
            NUMERIC_14_HEIGHT);
}

const lv_font_t matrix_font_numeric_12 = {
    .dsc = NULL,
    .fallback = NULL,
    .user_data = NULL,
    .get_glyph_dsc = glyph_12_descriptor_get,
    .get_glyph_bitmap = glyph_12_bitmap_get,
    .release_glyph = NULL,
    .line_height = NUMERIC_12_HEIGHT,
    .base_line = 0,
    .cap_height = NUMERIC_12_HEIGHT,
    .x_height = NUMERIC_12_HEIGHT,
    .subpx = LV_FONT_SUBPX_NONE,
    .kerning = LV_FONT_KERNING_NONE,
    .static_bitmap = 0,
    .underline_position = -1,
    .underline_thickness = 1
};

const lv_font_t matrix_font_numeric_14 = {
    .dsc = NULL,
    .fallback = NULL,
    .user_data = NULL,
    .get_glyph_dsc = glyph_14_descriptor_get,
    .get_glyph_bitmap = glyph_14_bitmap_get,
    .release_glyph = NULL,
    .line_height = NUMERIC_14_HEIGHT,
    .base_line = 0,
    .cap_height = NUMERIC_14_HEIGHT,
    .x_height = NUMERIC_14_HEIGHT,
    .subpx = LV_FONT_SUBPX_NONE,
    .kerning = LV_FONT_KERNING_NONE,
    .static_bitmap = 0,
    .underline_position = -1,
    .underline_thickness = 1
};
