#ifndef MATRIX_STARTUP_SCREEN_H
#define MATRIX_STARTUP_SCREEN_H

#include "matrix_framebuffer.h"

#include <stdbool.h>

bool matrix_startup_screen_render(
        MatrixFramebuffer *framebuffer,
        const char *version);

#endif
