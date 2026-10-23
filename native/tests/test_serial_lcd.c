#include "serial_lcd.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void feed_text(SerialLcd *lcd, const char *text) {
    serial_lcd_feed(
            lcd,
            (const uint8_t *)text,
            strlen(text));
}

static void test_framebuffer_bounds(void) {
    DisplayFramebuffer framebuffer;
    display_framebuffer_clear(&framebuffer);

    display_framebuffer_set(&framebuffer, 89, 34, true);
    display_framebuffer_set(&framebuffer, 90, 35, true);

    assert(display_framebuffer_get(&framebuffer, 89, 34));
    assert(!display_framebuffer_get(&framebuffer, 90, 35));
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

    serial_lcd_feed(&lcd, &marker, 1);

    assert(lcd.cursor_x == 7);
    assert(display_framebuffer_get(&lcd.framebuffer, 2, 0));
    assert(display_framebuffer_get(&lcd.framebuffer, 0, 7));
    assert(lcd.sprite_frame == 1);
}

static void test_repeated_sprite_markers_advance_frames(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {
        0xfe, 0xfe,
        0xfe, 0xfe
    };

    serial_lcd_feed(&lcd, input, sizeof(input));

    assert(lcd.cursor_x == 14);
    assert(lcd.sprite_frame == 2);
}

static void test_single_sprite_marker_falls_back_to_character_254(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    const uint8_t input[] = {0xfe, 'A'};
    serial_lcd_feed(&lcd, input, sizeof(input));

    assert(lcd.cursor_x == 14);

    for (uint16_t x = 0; x < 7; x++) {
        assert(display_framebuffer_get(
                &lcd.framebuffer,
                x,
                7));
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
            assert(!display_framebuffer_get(
                    &lcd.framebuffer,
                    x,
                    y));
        }
    }
}

static void test_position_command_sets_y_and_x_together(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "/L3489");

    assert(lcd.cursor_y == 34);
    assert(lcd.cursor_x == 89);
}

static void test_position_command_can_be_split_across_reads(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "/L05");

    assert(lcd.cursor_y == 0);
    assert(lcd.cursor_x == 0);

    feed_text(&lcd, "10");

    assert(lcd.cursor_y == 5);
    assert(lcd.cursor_x == 10);
}

static void test_position_command_rejects_invalid_coordinate(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "/L3590");

    assert(lcd.cursor_y == 0);
    assert(lcd.cursor_x == 0);
}

static void test_position_command_rejects_non_digits(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "/LA510");

    assert(lcd.cursor_y == 0);
    assert(lcd.cursor_x == 0);
}

static void test_clear_command_requires_filler(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "A/C");

    assert(display_framebuffer_get(
            &lcd.framebuffer,
            0,
            2));

    feed_text(&lcd, "?");

    assert(!display_framebuffer_get(
            &lcd.framebuffer,
            0,
            2));
    assert(lcd.cursor_y == 0);
    assert(lcd.cursor_x == 0);
}

static void test_clear_filler_value_is_ignored(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "A/CX");

    assert(!display_framebuffer_get(
            &lcd.framebuffer,
            0,
            2));
    assert(lcd.cursor_y == 0);
    assert(lcd.cursor_x == 0);
}

static void test_bitmap_digit_draws_at_cursor(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "/L0302/B0");

    assert(lcd.cursor_y == 3);
    assert(lcd.cursor_x == 2);
    assert(!display_framebuffer_get(
            &lcd.framebuffer,
            2,
            3));
    assert(display_framebuffer_get(
            &lcd.framebuffer,
            2,
            7));
    assert(display_framebuffer_get(
            &lcd.framebuffer,
            2,
            10));
    assert(display_framebuffer_get(
            &lcd.framebuffer,
            2,
            17));
}

static void test_invalid_bitmap_digit_is_ignored(void) {
    SerialLcd lcd;
    serial_lcd_init(&lcd);

    feed_text(&lcd, "/BX");

    for (uint16_t y = 0; y < DISPLAY_HEIGHT; y++) {
        for (uint16_t x = 0; x < DISPLAY_WIDTH; x++) {
            assert(!display_framebuffer_get(
                    &lcd.framebuffer,
                    x,
                    y));
        }
    }
}

int main(void) {
    test_framebuffer_bounds();
    test_custom_character_map_is_rendered();
    test_custom_character_map_is_proportional();
    test_full_custom_character_range_is_available();
    test_sprite_marker_can_be_split_across_reads();
    test_repeated_sprite_markers_advance_frames();
    test_single_sprite_marker_falls_back_to_character_254();
    test_backspace_uses_proportional_character_position();
    test_position_command_sets_y_and_x_together();
    test_position_command_can_be_split_across_reads();
    test_position_command_rejects_invalid_coordinate();
    test_position_command_rejects_non_digits();
    test_clear_command_requires_filler();
    test_clear_filler_value_is_ignored();
    test_bitmap_digit_draws_at_cursor();
    test_invalid_bitmap_digit_is_ignored();

    puts("serial_lcd tests passed");
    return 0;
}
