#include "matrix_framebuffer.h"

#include <assert.h>
#include <stdio.h>

static void test_geometry(void) {
    assert(MATRIX_DISPLAY_WIDTH == 160u);
    assert(MATRIX_DISPLAY_HEIGHT == 64u);
    assert(MATRIX_DISPLAY_PIXEL_COUNT == 10240u);
    assert(sizeof(MatrixFramebuffer) == 20480u);
}

static void test_rgb565_conversion(void) {
    assert(matrix_pixel_rgb565_create(255, 0, 0) == 0xf800u);
    assert(matrix_pixel_rgb565_create(0, 255, 0) == 0x07e0u);
    assert(matrix_pixel_rgb565_create(0, 0, 255) == 0x001fu);
    assert(matrix_pixel_rgb565_create(255, 255, 255) == 0xffffu);
    assert(matrix_pixel_rgb565_create(0, 0, 0) == 0x0000u);
}

static void test_set_get_and_bounds(void) {
    MatrixFramebuffer framebuffer;
    matrix_framebuffer_clear(&framebuffer);

    const MatrixPixel pixel =
            matrix_pixel_rgb565_create(
                    255,
                    128,
                    32);

    matrix_framebuffer_set(
            &framebuffer,
            159,
            63,
            pixel);
    matrix_framebuffer_set(
            &framebuffer,
            160,
            64,
            0xffffu);

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    159,
                    63)
            == pixel);

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    160,
                    64)
            == 0);
}

static void test_clear(void) {
    MatrixFramebuffer framebuffer;

    matrix_framebuffer_set(
            &framebuffer,
            10,
            20,
            0xffffu);

    matrix_framebuffer_clear(&framebuffer);

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    10,
                    20)
            == 0);
}

int main(void) {
    test_geometry();
    test_rgb565_conversion();
    test_set_get_and_bounds();
    test_clear();

    puts("matrix_framebuffer tests passed");
    return 0;
}
