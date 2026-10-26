#include "matrix_lvgl.h"

#include "lvgl.h"

#include <stdbool.h>

static bool initialized;

void matrix_lvgl_ensure_initialized(void) {
    if (initialized) {
        return;
    }

    lv_init();
    initialized = true;
}
