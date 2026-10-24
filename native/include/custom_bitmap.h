#ifndef CUSTOM_BITMAP_H
#define CUSTOM_BITMAP_H

#include <stdbool.h>
#include <stdint.h>

#define CUSTOM_BITMAP_DIGIT_COUNT 10u

typedef struct {
    uint8_t width;
    uint8_t height;
    const uint8_t *bands;
} CustomBitmap;

bool custom_bitmap_digit_lookup(uint8_t digit, CustomBitmap *bitmap);

#endif
