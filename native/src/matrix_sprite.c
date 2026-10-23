#include "matrix_sprite.h"

#include <stddef.h>

typedef struct {
    uint8_t x;
    uint8_t y;
} MatrixSpritePoint;

typedef struct {
    MatrixSpritePoint forward_elbow;
    MatrixSpritePoint forward_hand;
    MatrixSpritePoint backward_elbow;
    MatrixSpritePoint backward_hand;
    MatrixSpritePoint forward_knee;
    MatrixSpritePoint forward_foot;
    MatrixSpritePoint backward_knee;
    MatrixSpritePoint backward_foot;
} MatrixRunnerPose;

/*
 * Native 10x12 running cycle designed from the running motion itself.
 *
 * The body is fixed and faces left:
 * - 3x3 head centered slightly ahead of the torso;
 * - one-pixel vertical neck;
 * - torso leaning one pixel forward toward the left;
 * - one shoulder joint and one hip joint.
 *
 * Only the limbs move. Each arm and leg is a one-pixel polyline with one
 * intermediate joint. The arm swing stays compact around the torso; the legs
 * carry most of the visible motion through contact, recoil and passing phases.
 * The second half repeats those shapes with the opposite foot contacting the
 * ground.
 */
static const MatrixRunnerPose RUNNER_POSES[
        MATRIX_SPRITE_FRAME_COUNT] = {
    {
        .forward_elbow = {3, 5},
        .forward_hand = {3, 6},
        .backward_elbow = {5, 5},
        .backward_hand = {6, 6},
        .forward_knee = {3, 8},
        .forward_foot = {1, 10},
        .backward_knee = {6, 8},
        .backward_foot = {8, 9}
    },
    {
        .forward_elbow = {3, 5},
        .forward_hand = {4, 6},
        .backward_elbow = {5, 5},
        .backward_hand = {6, 6},
        .forward_knee = {3, 8},
        .forward_foot = {2, 10},
        .backward_knee = {6, 8},
        .backward_foot = {7, 10}
    },
    {
        .forward_elbow = {4, 5},
        .forward_hand = {4, 6},
        .backward_elbow = {5, 5},
        .backward_hand = {6, 6},
        .forward_knee = {4, 8},
        .forward_foot = {3, 10},
        .backward_knee = {5, 8},
        .backward_foot = {6, 10}
    },
    {
        .forward_elbow = {3, 5},
        .forward_hand = {3, 6},
        .backward_elbow = {5, 5},
        .backward_hand = {6, 6},
        .forward_knee = {3, 8},
        .forward_foot = {1, 9},
        .backward_knee = {6, 8},
        .backward_foot = {8, 10}
    },
    {
        .forward_elbow = {3, 5},
        .forward_hand = {4, 6},
        .backward_elbow = {5, 5},
        .backward_hand = {6, 6},
        .forward_knee = {3, 8},
        .forward_foot = {2, 10},
        .backward_knee = {6, 8},
        .backward_foot = {7, 10}
    },
    {
        .forward_elbow = {4, 5},
        .forward_hand = {4, 6},
        .backward_elbow = {5, 5},
        .backward_hand = {6, 6},
        .forward_knee = {4, 8},
        .forward_foot = {3, 10},
        .backward_knee = {5, 8},
        .backward_foot = {6, 10}
    }
}

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


static void draw_pixel(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixSpritePoint point,
        MatrixPixel color) {
    matrix_framebuffer_set(
            framebuffer,
            (uint16_t)(x + point.x),
            (uint16_t)(y + point.y),
            color);
}

static void draw_line(
        MatrixFramebuffer *framebuffer,
        uint16_t origin_x,
        uint16_t origin_y,
        MatrixSpritePoint from,
        MatrixSpritePoint to,
        MatrixPixel color) {
    int x0 = (int)from.x;
    int y0 = (int)from.y;
    const int x1 = (int)to.x;
    const int y1 = (int)to.y;
    const int dx =
            x1 >= x0
                    ? x1 - x0
                    : x0 - x1;
    const int sx =
            x0 < x1
                    ? 1
                    : -1;
    const int dy =
            y1 >= y0
                    ? y0 - y1
                    : y1 - y0;
    const int sy =
            y0 < y1
                    ? 1
                    : -1;
    int error = dx + dy;

    for (;;) {
        MatrixSpritePoint point = {
            (uint8_t)x0,
            (uint8_t)y0
        };

        draw_pixel(
                framebuffer,
                origin_x,
                origin_y,
                point,
                color);

        if (x0 == x1
                && y0 == y1) {
            break;
        }

        const int doubled_error =
                2 * error;

        if (doubled_error >= dy) {
            error += dy;
            x0 += sx;
        }

        if (doubled_error <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

static void draw_runner(
        MatrixFramebuffer *framebuffer,
        uint8_t frame,
        uint16_t x,
        uint16_t y,
        MatrixPixel color) {
    const MatrixRunnerPose *pose =
            &RUNNER_POSES[
                    frame
                    % MATRIX_SPRITE_FRAME_COUNT];

    for (uint8_t head_y = 0;
            head_y < 3u;
            head_y++) {
        for (uint8_t head_x = 3;
                head_x < 6u;
                head_x++) {
            draw_pixel(
                    framebuffer,
                    x,
                    y,
                    (MatrixSpritePoint){
                        head_x,
                        head_y
                    },
                    color);
        }
    }

    draw_pixel(
            framebuffer,
            x,
            y,
            (MatrixSpritePoint){4, 3},
            color);

    draw_line(
            framebuffer,
            x,
            y,
            (MatrixSpritePoint){4, 4},
            (MatrixSpritePoint){5, 6},
            color);

    const MatrixSpritePoint shoulder = {4, 4};
    const MatrixSpritePoint hip = {5, 6};

    draw_line(
            framebuffer,
            x,
            y,
            shoulder,
            pose->forward_elbow,
            color);
    draw_line(
            framebuffer,
            x,
            y,
            pose->forward_elbow,
            pose->forward_hand,
            color);
    draw_line(
            framebuffer,
            x,
            y,
            shoulder,
            pose->backward_elbow,
            color);
    draw_line(
            framebuffer,
            x,
            y,
            pose->backward_elbow,
            pose->backward_hand,
            color);

    draw_line(
            framebuffer,
            x,
            y,
            hip,
            pose->forward_knee,
            color);
    draw_line(
            framebuffer,
            x,
            y,
            pose->forward_knee,
            pose->forward_foot,
            color);
    draw_line(
            framebuffer,
            x,
            y,
            hip,
            pose->backward_knee,
            color);
    draw_line(
            framebuffer,
            x,
            y,
            pose->backward_knee,
            pose->backward_foot,
            color);
}

static void draw_warmup(
        MatrixFramebuffer *framebuffer,
        uint8_t frame,
        uint16_t x,
        uint16_t y,
        MatrixPixel color) {
    const uint16_t *rows =
            WARMUP[
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
