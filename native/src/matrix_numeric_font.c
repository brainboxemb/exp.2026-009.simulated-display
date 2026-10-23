#include "matrix_numeric_font.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define NUMERIC_HEIGHT 12u

typedef struct {
    uint32_t codepoint;
    uint8_t width;
    uint8_t advance;
    uint8_t rows[NUMERIC_HEIGHT];
} NumericGlyph;

/*
 * Hand-drawn 8x12 numeric glyphs for the large-value fields.
 *
 * This is deliberately not a scaled VBZ glyph set. Each digit is designed at
 * its final pixel size so round and diagonal forms remain intentional.
 */
static const NumericGlyph glyphs[] = {
    {' ', 0, 4, {0}},
    {'-', 8, 9, {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x7e, 0x7e, 0x00,
        0x00, 0x00, 0x00, 0x00
    }},
    {'0', 8, 9, {
        0x7e, 0x7e, 0xc3, 0xc3,
        0xc3, 0xc3, 0xc3, 0xc3,
        0xc3, 0xc3, 0x7e, 0x7e
    }},
    {'1', 8, 9, {
        0x18, 0x38, 0x78, 0x18,
        0x18, 0x18, 0x18, 0x18,
        0x18, 0x18, 0x7e, 0x7e
    }},
    {'2', 8, 9, {
        0x7e, 0x7e, 0x03, 0x03,
        0x06, 0x0c, 0x18, 0x30,
        0x60, 0xc0, 0xff, 0xff
    }},
    {'3', 8, 9, {
        0x7e, 0x7e, 0x03, 0x03,
        0x0e, 0x0e, 0x0e, 0x0e,
        0x03, 0x03, 0x7e, 0x7e
    }},
    {'4', 8, 9, {
        0xc3, 0xc3, 0xc3, 0xc3,
        0xc3, 0xff, 0xff, 0x03,
        0x03, 0x03, 0x03, 0x03
    }},
    {'5', 8, 9, {
        0xff, 0xff, 0xc0, 0xc0,
        0xfc, 0xfc, 0x03, 0x03,
        0x03, 0x03, 0x7e, 0x7e
    }},
    {'6', 8, 9, {
        0x7e, 0x7e, 0xc0, 0xc0,
        0xfc, 0xfc, 0xc3, 0xc3,
        0xc3, 0xc3, 0x7e, 0x7e
    }},
    {'7', 8, 9, {
        0xff, 0xff, 0x03, 0x06,
        0x0c, 0x18, 0x30, 0x60,
        0xc0, 0xc0, 0xc0, 0xc0
    }},
    {'8', 8, 9, {
        0x7e, 0x7e, 0xc3, 0xc3,
        0x7e, 0x7e, 0x7e, 0x7e,
        0xc3, 0xc3, 0x7e, 0x7e
    }},
    {'9', 8, 9, {
        0x7e, 0x7e, 0xc3, 0xc3,
        0xc3, 0xc3, 0x3f, 0x3f,
        0x03, 0x03, 0x7e, 0x7e
    }}
};

#define GLYPH_COUNT (sizeof(glyphs) / sizeof(glyphs[0]))

static int32_t glyph_index_find(uint32_t letter) {
    for (uint32_t index = 0; index < GLYPH_COUNT; index++) {
        if (glyphs[index].codepoint == letter) {
            return (int32_t)index;
        }
    }
    return -1;
}

static bool glyph_descriptor_get(
        const lv_font_t *font,
        lv_font_glyph_dsc_t *descriptor,
        uint32_t letter,
        uint32_t letter_next) {
    LV_UNUSED(font);
    LV_UNUSED(letter_next);

    const int32_t index = glyph_index_find(letter);
    if (index < 0) {
        return false;
    }

    const NumericGlyph *glyph = &glyphs[index];

    descriptor->adv_w = glyph->advance;
    descriptor->box_w = glyph->width;
    descriptor->box_h = glyph->width == 0 ? 0 : NUMERIC_HEIGHT;
    descriptor->ofs_x = 0;
    descriptor->ofs_y = 0;
    descriptor->stride = glyph->width;
    descriptor->format = LV_FONT_GLYPH_FORMAT_A8;
    descriptor->is_placeholder = false;
    descriptor->gid.index = (uint32_t)index;

    return true;
}

static const void *glyph_bitmap_get(
        lv_font_glyph_dsc_t *descriptor,
        lv_draw_buf_t *draw_buffer) {
    if (descriptor->gid.index >= GLYPH_COUNT || draw_buffer == NULL) {
        return NULL;
    }

    const NumericGlyph *glyph = &glyphs[descriptor->gid.index];

    memset(draw_buffer->data, 0, draw_buffer->data_size);

    for (uint32_t y = 0; y < NUMERIC_HEIGHT; y++) {
        for (uint32_t x = 0; x < glyph->width; x++) {
            draw_buffer->data[y * draw_buffer->header.stride + x] =
                    (glyph->rows[y] & (0x80u >> x)) != 0
                    ? 0xffu
                    : 0x00u;
        }
    }

    return draw_buffer;
}

const lv_font_t matrix_font_numeric_12 = {
    .dsc = NULL,
    .fallback = NULL,
    .user_data = NULL,
    .get_glyph_dsc = glyph_descriptor_get,
    .get_glyph_bitmap = glyph_bitmap_get,
    .release_glyph = NULL,
    .line_height = NUMERIC_HEIGHT,
    .base_line = 0,
    .cap_height = NUMERIC_HEIGHT,
    .x_height = NUMERIC_HEIGHT,
    .subpx = LV_FONT_SUBPX_NONE,
    .kerning = LV_FONT_KERNING_NONE,
    .static_bitmap = 0,
    .underline_position = -1,
    .underline_thickness = 1
};


