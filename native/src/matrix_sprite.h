#ifndef MATRIX_SPRITE_H
#define MATRIX_SPRITE_H

#include "matrix_framebuffer.h"

#include <stdint.h>

#define MATRIX_SPRITE_WIDTH 10u
#define MATRIX_SPRITE_HEIGHT 12u
#define MATRIX_SPRITE_FRAME_COUNT 6u

typedef enum {
    MATRIX_SPRITE_RUNNER,
    MATRIX_SPRITE_WARMUP
} MatrixSpriteKind;

void matrix_sprite_draw(
        MatrixFramebuffer *framebuffer,
        MatrixSpriteKind kind,
        uint8_t frame,
        uint16_t x,
        uint16_t y,
        MatrixPixel color);

#endif
