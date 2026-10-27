#ifndef MATRIX_STATUS_ICON_H
#define MATRIX_STATUS_ICON_H

#include "matrix_framebuffer.h"

#include <stdbool.h>
#include <stdint.h>

#define MATRIX_STATUS_ICON_WIDTH 11u
#define MATRIX_STATUS_ICON_HEIGHT 11u
#define MATRIX_STATUS_ICON_GAP 1u

typedef enum {
    MATRIX_STATUS_ICON_WIFI,
    MATRIX_STATUS_ICON_LINK,
    MATRIX_STATUS_ICON_SYNC
} MatrixStatusIcon;

void matrix_status_icon_draw(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixStatusIcon icon,
        MatrixPixel background_color,
        MatrixPixel icon_color,
        bool strike,
        MatrixPixel strike_color);

void matrix_status_icon_draw_progress(
        MatrixFramebuffer *framebuffer,
        uint16_t x,
        uint16_t y,
        MatrixStatusIcon icon,
        uint8_t animation_frame,
        MatrixPixel background_color,
        MatrixPixel icon_color);

#endif