#define NUMERIC_14_HEIGHT 14u

typedef struct {
    uint32_t codepoint;
    uint8_t width;
    uint8_t advance;
    uint16_t rows[NUMERIC_14_HEIGHT];
} NumericGlyph14;

/*
 * Hand-drawn 9x14 numeric glyphs for the large-value fields.
 *
 * They are intentionally designed at 14 pixels rather than scaling the 12px
 * set, so two-pixel horizontal strokes remain explicit.
 */
static const NumericGlyph14 glyphs_14[] = {
    {' ', 0, 4, {0}},
    {'-', 9, 10, {
        0x000, 0x000, 0x000, 0x000,
        0x000, 0x0fe, 0x0fe, 0x000,
        0x000, 0x000, 0x000, 0x000,
        0x000, 0x000
    }},
    {'0', 9, 10, {
        0x0fe, 0x0fe, 0x183, 0x183,
        0x183, 0x183, 0x183, 0x183,
        0x183, 0x183, 0x183, 0x183,
        0x0fe, 0x0fe
    }},
    {'1', 9, 10, {
        0x018, 0x038, 0x078, 0x018,
        0x018, 0x018, 0x018, 0x018,
        0x018, 0x018, 0x018, 0x018,
        0x0fe, 0x0fe
    }},
    {'2', 9, 10, {
        0x0fe, 0x0fe, 0x003, 0x003,
        0x006, 0x00c, 0x018, 0x030,
        0x060, 0x0c0, 0x180, 0x180,
        0x1ff, 0x1ff
    }},
    {'3', 9, 10, {
        0x0fe, 0x0fe, 0x003, 0x003,
        0x003, 0x01e, 0x01e, 0x003,
        0x003, 0x003, 0x003, 0x003,
        0x0fe, 0x0fe
    }},
    {'4', 9, 10, {
        0x183, 0x183, 0x183, 0x183,
        0x183, 0x1ff, 0x1ff, 0x003,
        0x003, 0x003, 0x003, 0x003,
        0x003, 0x003
    }},
    {'5', 9, 10, {
        0x1ff, 0x1ff, 0x180, 0x180,
        0x180, 0x1fc, 0x1fc, 0x003,
        0x003, 0x003, 0x003, 0x003,
        0x0fe, 0x0fe
    }},
    {'6', 9, 10, {
        0x0fe, 0x0fe, 0x180, 0x180,
        0x180, 0x1fc, 0x1fc, 0x183,
        0x183, 0x183, 0x183, 0x183,
        0x0fe, 0x0fe
    }},
    {'7', 9, 10, {
        0x1ff, 0x1ff, 0x003, 0x006,
        0x00c, 0x018, 0x030, 0x060,
        0x0c0, 0x180, 0x180, 0x180,
        0x180, 0x180
    }},
    {'8', 9, 10, {
        0x0fe, 0x0fe, 0x183, 0x183,
        0x183, 0x183, 0x0fe, 0x0fe,
        0x183, 0x183, 0x183, 0x183,
        0x0fe, 0x0fe
    }},
    {'9', 9, 10, {
        0x0fe, 0x0fe, 0x183, 0x183,
        0x183, 0x183, 0x07f, 0x07f,
        0x003, 0x003, 0x003, 0x003,
        0x0fe, 0x0fe
    }}
};

#define GLYPH_14_COUNT (sizeof(glyphs_14) / sizeof(glyphs_14[0]))

static int32_t glyph_14_index_find(uint32_t letter) {
    for (uint32_t index = 0; index < GLYPH_14_COUNT; index++) {
        if (glyphs_14[index].codepoint == letter) {
            return (int32_t)index;
        }
    }

    return -1;
}

static bool glyph_14_descriptor_get(
        const lv_font_t *font,
        lv_font_glyph_dsc_t *descriptor,
        uint32_t letter,
        uint32_t letter_next) {
    LV_UNUSED(font);
    LV_UNUSED(letter_next);

    const int32_t index =
            glyph_14_index_find(letter);
    if (index < 0) {
        return false;
    }

    const NumericGlyph14 *glyph =
            &glyphs_14[index];

    descriptor->adv_w = glyph->advance;
    descriptor->box_w = glyph->width;
    descriptor->box_h =
            glyph->width == 0
                    ? 0
                    : NUMERIC_14_HEIGHT;
    descriptor->ofs_x = 0;
    descriptor->ofs_y = 0;
    descriptor->stride = glyph->width;
    descriptor->format = LV_FONT_GLYPH_FORMAT_A8;
    descriptor->is_placeholder = false;
    descriptor->gid.index = (uint32_t)index;

    return true;
}

static const void *glyph_14_bitmap_get(
        lv_font_glyph_dsc_t *descriptor,
        lv_draw_buf_t *draw_buffer) {
    if (descriptor->gid.index >= GLYPH_14_COUNT
            || draw_buffer == NULL) {
        return NULL;
    }

    const NumericGlyph14 *glyph =
            &glyphs_14[descriptor->gid.index];

    memset(
            draw_buffer->data,
            0,
            draw_buffer->data_size);

    for (uint32_t y = 0;
            y < NUMERIC_14_HEIGHT;
            y++) {
        for (uint32_t x = 0;
                x < glyph->width;
                x++) {
            draw_buffer->data[
                    y * draw_buffer->header.stride
                    + x] =
                    (glyph->rows[y]
                            & (1u << (glyph->width - 1u - x)))
                    != 0
                    ? 0xffu
                    : 0x00u;
        }
    }

    return draw_buffer;
}

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
