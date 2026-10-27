#include "matrix_startup_screen.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    MatrixFramebuffer framebuffer;

    assert(
            matrix_startup_screen_render(
                    &framebuffer,
                    "v0.1.0"));

    const MatrixPixel black =
            matrix_pixel_rgb565_create(
                    0,
                    0,
                    0);
    const MatrixPixel lime =
            matrix_pixel_rgb565_create(
                    205,
                    255,
                    0);
    const MatrixPixel yellow =
            matrix_pixel_rgb565_create(
                    255,
                    216,
                    0);
    const MatrixPixel cyan =
            matrix_pixel_rgb565_create(
                    0,
                    220,
                    255);
    const MatrixPixel blue =
            matrix_pixel_rgb565_create(
                    0,
                    80,
                    255);
    const MatrixPixel magenta =
            matrix_pixel_rgb565_create(
                    255,
                    0,
                    120);
    const MatrixPixel red =
            matrix_pixel_rgb565_create(
                    255,
                    32,
                    48);
    const MatrixPixel version_color =
            matrix_pixel_rgb565_create(
                    128,
                    128,
                    128);

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    0)
            == black);

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    24,
                    2)
            == lime);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    40,
                    34)
            == yellow);

    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    12)
            == cyan);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    18)
            == blue);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    47)
            == magenta);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    0,
                    55)
            == red);

    /* v0.1.0 is right aligned with one pixel of bottom/right margin. */
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    140,
                    58)
            == version_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    158,
                    62)
            == version_color);
    assert(
            matrix_framebuffer_get(
                    &framebuffer,
                    159,
                    63)
            == black);

    puts("matrix_startup_screen tests passed");
    return 0;
}
