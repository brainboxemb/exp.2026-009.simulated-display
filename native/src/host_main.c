#include "serial_lcd.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define HOST_REQUEST_ANIMATION_TICK UINT32_MAX

static void write_u16_be(uint16_t value) {
    fputc((value >> 8) & 0xff, stdout);
    fputc(value & 0xff, stdout);
}

static void write_u32_be(uint32_t value) {
    fputc((value >> 24) & 0xff, stdout);
    fputc((value >> 16) & 0xff, stdout);
    fputc((value >> 8) & 0xff, stdout);
    fputc(value & 0xff, stdout);
}

static void write_frame(const SerialLcd *lcd) {
    static const uint8_t magic[] = {'S', 'D', 'F', '1'};

    fwrite(magic, 1, sizeof(magic), stdout);
    write_u16_be(DISPLAY_WIDTH);
    write_u16_be(DISPLAY_HEIGHT);
    write_u32_be(DISPLAY_PIXEL_COUNT);

    for (uint16_t y = 0; y < DISPLAY_HEIGHT; y++) {
        for (uint16_t x = 0; x < DISPLAY_WIDTH; x++) {
            fputc(
                    display_framebuffer_get(
                            &lcd->framebuffer,
                            x,
                            y)
                            ? 1
                            : 0,
                    stdout);
        }
    }

    fflush(stdout);
}

static bool read_exact(uint8_t *buffer, size_t size) {
    size_t offset = 0;

    while (offset < size) {
        const size_t count =
                fread(
                        buffer + offset,
                        1,
                        size - offset,
                        stdin);

        if (count == 0) {
            return false;
        }

        offset += count;
    }

    return true;
}

static bool read_u32_be(uint32_t *value) {
    uint8_t bytes[4];

    if (!read_exact(bytes, sizeof(bytes))) {
        return false;
    }

    *value =
            ((uint32_t)bytes[0] << 24)
            | ((uint32_t)bytes[1] << 16)
            | ((uint32_t)bytes[2] << 8)
            | (uint32_t)bytes[3];

    return true;
}

static void write_changed_frame(SerialLcd *lcd) {
    if (!serial_lcd_dirty(lcd)) {
        return;
    }

    write_frame(lcd);
    serial_lcd_mark_clean(lcd);
}

int main(void) {
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1) {
        return 4;
    }

    if (_setmode(_fileno(stdout), _O_BINARY) == -1) {
        return 5;
    }
#endif

    SerialLcd lcd;
    serial_lcd_init(&lcd);

    write_frame(&lcd);
    serial_lcd_mark_clean(&lcd);

    uint8_t input[4096];

    for (;;) {
        uint32_t size = 0;

        if (!read_u32_be(&size)) {
            break;
        }

        if (size == HOST_REQUEST_ANIMATION_TICK) {
            serial_lcd_update_sprites(&lcd);
            write_changed_frame(&lcd);
            continue;
        }

        if (size > sizeof(input)) {
            return 2;
        }

        if (size > 0 && !read_exact(input, size)) {
            return 3;
        }

        serial_lcd_feed(&lcd, input, size);
        write_changed_frame(&lcd);
    }

    return 0;
}
