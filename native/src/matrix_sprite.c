#include "matrix_sprite.h"

#include <stddef.h>

static const uint16_t RUNNER[
        MATRIX_SPRITE_FRAME_COUNT][MATRIX_SPRITE_HEIGHT] = {
    {
        0x030, 0x078, 0x078, 0x030,
        0x0fc, 0x1b6, 0x030, 0x078,
        0x0cc, 0x186, 0x300, 0x003
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x07c, 0x0ec, 0x1b0, 0x038,
        0x06c, 0x0c6, 0x181, 0x300
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x0f8, 0x1b8, 0x330, 0x078,
        0x0cc, 0x186, 0x006, 0x00c
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x0fc, 0x1b6, 0x030, 0x078,
        0x0cc, 0x003, 0x300, 0x180
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x0f8, 0x0dc, 0x036, 0x070,
        0x0d8, 0x18c, 0x206, 0x003
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x07c, 0x076, 0x033, 0x078,
        0x0cc, 0x186, 0x180, 0x0c0
    }
};

static const uint16_t WARMUP[
        MATRIX_SPRITE_FRAME_COUNT][MATRIX_SPRITE_HEIGHT] = {
    {
        0x030, 0x078, 0x078, 0x030,
        0x078, 0x0b4, 0x0b4, 0x030,
        0x078, 0x048, 0x0cc, 0x0cc
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x3ff, 0x333, 0x030, 0x030,
        0x078, 0x048, 0x0cc, 0x0cc
    },
    {
        0x186, 0x0cc, 0x078, 0x078,
        0x030, 0x078, 0x030, 0x030,
        0x078, 0x048, 0x0cc, 0x0cc
    },
    {
        0x0c0, 0x1e0, 0x1e0, 0x0c0,
        0x1f0, 0x360, 0x060, 0x060,
        0x0f0, 0x090, 0x198, 0x198
    },
    {
        0x186, 0x0cc, 0x078, 0x078,
        0x030, 0x078, 0x030, 0x030,
        0x078, 0x048, 0x0cc, 0x0cc
    },
    {
        0x00c, 0x01e, 0x01e, 0x00c,
        0x03e, 0x01b, 0x018, 0x018,
        0x03c, 0x024, 0x066, 0x066
    }
};

static const uint16_t *sprite_rows(
        MatrixSpriteKind kind,
        uint8_t frame) {
    const uint8_t index =
            (uint8_t)(frame % MATRIX_SPRITE_FRAME_COUNT);

    switch (kind) {
        case MATRIX_SPRITE_RUNNER:
            return RUNNER[index];

        case MATRIX_SPRITE_WARMUP:
            return WARMUP[index];

        default:
            return NULL;
    }
}

void matrix_sprite_draw(
        MatrixFramebuffer *framebuffer,
        MatrixSpriteKind kind,
        uint8_t frame,
        uint16_t x,
        uint16_t y,
        MatrixPixel color) {
    const uint16_t *rows =
            sprite_rows(
                    kind,
                    frame);

    if (framebuffer == NULL
            || rows == NULL) {
        return;
    }

    for (uint16_t row = 0;
            row < MATRIX_SPRITE_HEIGHT;
            row++) {
        for (uint16_t column = 0;
                column < MATRIX_SPRITE_WIDTH;
                column++) {
            const uint16_t mask =
                    (uint16_t)(
                            1u
                            << (MATRIX_SPRITE_WIDTH
                                    - 1u
                                    - column));

            if ((rows[row] & mask) == 0) {
                continue;
            }

            matrix_framebuffer_set(
                    framebuffer,
                    (uint16_t)(x + column),
                    (uint16_t)(y + row),
                    color);
        }
    }
}
