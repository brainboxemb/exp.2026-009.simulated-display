#ifndef EXP_009_HUB75_OUTPUT_H
#define EXP_009_HUB75_OUTPUT_H

#include "matrix_framebuffer.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool hub75_output_init(void);

bool hub75_output_present(
        const MatrixFramebuffer *framebuffer);

void hub75_output_set_brightness(
        uint8_t brightness);

void hub75_output_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
