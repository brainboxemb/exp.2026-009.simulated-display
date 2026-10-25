#ifndef MATRIX_FRAMEBUFFER_H
#define MATRIX_FRAMEBUFFER_H

#include <stddef.h>
#include <stdint.h>

#define MATRIX_DISPLAY_WIDTH 160u
#define MATRIX_DISPLAY_HEIGHT 64u
#define MATRIX_DISPLAY_PIXEL_COUNT \
    (MATRIX_DISPLAY_WIDTH * MATRIX_DISPLAY_HEIGHT)

typedef uint16_t MatrixPixel;

typedef struct {
    MatrixPixel pixels[MATRIX_DISPLAY_PIXEL_COUNT];
} MatrixFramebuffer;

MatrixPixel matrix_pixel_rgb565_create(
        uint8_t red,
        uint8_t green,
        uint8_t blue);

void matrix_framebuffer_clear(
        MatrixFramebuffer *framebuffer);

MatrixPixel matrix_framebuffer_get(
        const MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y);

void matrix_framebuffer_set(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixPixel pixel);

#endif
