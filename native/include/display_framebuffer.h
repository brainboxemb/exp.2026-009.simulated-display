#ifndef DISPLAY_FRAMEBUFFER_H
#define DISPLAY_FRAMEBUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DISPLAY_WIDTH 90u
#define DISPLAY_HEIGHT 35u
#define DISPLAY_PIXEL_COUNT (DISPLAY_WIDTH * DISPLAY_HEIGHT)
#define DISPLAY_FRAMEBUFFER_BYTES ((DISPLAY_PIXEL_COUNT + 7u) / 8u)

typedef struct {
    uint8_t bits[DISPLAY_FRAMEBUFFER_BYTES];
} DisplayFramebuffer;

void display_framebuffer_clear(DisplayFramebuffer *framebuffer);
bool display_framebuffer_get(const DisplayFramebuffer *framebuffer, uint16_t x, uint16_t y);
void display_framebuffer_set(DisplayFramebuffer *framebuffer, uint16_t x, uint16_t y, bool on);

#endif
