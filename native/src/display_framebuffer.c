#include "display_framebuffer.h"

#include <string.h>

static bool valid_coordinate(uint16_t x, uint16_t y) {
    return x < DISPLAY_WIDTH && y < DISPLAY_HEIGHT;
}

static size_t pixel_index(uint16_t x, uint16_t y) {
    return (size_t)y * DISPLAY_WIDTH + x;
}

void display_framebuffer_clear(DisplayFramebuffer *framebuffer) {
    memset(framebuffer->bits, 0, sizeof(framebuffer->bits));
}

bool display_framebuffer_get(const DisplayFramebuffer *framebuffer, uint16_t x, uint16_t y) {
    if (!valid_coordinate(x, y)) {
        return false;
    }

    const size_t index = pixel_index(x, y);
    return (framebuffer->bits[index >> 3u] & (uint8_t)(1u << (index & 7u))) != 0;
}

void display_framebuffer_set(DisplayFramebuffer *framebuffer, uint16_t x, uint16_t y, bool on) {
    if (!valid_coordinate(x, y)) {
        return;
    }

    const size_t index = pixel_index(x, y);
    const uint8_t mask = (uint8_t)(1u << (index & 7u));
    if (on) {
        framebuffer->bits[index >> 3u] |= mask;
    } else {
        framebuffer->bits[index >> 3u] &= (uint8_t)~mask;
    }
}
