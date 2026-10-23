#include "serial_lcd.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void feed_text(SerialLcd *lcd, const char *text) {
    serial_lcd_feed(lcd, (const uint8_t *)text, strlen(text));
}

static void test_framebuffer_bounds(void) {
    DisplayFramebuffer framebuffer;
    display_framebuffer_clear(&framebuffer);

    display_framebuffer_set(&framebuffer, 89, 34, true);
    display_framebuffer_set(&framebuffer, 90, 35, true);

    assert(display_framebuffer_get(&framebuffer, 89, 34));
    assert(!display_framebuffer_get(&framebuffer, 90, 35));
}

static void test_command_can_be_split_across_reads(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t a[] = {0x7c, 0x10, 12};
    const uint8_t b[] = {9};
    const uint8_t c[] = {1};

    serial_lcd_feed(&lcd, a, sizeof(a));
    serial_lcd_feed(&lcd, b, sizeof(b));
    assert(!display_framebuffer_get(&lcd.framebuffer, 12, 9));

    serial_lcd_feed(&lcd, c, sizeof(c));
    assert(display_framebuffer_get(&lcd.framebuffer, 12, 9));
}

static void test_custom_character_map_is_rendered(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "A");

    assert(!display_framebuffer_get(&lcd.framebuffer, 0, 0));
    assert(display_framebuffer_get(&lcd.framebuffer, 0, 2));
    assert(display_framebuffer_get(&lcd.framebuffer, 1, 1));
    assert(display_framebuffer_get(&lcd.framebuffer, 4, 6));
    assert(lcd.cursor_x == 6);
}

static void test_custom_character_map_is_proportional(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "AI!");

    assert(lcd.cursor_x == 12);
}

static void test_full_custom_character_range_is_available(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {31, 253, 255};
    serial_lcd_feed(&lcd, input, sizeof(input));

    assert(lcd.cursor_x == 14);
    assert(display_framebuffer_get(&lcd.framebuffer, 0, 3));
    assert(display_framebuffer_get(&lcd.framebuffer, 4, 0));
}

static void test_sprite_marker_can_be_split_across_reads(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t marker = 0xfe;
    serial_lcd_feed(&lcd, &marker, 1);

    assert(lcd.cursor_x == 0);
    assert(!display_framebuffer_get(&lcd.framebuffer, 2, 0));

    serial_lcd_feed(&lcd, &marker, 1);

    assert(lcd.cursor_x == 7);
    assert(display_framebuffer_get(&lcd.framebuffer, 2, 0));
    assert(display_framebuffer_get(&lcd.framebuffer, 0, 7));
    assert(lcd.sprite_frame == 1);
}

static void test_repeated_sprite_markers_advance_frames(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t sprite[] = {0xfe, 0xfe, 0xfe, 0xfe};
    serial_lcd_feed(&lcd, sprite, sizeof(sprite));

    assert(lcd.cursor_x == 14);
    assert(lcd.sprite_frame == 2);
    assert(display_framebuffer_get(&lcd.framebuffer, 2, 0));
    assert(!display_framebuffer_get(&lcd.framebuffer, 7, 0));
    assert(display_framebuffer_get(&lcd.framebuffer, 7, 5));
}

static void test_single_sprite_marker_falls_back_to_character_254(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {0xfe, 'A'};
    serial_lcd_feed(&lcd, input, sizeof(input));

    assert(lcd.cursor_x == 14);
    for (uint16_t x = 0; x < 7; x++) {
        assert(display_framebuffer_get(&lcd.framebuffer, x, 7));
    }
}

static void test_backspace_uses_proportional_character_position(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {'A', 'i', '\b'};
    serial_lcd_feed(&lcd, input, sizeof(input));

    assert(lcd.cursor_x == 6);
    for (uint16_t x = 6; x < 14; x++) {
        for (uint16_t y = 0; y < 8; y++) {
            assert(!display_framebuffer_get(&lcd.framebuffer, x, y));
        }
    }
}

static void test_fifteen_a_characters_fit_90_pixels(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "AAAAAAAAAAAAAAA");

    assert(lcd.cursor_x == 0);
    assert(lcd.cursor_y == 8);
    assert(display_framebuffer_get(&lcd.framebuffer, 84, 2));
}

static void test_invalid_text_origin_is_ignored(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {0x7c, 0x18, 90, 0x7c, 0x19, 28};
    serial_lcd_feed(&lcd, input, sizeof(input));

    assert(lcd.text_origin_x == 0);
    assert(lcd.text_origin_y == 0);
}

static void test_position_and_clear(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {
        0x7c, 0x18, 12,
        0x7c, 0x19, 8,
        'A',
        0x7c, 0x00
    };
    serial_lcd_feed(&lcd, input, sizeof(input));

    assert(lcd.text_origin_x == 12);
    assert(lcd.text_origin_y == 8);
    assert(lcd.cursor_x == 12);
    assert(lcd.cursor_y == 8);

    for (uint16_t y = 0; y < DISPLAY_HEIGHT; y++) {
        for (uint16_t x = 0; x < DISPLAY_WIDTH; x++) {
            assert(!display_framebuffer_get(&lcd.framebuffer, x, y));
        }
    }
}

static void test_line_command(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {0x7c, 0x0c, 1, 2, 5, 2, 1};
    serial_lcd_feed(&lcd, input, sizeof(input));

    for (uint16_t x = 1; x <= 5; x++) {
        assert(display_framebuffer_get(&lcd.framebuffer, x, 2));
    }
}

int main(void) {
    test_framebuffer_bounds();
    test_command_can_be_split_across_reads();
    test_custom_character_map_is_rendered();
    test_custom_character_map_is_proportional();
    test_full_custom_character_range_is_available();
    test_sprite_marker_can_be_split_across_reads();
    test_repeated_sprite_markers_advance_frames();
    test_single_sprite_marker_falls_back_to_character_254();
    test_backspace_uses_proportional_character_position();
    test_fifteen_a_characters_fit_90_pixels();
    test_invalid_text_origin_is_ignored();
    test_position_and_clear();
    test_line_command();

    puts("serial_lcd tests passed");
    return 0;
}
