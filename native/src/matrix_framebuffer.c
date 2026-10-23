#include "matrix_framebuffer.h"

#include <string.h>

static bool valid_coordinate(
        uint16_t x,
        uint16_t y) {
    return x < MATRIX_DISPLAY_WIDTH
            && y < MATRIX_DISPLAY_HEIGHT;
}

static size_t pixel_index(
        uint16_t x,
        uint16_t y) {
    return (size_t)y * MATRIX_DISPLAY_WIDTH + x;
}

MatrixPixel matrix_pixel_rgb565_create(
        uint8_t red,
        uint8_t green,
        uint8_t blue) {
    const uint16_t red5 =
            (uint16_t)(red >> 3u);
    const uint16_t green6 =
            (uint16_t)(green >> 2u);
    const uint16_t blue5 =
            (uint16_t)(blue >> 3u);

    return (MatrixPixel)(
            (red5 << 11u)
            | (green6 << 5u)
            | blue5);
}

void matrix_framebuffer_clear(
        MatrixFramebuffer *framebuffer) {
    memset(
            framebuffer->pixels,
            0,
            sizeof(framebuffer->pixels));
}

MatrixPixel matrix_framebuffer_get(
        const MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y) {
    if (!valid_coordinate(x, y)) {
        return 0;
    }

    return framebuffer->pixels[pixel_index(x, y)];
}

void matrix_framebuffer_set(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixPixel pixel) {
    if (!valid_coordinate(x, y)) {
        return;
    }

    framebuffer->pixels[pixel_index(x, y)] = pixel;
}
