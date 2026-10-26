#ifndef MATRIX_SERIAL_H
#define MATRIX_SERIAL_H

#include "matrix_framebuffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MATRIX_SERIAL_PAGE_COUNT 3u
#define MATRIX_SERIAL_PALETTE_SIZE 16u
#define MATRIX_SERIAL_TRANSPARENT 0xffu
#define MATRIX_SERIAL_ICON_COUNT 4u
#define MATRIX_SERIAL_NOTIFICATION_COUNT 2u
#define MATRIX_SERIAL_SPRITE_CAPACITY 16u

typedef struct {
    uint8_t kind;
    uint16_t x;
    uint16_t y;
    uint8_t color;
} MatrixSerialSprite;

typedef struct {
    uint8_t pixels[MATRIX_DISPLAY_PIXEL_COUNT];
    MatrixSerialSprite sprites[MATRIX_SERIAL_SPRITE_CAPACITY];
    uint8_t sprite_count;
} MatrixSerialPage;

typedef struct {
    MatrixSerialPage pages[MATRIX_SERIAL_PAGE_COUNT];
    MatrixPixel palette[MATRIX_SERIAL_PALETTE_SIZE];
    bool notifications[MATRIX_SERIAL_NOTIFICATION_COUNT];
    uint8_t icon_colors[MATRIX_SERIAL_ICON_COUNT][3];
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint8_t selected_page;
    uint8_t foreground;
    uint8_t background;
    uint8_t font_size;
    uint8_t brightness;
    uint8_t sprite_velocity;
    uint8_t sprite_elapsed_ticks;
    uint8_t sprite_frame;
    char command[16];
    uint8_t command_length;
    bool in_command;
    bool update_active;
} MatrixSerialDisplay;

void matrix_serial_init(MatrixSerialDisplay *display);

bool matrix_serial_feed(
        MatrixSerialDisplay *display,
        const uint8_t *bytes,
        size_t length);

void matrix_serial_compose(
        const MatrixSerialDisplay *display,
        MatrixFramebuffer *framebuffer);

bool matrix_serial_tick(MatrixSerialDisplay *display);

bool matrix_serial_has_sprites(
        const MatrixSerialDisplay *display);

bool matrix_serial_update_active(
        const MatrixSerialDisplay *display);

uint8_t matrix_serial_brightness(
        const MatrixSerialDisplay *display);

#endif
