#ifndef MATRIX_RACE_SCREEN_H
#define MATRIX_RACE_SCREEN_H

#include "matrix_framebuffer.h"
#include "matrix_theme.h"

#include <stdbool.h>

#define MATRIX_RACE_LEFT_ROW_COUNT 5u
#define MATRIX_RACE_RIGHT_ROW_COUNT 2u

typedef enum {
    MATRIX_RACE_LAYOUT_REGISTRATIONS,
    MATRIX_RACE_LAYOUT_MIXED,
    MATRIX_RACE_LAYOUT_MIXED_COMPACT_FOOTER
} MatrixRaceLayout;

typedef struct {
    const char *number;
    const char *time;
    const char *hundredths;
    const char *rank;
} MatrixRaceLeftRow;

typedef struct {
    const char *left;
    const char *right;
} MatrixRaceRightRow;

typedef struct {
    const char *location;
    const char *clock;
    const char *counter;
    MatrixRaceLayout layout;
    bool wifi_connected;
    bool registration_server_connected;
    bool synchronizing;
    MatrixRaceLeftRow left_rows[MATRIX_RACE_LEFT_ROW_COUNT];
    MatrixRaceRightRow right_rows[MATRIX_RACE_RIGHT_ROW_COUNT];
    const char *footer;
} MatrixRaceScreenData;

bool matrix_race_screen_render(
        MatrixFramebuffer *framebuffer,
        const MatrixRaceScreenData *data,
        const MatrixTheme *theme,
        uint8_t animation_frame);

#endif
