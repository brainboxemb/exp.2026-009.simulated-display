#include "matrix_serial.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void feed(
        MatrixSerialDisplay *display,
        const char *text) {
    assert(
            matrix_serial_feed(
                    display,
                    (const uint8_t *)text,
                    strlen(text)));
}

static void test_pages_and_lines(void) {
    MatrixSerialDisplay display;
    MatrixFramebuffer frame;

    matrix_serial_init(&display);

    feed(
            &display,
            "/PS0/CF06/L00000/DL00010");
    feed(
            &display,
            "/PS1/CF00/L00005/DL00005");

    matrix_serial_compose(
            &display,
            &frame);

    assert(
            matrix_framebuffer_get(
                    &frame,
                    4,
                    0)
            == display.palette[6]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    5,
                    0)
            == display.palette[0]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    6,
                    0)
            == display.palette[6]);

    feed(&display, "/PC");

    matrix_serial_compose(
            &display,
            &frame);

    assert(
            matrix_framebuffer_get(
                    &frame,
                    5,
                    0)
            == display.palette[6]);
    assert(display.cursor_x == 0u);
    assert(display.cursor_y == 0u);
}

static void test_rectangles_and_transparent_text(void) {
    MatrixSerialDisplay display;
    MatrixFramebuffer frame;

    matrix_serial_init(&display);

    feed(
            &display,
            "/PS0/CF04/L11000/DL11159");
    feed(
            &display,
            "/PS1/CF06/L00000/DF10015");
    feed(
            &display,
            "/CF00/CB16/FS09/L00001"
            "99");

    matrix_serial_compose(
            &display,
            &frame);

    assert(
            matrix_framebuffer_get(
                    &frame,
                    0,
                    0)
            == display.palette[6]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    15,
                    10)
            == display.palette[6]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    20,
                    11)
            == display.palette[4]);

    feed(
            &display,
            "/CF05/L20020/DR24030");

    matrix_serial_compose(
            &display,
            &frame);

    assert(
            matrix_framebuffer_get(
                    &frame,
                    20,
                    20)
            == display.palette[5]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    30,
                    24)
            == display.palette[5]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    25,
                    22)
            == 0u);
}

static void test_full_width_coordinate_and_chunking(void) {
    MatrixSerialDisplay display;

    matrix_serial_init(&display);

    assert(
            matrix_serial_feed(
                    &display,
                    (const uint8_t *)"/L63",
                    4u));
    assert(
            matrix_serial_feed(
                    &display,
                    (const uint8_t *)"159",
                    3u));

    assert(display.cursor_x == 159u);
    assert(display.cursor_y == 63u);
}

static void test_font_and_text(void) {
    MatrixSerialDisplay display;
    MatrixFramebuffer frame;

    matrix_serial_init(&display);

    feed(
            &display,
            "/FS09/CF01/CB03/L00000A");

    assert(display.font_size == 9u);
    assert(display.cursor_x > 0u);

    matrix_serial_compose(
            &display,
            &frame);

    bool foreground_seen = false;
    bool background_seen = false;

    for (uint16_t y = 0u; y < 9u; y++) {
        for (uint16_t x = 0u; x < 12u; x++) {
            const MatrixPixel pixel =
                    matrix_framebuffer_get(
                            &frame,
                            x,
                            y);

            if (pixel == display.palette[1]) {
                foreground_seen = true;
            }

            if (pixel == display.palette[3]) {
                background_seen = true;
            }
        }
    }

    assert(foreground_seen);
    assert(background_seen);
}

static void test_icon_color_configuration(void) {
    MatrixSerialDisplay display;
    MatrixFramebuffer frame;

    matrix_serial_init(&display);

    feed(
            &display,
            "/PS1/CI3C10C01/L08010/BI3");

    matrix_serial_compose(
            &display,
            &frame);

    assert(
            matrix_framebuffer_get(
                    &frame,
                    10,
                    8)
            == display.palette[10]);

    bool glyph_seen = false;

    for (uint16_t y = 8u; y < 19u; y++) {
        for (uint16_t x = 10u; x < 21u; x++) {
            if (matrix_framebuffer_get(
                    &frame,
                    x,
                    y)
                    == display.palette[1]) {
                glyph_seen = true;
            }
        }
    }

    assert(glyph_seen);

    feed(&display, "/PC");
    matrix_serial_compose(
            &display,
            &frame);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    10,
                    8)
            == 0u);
}

static void test_notifications_overlay_and_restore(void) {
    MatrixSerialDisplay display;
    MatrixFramebuffer frame;

    matrix_serial_init(&display);

    feed(
            &display,
            "/PS2/CF13/L00137/DF10159");
    feed(
            &display,
            "/N11/N21");

    matrix_serial_compose(
            &display,
            &frame);

    assert(display.notifications[0]);
    assert(display.notifications[1]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    137,
                    0)
            == display.palette[10]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    149,
                    0)
            == display.palette[10]);

    feed(&display, "/N10");
    matrix_serial_compose(
            &display,
            &frame);

    assert(!display.notifications[0]);
    assert(display.notifications[1]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    137,
                    0)
            == display.palette[13]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    149,
                    0)
            == display.palette[10]);

    feed(&display, "/N20");
    matrix_serial_compose(
            &display,
            &frame);

    assert(!display.notifications[1]);
    assert(
            matrix_framebuffer_get(
                    &frame,
                    149,
                    0)
            == display.palette[13]);
}

static void test_sprites_and_velocity(void) {
    MatrixSerialDisplay display;

    matrix_serial_init(&display);

    feed(
            &display,
            "/CF13/L10010/S0/SV1");

    assert(matrix_serial_has_sprites(&display));
    assert(display.pages[0].sprite_count == 1u);
    assert(display.pages[0].sprites[0].color == 13u);
    assert(display.sprite_frame == 0u);

    assert(matrix_serial_tick(&display));
    assert(display.sprite_frame == 1u);

    feed(&display, "/SV4");

    assert(!matrix_serial_tick(&display));
    assert(!matrix_serial_tick(&display));
    assert(!matrix_serial_tick(&display));
    assert(matrix_serial_tick(&display));
    assert(display.sprite_frame == 2u);
}

static void test_update_transaction_state(void) {
    MatrixSerialDisplay display;

    matrix_serial_init(&display);

    assert(!matrix_serial_update_active(&display));

    feed(&display, "/UB");
    assert(matrix_serial_update_active(&display));

    assert(
            !matrix_serial_feed(
                    &display,
                    (const uint8_t *)"/UB",
                    3u));
    assert(matrix_serial_update_active(&display));

    feed(&display, "/PS0/PC/CF06/L00000/DL00010");
    assert(matrix_serial_update_active(&display));

    feed(&display, "/UE");
    assert(!matrix_serial_update_active(&display));

    assert(
            !matrix_serial_feed(
                    &display,
                    (const uint8_t *)"/UE",
                    3u));
    assert(!matrix_serial_update_active(&display));
}

static void test_brightness_state(void) {
    MatrixSerialDisplay display;

    matrix_serial_init(&display);

    feed(&display, "/XB42");
    assert(matrix_serial_brightness(&display) == 42u);
}

int main(void) {
    test_pages_and_lines();
    test_rectangles_and_transparent_text();
    test_full_width_coordinate_and_chunking();
    test_font_and_text();
    test_icon_color_configuration();
    test_notifications_overlay_and_restore();
    test_sprites_and_velocity();
    test_update_transaction_state();
    test_brightness_state();

    puts("matrix_serial tests passed");
    return 0;
}
