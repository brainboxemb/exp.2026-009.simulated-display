#include "matrix_startup_screen.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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

    MatrixFramebuffer boot;
    MatrixFramebuffer wifi_scan0;
    MatrixFramebuffer wifi_scan2;
    MatrixFramebuffer wifi_connected;
    MatrixFramebuffer link_connect0;
    MatrixFramebuffer link_connect2;
    MatrixFramebuffer ready;

    assert(
            matrix_startup_sequence_render(
                    &boot,
                    "v0.1.0",
                    MATRIX_STARTUP_BOOT,
                    0u));
    assert(
            matrix_startup_sequence_render(
                    &wifi_scan0,
                    "v0.1.0",
                    MATRIX_STARTUP_WIFI_SCAN,
                    0u));
    assert(
            matrix_startup_sequence_render(
                    &wifi_scan2,
                    "v0.1.0",
                    MATRIX_STARTUP_WIFI_SCAN,
                    2u));
    assert(
            matrix_startup_sequence_render(
                    &wifi_connected,
                    "v0.1.0",
                    MATRIX_STARTUP_WIFI_CONNECTED,
                    0u));
    assert(
            matrix_startup_sequence_render(
                    &link_connect0,
                    "v0.1.0",
                    MATRIX_STARTUP_LINK_CONNECTING,
                    0u));
    assert(
            matrix_startup_sequence_render(
                    &link_connect2,
                    "v0.1.0",
                    MATRIX_STARTUP_LINK_CONNECTING,
                    2u));
    assert(
            matrix_startup_sequence_render(
                    &ready,
                    "v0.1.0",
                    MATRIX_STARTUP_READY,
                    0u));

    const MatrixPixel warning_background =
            matrix_pixel_rgb565_create(
                    0,
                    96,
                    255);

    /* Two startup warnings occupy the normal top-right status positions. */
    assert(
            matrix_framebuffer_get(
                    &boot,
                    137,
                    10)
            == warning_background);
    assert(
            matrix_framebuffer_get(
                    &boot,
                    149,
                    10)
            == warning_background);

    /* Scanning uses an active lime block; the link remains a warning. */
    assert(
            matrix_framebuffer_get(
                    &wifi_scan0,
                    137,
                    10)
            == warning_background);
    assert(
            matrix_framebuffer_get(
                    &wifi_scan0,
                    149,
                    10)
            == warning_background);
    assert(
            memcmp(
                    wifi_scan0.pixels,
                    wifi_scan2.pixels,
                    sizeof(wifi_scan0.pixels))
            != 0);

    /* Once WiFi is connected, only link remains and owns the rightmost slot. */
    assert(
            matrix_framebuffer_get(
                    &wifi_connected,
                    149,
                    10)
            == warning_background);

    assert(
            matrix_framebuffer_get(
                    &link_connect0,
                    149,
                    10)
            == warning_background);
    assert(
            memcmp(
                    link_connect0.pixels,
                    link_connect2.pixels,
                    sizeof(link_connect0.pixels))
            != 0);

    /* READY is exactly the untouched static splash again. */
    assert(
            memcmp(
                    framebuffer.pixels,
                    ready.pixels,
                    sizeof(framebuffer.pixels))
            == 0);

    puts("matrix_startup_screen tests passed");
    return 0;
}
