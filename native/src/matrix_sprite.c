#include "matrix_sprite.h"

#include <stddef.h>

/*
 * Six native 10x12 running poses.
 *
 * The legacy SpriteMap1 sequence defines the gait phases, but these frames are
 * redrawn for the larger matrix resolution instead of scaling the 7x7 bitmap.
 * Every pose keeps a separate 3x3 head, one-pixel neck and stable torso.
 * Only arms and legs change between phases. Limbs remain one-pixel strokes;
 * crossing legs may overlap rather than becoming a thick two-pixel bar.
 */
static const uint16_t RUNNER[
        MATRIX_SPRITE_FRAME_COUNT][MATRIX_SPRITE_HEIGHT] = {
    {
        0x038, 0x038, 0x038, 0x010,
        0x070, 0x0b8, 0x134, 0x030,
        0x048, 0x084, 0x102, 0x201
    },
    {
        0x038, 0x038, 0x038, 0x010,
        0x070, 0x0b8, 0x078, 0x030,
        0x048, 0x044, 0x024, 0x022
    },
    {
        0x038, 0x038, 0x038, 0x010,
        0x078, 0x0b4, 0x030, 0x030,
        0x030, 0x028, 0x044, 0x082
    },
    {
        0x038, 0x038, 0x038, 0x010,
        0x038, 0x074, 0x0b2, 0x030,
        0x048, 0x084, 0x102, 0x201
    },
    {
        0x038, 0x038, 0x038, 0x010,
        0x038, 0x074, 0x078, 0x030,
        0x048, 0x088, 0x090, 0x110
    },
    {
        0x038, 0x038, 0x038, 0x010,
        0x078, 0x0b4, 0x030, 0x030,
        0x030, 0x050, 0x088, 0x104
    }
}

/*
 * Warmup keeps head, torso and legs fixed. Only the arm position changes:
 * down -> diagonal -> horizontal -> raised -> overhead -> horizontal.
 */
static const uint16_t WARMUP[
        MATRIX_SPRITE_FRAME_COUNT][MATRIX_SPRITE_HEIGHT] = {
    {
        0x030, 0x078, 0x078, 0x030,
        0x030, 0x078, 0x0b4, 0x0b4,
        0x030, 0x048, 0x084, 0x084
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x0b4, 0x078, 0x030, 0x030,
        0x030, 0x048, 0x084, 0x084
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x3ff, 0x078, 0x030, 0x030,
        0x030, 0x048, 0x084, 0x084
    },
    {
        0x030, 0x078, 0x078, 0x0b4,
        0x078, 0x030, 0x030, 0x030,
        0x030, 0x048, 0x084, 0x084
    },
    {
        0x0cc, 0x0cc, 0x078, 0x078,
        0x030, 0x030, 0x030, 0x030,
        0x030, 0x048, 0x084, 0x084
    },
    {
        0x030, 0x078, 0x078, 0x030,
        0x3ff, 0x078, 0x030, 0x030,
        0x030, 0x048, 0x084, 0x084
    }
};

static const uint16_t *sprite_rows(
        MatrixSpriteKind kind,
        uint8_t frame) {
    const uint8_t index =
            (uint8_t)(
                    frame
                    % MATRIX_SPRITE_FRAME_COUNT);

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
    if (framebuffer == NULL) {
        return;
    }

    const uint16_t *rows =
            sprite_rows(
                    kind,
                    frame);

    if (rows == NULL) {
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
