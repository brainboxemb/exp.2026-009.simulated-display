#include "matrix_numeric_font.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define NUMERIC_12_HEIGHT 12u
#define NUMERIC_14_HEIGHT 14u

typedef struct {
    uint32_t codepoint;
    uint8_t width;
    uint8_t advance;
    uint16_t rows[NUMERIC_12_HEIGHT];
} NumericGlyph12;

typedef struct {
    uint32_t codepoint;
    uint8_t width;
    uint8_t advance;
    uint16_t rows[NUMERIC_14_HEIGHT];
} NumericGlyph14;

/*
 * Fixed-size numeric variants derived from the original VBZ digit shapes.
 *
 * These are deliberately not nearest-neighbour scaled. Automatic 7->9/10
 * column expansion created asymmetric three-pixel strokes. Instead each final
 * glyph is authored at its actual size with two-pixel stems and symmetric
 * curves while retaining the original VBZ topology.
 */
static const NumericGlyph12 glyphs_12[] = {
    {' ', 0, 4, {0}},
    {'-', 9, 9, {
        0x000, 0x000, 0x000, 0x000,
        0x000, 0x0fe, 0x0fe, 0x000,
        0x000, 0x000, 0x000, 0x000
    }},
    {'0', 9, 9, {
        0x07c, 0x0c6, 0x183, 0x183,
        0x183, 0x183, 0x183, 0x183,
        0x183, 0x183, 0x0c6, 0x07c
    }},
    {'1', 9, 9, {
        0x030, 0x070, 0x0f0, 0x030,
        0x030, 0x030, 0x030, 0x030,
        0x030, 0x030, 0x030, 0x030
    }},
    {'2', 9, 9, {
        0x07c, 0x0c6, 0x186, 0x006,
        0x00c, 0x018, 0x030, 0x060,
        0x0c0, 0x186, 0x1fe, 0x1fe
    }},
    {'3', 9, 9, {
        0x07c, 0x0c6, 0x186, 0x006,
        0x00c, 0x018, 0x00c, 0x006,
        0x186, 0x186, 0x0c6, 0x07c
    }},
    {'4', 9, 9, {
        0x180, 0x180, 0x180, 0x198,
        0x198, 0x198, 0x1fe, 0x1fe,
        0x018, 0x018, 0x018, 0x018
    }},
    {'5', 9, 9, {
        0x1fe, 0x1fe, 0x186, 0x180,
        0x180, 0x1fc, 0x1fc, 0x006,
        0x006, 0x186, 0x0c6, 0x07c
    }},
    {'6', 9, 9, {
        0x07c, 0x0c6, 0x186, 0x180,
        0x180, 0x1fc, 0x1fc, 0x186,
        0x186, 0x186, 0x0c6, 0x07c
    }},
    {'7', 9, 9, {
        0x1fe, 0x1fe, 0x186, 0x006,
        0x00c, 0x018, 0x030, 0x060,
        0x0c0, 0x180, 0x180, 0x180
    }},
    {'8', 9, 9, {
        0x07c, 0x0c6, 0x186, 0x186,
        0x0c6, 0x07c, 0x0c6, 0x186,
        0x186, 0x186, 0x0c6, 0x07c
    }},
    {'9', 9, 9, {
        0x07c, 0x0c6, 0x186, 0x186,
        0x186, 0x0fc, 0x07c, 0x006,
        0x006, 0x186, 0x0c6, 0x07c
    }}
};

static const NumericGlyph14 glyphs_14[] = {
    {' ', 0, 4, {0}},
    {'-', 10, 10, {
        0x000, 0x000, 0x000, 0x000,
        0x000, 0x000, 0x1fe, 0x1fe,
        0x000, 0x000, 0x000, 0x000,
        0x000, 0x000
    }},
    {'0', 10, 10, {
        0x0fc, 0x186, 0x303, 0x303,
        0x303, 0x303, 0x303, 0x303,
        0x303, 0x303, 0x303, 0x303,
        0x186, 0x0fc
    }},
    {'1', 10, 10, {
        0x060, 0x0e0, 0x1e0, 0x060,
        0x060, 0x060, 0x060, 0x060,
        0x060, 0x060, 0x060, 0x060,
        0x060, 0x060
    }},
    {'2', 10, 10, {
        0x0fc, 0x186, 0x306, 0x006,
        0x00c, 0x018, 0x030, 0x060,
        0x0c0, 0x180, 0x306, 0x30c,
        0x3fe, 0x3fe
    }},
    {'3', 10, 10, {
        0x0fc, 0x186, 0x306, 0x006,
        0x00c, 0x018, 0x00c, 0x006,
        0x006, 0x306, 0x306, 0x30c,
        0x186, 0x0fc
    }},
    {'4', 10, 10, {
        0x300, 0x300, 0x300, 0x318,
        0x318, 0x318, 0x318, 0x3fe,
        0x3fe, 0x018, 0x018, 0x018,
        0x018, 0x018
    }},
    {'5', 10, 10, {
        0x3fe, 0x3fe, 0x306, 0x300,
        0x300, 0x300, 0x3fc, 0x3fc,
        0x006, 0x006, 0x006, 0x306,
        0x186, 0x0fc
    }},
    {'6', 10, 10, {
        0x0fc, 0x186, 0x306, 0x300,
        0x300, 0x300, 0x3fc, 0x3fc,
        0x306, 0x306, 0x306, 0x306,
        0x186, 0x0fc
    }},
    {'7', 10, 10, {
        0x3fe, 0x3fe, 0x306, 0x006,
        0x00c, 0x018, 0x030, 0x060,
        0x0c0, 0x180, 0x300, 0x300,
        0x300, 0x300
    }},
    {'8', 10, 10, {
        0x0fc, 0x186, 0x306, 0x306,
        0x306, 0x186, 0x0fc, 0x186,
        0x306, 0x306, 0x306, 0x306,
        0x186, 0x0fc
    }},
    {'9', 10, 10, {
        0x0fc, 0x186, 0x306, 0x306,
        0x306, 0x306, 0x1fc, 0x0fc,
        0x006, 0x006, 0x006, 0x306,
        0x186, 0x0fc
    }}
};

