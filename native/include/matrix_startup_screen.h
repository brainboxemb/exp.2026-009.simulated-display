#ifndef MATRIX_STARTUP_SCREEN_H
#define MATRIX_STARTUP_SCREEN_H

#include "matrix_framebuffer.h"

#include <stdbool.h>
#include <stdint.h>

#define MATRIX_STARTUP_SEQUENCE_FRAME_COUNT 4u

typedef enum {
    MATRIX_STARTUP_BOOT,
    MATRIX_STARTUP_WIFI_SCAN,
    MATRIX_STARTUP_WIFI_CONNECTED,
    MATRIX_STARTUP_LINK_CONNECTING,
    MATRIX_STARTUP_READY
} MatrixStartupState;

bool matrix_startup_screen_render(
        MatrixFramebuffer *framebuffer,
        const char *version);

bool matrix_startup_sequence_render(
        MatrixFramebuffer *framebuffer,
        const char *version,
        MatrixStartupState state,
        uint8_t animation_frame);

#endif
