#include "matrix_sprite.h"

#include <stddef.h>

#define RUNNER_SOURCE_WIDTH 7u
#define RUNNER_SOURCE_HEIGHT 7u

/*
 * Original six-frame SpriteMap1 pattern from the legacy display, expressed as
 * seven visible columns. The matrix renderer scales this source pattern to
 * 10x12 with nearest-neighbour pixels so the gait stays recognizable.
 */
static const uint8_t RUNNER[
        MATRIX_SPRITE_FRAME_COUNT][RUNNER_SOURCE_WIDTH] = {
    {0x00, 0x00, 0x33, 0x2f, 0x70, 0x20, 0x00},
    {0x20, 0x10, 0x13, 0x0f, 0x30, 0x40, 0x00},
    {0x20, 0x10, 0x13, 0x0f, 0x10, 0x20, 0x20},
    {0x00, 0x60, 0x13, 0x0f, 0x10, 0x20, 0x20},
    {0x00, 0x00, 0x63, 0x1f, 0x30, 0x20, 0x00},
    {0x00, 0x00, 0x03, 0x7f, 0x20, 0x20, 0x00}
};

/*
 * SpriteMap1 has no room for readable arms at 7x7. At 10x12 we retain the
 * original gait and add one-pixel arm strokes around the scaled torso.
 * The arm swing reverses through the six legacy gait phases.
 */
static const uint16_t RUNNER_ARMS[
        MATRIX_SPRITE_FRAME_COUNT][MATRIX_SPRITE_HEIGHT] = {
    {
        0x000, 0x000, 0x000, 0x000,
        0x024, 0x04c, 0x080, 0x000,
        0x000, 0x000, 0x000, 0x000
    },
    {
        0x000, 0x000, 0x000, 0x000,
        0x028, 0x044, 0x044, 0x000,
        0x000, 0x000, 0x000, 0x000
    },
    {
        0x000, 0x000, 0x000, 0x000,
        0x088, 0x0c4, 0x002, 0x000,
        0x000, 0x000, 0x000, 0x000
    },
    {
        0x000, 0x000, 0x000, 0x000,
        0x088, 0x0c4, 0x002, 0x000,
        0x000, 0x000, 0x000, 0x000
    },
    {
        0x000, 0x000, 0x000, 0x000,
        0x028, 0x044, 0x044, 0x000,
        0x000, 0x000, 0x000, 0x000
    },
    {
        0x000, 0x000, 0x000, 0x000,
        0x024, 0x04c, 0x080, 0x000,
        0x000, 0x000, 0x000, 0x000
    }
};

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

static void draw_runner(
        MatrixFramebuffer *framebuffer,
        uint8_t frame,
        uint16_t x,
        uint16_t y,
        MatrixPixel color) {
    const uint8_t *columns =
            RUNNER[frame % MATRIX_SPRITE_FRAME_COUNT];

    for (uint16_t target_y = 0;
            target_y < MATRIX_SPRITE_HEIGHT;
            target_y++) {
        const uint16_t source_y =
                (uint16_t)(
                        target_y
                        * RUNNER_SOURCE_HEIGHT
                        / MATRIX_SPRITE_HEIGHT);

        for (uint16_t target_x = 0;
                target_x < MATRIX_SPRITE_WIDTH;
                target_x++) {
            const uint16_t source_x =
                    (uint16_t)(
                            target_x
                            * RUNNER_SOURCE_WIDTH
                            / MATRIX_SPRITE_WIDTH);

            if ((columns[source_x]
                    & (uint8_t)(1u << source_y))
                    == 0) {
                continue;
            }

            matrix_framebuffer_set(
                    framebuffer,
                    (uint16_t)(x + target_x),
                    (uint16_t)(y + target_y),
                    color);
        }
    }

    const uint16_t *arms =
            RUNNER_ARMS[
                    frame
                    % MATRIX_SPRITE_FRAME_COUNT];

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

            if ((arms[row] & mask) == 0) {
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

static void draw_warmup(
        MatrixFramebuffer *framebuffer,
        uint8_t frame,
        uint16_t x,
        uint16_t y,
        MatrixPixel color) {
    const uint16_t *rows =
            WARMUP[frame % MATRIX_SPRITE_FRAME_COUNT];

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

    switch (kind) {
        case MATRIX_SPRITE_RUNNER:
            draw_runner(
                    framebuffer,
                    frame,
                    x,
                    y,
                    color);
            break;

        case MATRIX_SPRITE_WARMUP:
            draw_warmup(
                    framebuffer,
                    frame,
                    x,
                    y,
                    color);
            break;

        default:
            break;
    }
}