#define GLYPH_12_COUNT \
        (sizeof(glyphs_12) / sizeof(glyphs_12[0]))
#define GLYPH_14_COUNT \
        (sizeof(glyphs_14) / sizeof(glyphs_14[0]))

static int32_t glyph_12_index_find(uint32_t letter) {
    for (uint32_t index = 0;
            index < GLYPH_12_COUNT;
            index++) {
        if (glyphs_12[index].codepoint == letter) {
            return (int32_t)index;
        }
    }

    return -1;
}

static int32_t glyph_14_index_find(uint32_t letter) {
    for (uint32_t index = 0;
            index < GLYPH_14_COUNT;
            index++) {
        if (glyphs_14[index].codepoint == letter) {
            return (int32_t)index;
        }
    }

    return -1;
}

static bool glyph_descriptor_set(
        lv_font_glyph_dsc_t *descriptor,
        uint32_t index,
        uint8_t width,
        uint8_t advance,
        uint8_t height) {
    descriptor->adv_w = advance;
    descriptor->box_w = width;
    descriptor->box_h =
            width == 0
                    ? 0
                    : height;
    descriptor->ofs_x = 0;
    descriptor->ofs_y = 0;
    descriptor->stride = width;
    descriptor->format = LV_FONT_GLYPH_FORMAT_A8;
    descriptor->is_placeholder = false;
    descriptor->gid.index = index;

    return true;
}

static bool glyph_12_descriptor_get(
        const lv_font_t *font,
        lv_font_glyph_dsc_t *descriptor,
        uint32_t letter,
        uint32_t letter_next) {
    LV_UNUSED(font);
    LV_UNUSED(letter_next);

    const int32_t index =
            glyph_12_index_find(letter);
    if (index < 0) {
        return false;
    }

    const NumericGlyph12 *glyph =
            &glyphs_12[index];

    return glyph_descriptor_set(
            descriptor,
            (uint32_t)index,
            glyph->width,
            glyph->advance,
            NUMERIC_12_HEIGHT);
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

    return glyph_descriptor_set(
            descriptor,
            (uint32_t)index,
            glyph->width,
            glyph->advance,
            NUMERIC_14_HEIGHT);
}

static void bitmap_rows_draw(
        const uint16_t *rows,
        uint8_t width,
        uint8_t height,
        lv_draw_buf_t *draw_buffer) {
    memset(
            draw_buffer->data,
            0,
            draw_buffer->data_size);

    for (uint32_t y = 0;
            y < height;
            y++) {
        for (uint32_t x = 0;
                x < width;
                x++) {
            draw_buffer->data[
                    y * draw_buffer->header.stride
                    + x] =
                    (rows[y]
                            & (1u << (width - 1u - x)))
                    != 0
                    ? 0xffu
                    : 0x00u;
        }
    }
}

static const void *glyph_12_bitmap_get(
        lv_font_glyph_dsc_t *descriptor,
        lv_draw_buf_t *draw_buffer) {
    if (descriptor->gid.index >= GLYPH_12_COUNT
            || draw_buffer == NULL) {
        return NULL;
    }

    const NumericGlyph12 *glyph =
            &glyphs_12[descriptor->gid.index];

    bitmap_rows_draw(
            glyph->rows,
            glyph->width,
            NUMERIC_12_HEIGHT,
            draw_buffer);
    return draw_buffer;
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

    bitmap_rows_draw(
            glyph->rows,
            glyph->width,
            NUMERIC_14_HEIGHT,
            draw_buffer);
    return draw_buffer;
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
