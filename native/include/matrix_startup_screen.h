#ifndef MATRIX_STARTUP_SCREEN_H
#define MATRIX_STARTUP_SCREEN_H

#include "matrix_framebuffer.h"

#include <stdbool.h>
#include <stdint.h>

#define MATRIX_STARTUP_SEQUENCE_FRAME_COUNT 4u
#define MATRIX_STARTUP_MENU_OPTION_COUNT 11u

typedef enum {
    MATRIX_STARTUP_BOOT,
    MATRIX_STARTUP_WIFI_SCAN,
    MATRIX_STARTUP_WIFI_CONNECTED,
    MATRIX_STARTUP_LINK_CONNECTING,
    MATRIX_STARTUP_READY
} MatrixStartupState;

typedef struct {
    const char *version;
    char display_id;
    char ssid_id;
} MatrixStartupInfo;

bool matrix_startup_screen_render(
        MatrixFramebuffer *framebuffer,
        const MatrixStartupInfo *info);

bool matrix_startup_sequence_render(
        MatrixFramebuffer *framebuffer,
        const MatrixStartupInfo *info,
        MatrixStartupState state,
        uint8_t animation_frame);

bool matrix_startup_menu_render(
        MatrixFramebuffer *framebuffer,
        const MatrixStartupInfo *info,
        uint8_t selected_index);

#endif
