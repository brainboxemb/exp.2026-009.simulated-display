#include "matrix_serial.h"

#include "matrix_font.h"
#include "matrix_lvgl.h"
#include "matrix_sprite.h"

#include "lvgl.h"

#include <ctype.h>
#include <string.h>

#define MATRIX_SERIAL_COMMAND_MAX 15u

#define ICON_FINISH 0u
#define ICON_WIFI 1u
#define ICON_LINK 2u
#define ICON_SYNC 3u

#define NOTIFICATION_WIFI 1u
#define NOTIFICATION_LINK 2u
#define NOTIFICATION_Y 0u
#define NOTIFICATION_WIDTH 11u
#define NOTIFICATION_GAP 1u

static const uint8_t FINISH_FLAG[7] = {
    0x55, 0x2a, 0x55, 0x2a, 0x55, 0x2a, 0x55
};

static const uint16_t WARNING_WIFI[11] = {
    0x000, 0x3fe, 0x3fe, 0x000, 0x0f8, 0x0f8,
    0x000, 0x060, 0x060, 0x000, 0x000
};

static const uint16_t WARNING_LINK[11] = {
    0x00e, 0x011, 0x021, 0x042, 0x074, 0x090,
    0x170, 0x210, 0x420, 0x440, 0x380
};

static const uint16_t WARNING_STRIKE[11] = {
    0x600, 0x300, 0x180, 0x0c0, 0x060, 0x030,
    0x018, 0x00c, 0x006, 0x003, 0x003
};

static const uint16_t STATUS_SYNC[11] = {
    0x000, 0x008, 0x3fc, 0x3fe, 0x008, 0x000,
    0x080, 0x1fe, 0x3fe, 0x080, 0x000
};

static size_t pixel_index(
        uint16_t x,
        uint16_t y) {
    return (size_t)y * MATRIX_DISPLAY_WIDTH + x;
}

static bool valid_coordinate(
        uint16_t x,
        uint16_t y) {
    return x < MATRIX_DISPLAY_WIDTH
            && y < MATRIX_DISPLAY_HEIGHT;
}

static void page_clear(
        MatrixSerialPage *page) {
    memset(
            page->pixels,
            MATRIX_SERIAL_TRANSPARENT,
            sizeof(page->pixels));
    page->sprite_count = 0u;
}

static void page_set(
        MatrixSerialPage *page,
        uint16_t x,
        uint16_t y,
        uint8_t color) {
    if (!valid_coordinate(x, y)
            || color >= MATRIX_SERIAL_PALETTE_SIZE) {
        return;
    }

    page->pixels[pixel_index(x, y)] = color;
}

static void fill_rect(
        MatrixSerialDisplay *display,
        uint16_t x,
        uint16_t y,
        uint16_t width,
        uint16_t height,
        uint8_t color) {
    MatrixSerialPage *page =
            &display->pages[display->selected_page];

    for (uint16_t row = 0u;
            row < height;
            row++) {
        for (uint16_t column = 0u;
                column < width;
                column++) {
            page_set(
                    page,
                    (uint16_t)(x + column),
                    (uint16_t)(y + row),
                    color);
        }
    }
}

static MatrixFontSpec selected_font(
        const MatrixSerialDisplay *display) {
    MatrixFontSpec font = {
        .family = MATRIX_FONT_VBZ,
        .size = display->font_size
    };

    if (display->font_size == 8u
            || display->font_size == 10u
            || display->font_size == 16u) {
        font.family = MATRIX_FONT_UNSCII;
    }

    return font;
}

static const lv_font_t *resolve_selected_font(
        const MatrixSerialDisplay *display) {
    const MatrixFontSpec font =
            selected_font(display);

    if (font.family == MATRIX_FONT_VBZ
            && (font.size == 12u
                    || font.size == 14u)) {
        return matrix_large_font_resolve(font);
    }

    return matrix_font_resolve(font);
}

static bool set_font_size(
        MatrixSerialDisplay *display,
        uint8_t size) {
    switch (size) {
        case 8u:
        case 9u:
        case 10u:
        case 12u:
        case 14u:
        case 16u:
            display->font_size = size;
            return true;

        default:
            return false;
    }
}

static void draw_character(
        MatrixSerialDisplay *display,
        uint8_t character) {
    matrix_lvgl_ensure_initialized();

    const lv_font_t *font =
            resolve_selected_font(display);

    if (font == NULL) {
        return;
    }

    lv_font_glyph_dsc_t glyph = {0};

    if (!font->get_glyph_dsc(
            font,
            &glyph,
            character,
            0u)) {
        return;
    }

    const uint16_t advance =
            glyph.adv_w > 0u
                    ? glyph.adv_w
                    : glyph.box_w;

    if (display->background
            != MATRIX_SERIAL_TRANSPARENT) {
        fill_rect(
                display,
                display->cursor_x,
                display->cursor_y,
                advance,
                (uint16_t)font->line_height,
                display->background);
    }

    lv_draw_buf_t *draw_buffer =
            lv_draw_buf_create(
                    glyph.box_w,
                    glyph.box_h,
                    LV_COLOR_FORMAT_A8,
                    LV_STRIDE_AUTO);

    if (draw_buffer != NULL) {
        const void *bitmap =
                font->get_glyph_bitmap(
                        &glyph,
                        draw_buffer);

        if (bitmap != NULL) {
            const int32_t left =
                    (int32_t)display->cursor_x
                    + (int32_t)glyph.ofs_x;
            const int32_t top =
                    (int32_t)display->cursor_y
                    + (int32_t)font->line_height
                    - (int32_t)font->base_line
                    - (int32_t)glyph.box_h
                    - (int32_t)glyph.ofs_y;
            MatrixSerialPage *page =
                    &display->pages[
                            display->selected_page];

            for (uint16_t row = 0u;
                    row < glyph.box_h;
                    row++) {
                for (uint16_t column = 0u;
                        column < glyph.box_w;
                        column++) {
                    const uint8_t alpha =
                            draw_buffer->data[
                                    (size_t)row
                                    * draw_buffer->header.stride
                                    + column];

                    if (alpha == 0u) {
                        continue;
                    }

                    const int32_t x =
                            left + (int32_t)column;
                    const int32_t y =
                            top + (int32_t)row;

                    if (x < 0 || y < 0) {
                        continue;
                    }

                    page_set(
                            page,
                            (uint16_t)x,
                            (uint16_t)y,
                            display->foreground);
                }
            }
        }

        lv_draw_buf_destroy(draw_buffer);
    }

    display->cursor_x =
            (uint16_t)(
                    display->cursor_x
                    + advance);
}

static void draw_line(
        MatrixSerialDisplay *display,
        uint16_t target_x,
        uint16_t target_y) {
    int32_t x0 = (int32_t)display->cursor_x;
    int32_t y0 = (int32_t)display->cursor_y;
    const int32_t x1 = (int32_t)target_x;
    const int32_t y1 = (int32_t)target_y;
    const int32_t dx =
            x1 >= x0
                    ? x1 - x0
                    : x0 - x1;
    const int32_t sx =
            x0 < x1 ? 1 : -1;
    const int32_t dy =
            -(y1 >= y0
                    ? y1 - y0
                    : y0 - y1);
    const int32_t sy =
            y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;
    MatrixSerialPage *page =
            &display->pages[display->selected_page];

    for (;;) {
        if (x0 >= 0 && y0 >= 0) {
            page_set(
                    page,
                    (uint16_t)x0,
                    (uint16_t)y0,
                    display->foreground);
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int32_t doubled =
                2 * error;

        if (doubled >= dy) {
            error += dy;
            x0 += sx;
        }

        if (doubled <= dx) {
            error += dx;
            y0 += sy;
        }
    }

    display->cursor_x = target_x;
    display->cursor_y = target_y;
}

static void draw_rectangle(
        MatrixSerialDisplay *display,
        uint16_t target_x,
        uint16_t target_y,
        bool filled) {
    const uint16_t start_x =
            display->cursor_x;
    const uint16_t start_y =
            display->cursor_y;
    const uint16_t left =
            start_x < target_x
                    ? start_x
                    : target_x;
    const uint16_t right =
            start_x > target_x
                    ? start_x
                    : target_x;
    const uint16_t top =
            start_y < target_y
                    ? start_y
                    : target_y;
    const uint16_t bottom =
            start_y > target_y
                    ? start_y
                    : target_y;
    MatrixSerialPage *page =
            &display->pages[
                    display->selected_page];

    if (filled) {
        for (uint16_t y = top;
                y <= bottom;
                y++) {
            for (uint16_t x = left;
                    x <= right;
                    x++) {
                page_set(
                        page,
                        x,
                        y,
                        display->foreground);
            }
        }
    } else {
        for (uint16_t x = left;
                x <= right;
                x++) {
            page_set(
                    page,
                    x,
                    top,
                    display->foreground);
            page_set(
                    page,
                    x,
                    bottom,
                    display->foreground);
        }

        for (uint16_t y = top;
                y <= bottom;
                y++) {
            page_set(
                    page,
                    left,
                    y,
                    display->foreground);
            page_set(
                    page,
                    right,
                    y,
                    display->foreground);
        }
    }

    display->cursor_x = target_x;
    display->cursor_y = target_y;
}

static uint8_t icon_color_count(
        uint8_t icon) {
    switch (icon) {
        case ICON_FINISH:
        case ICON_SYNC:
            return 2u;

        case ICON_WIFI:
        case ICON_LINK:
            return 3u;

        default:
            return 0u;
    }
}

static uint8_t icon_width(
        uint8_t icon) {
    return icon == ICON_FINISH
            ? 7u
            : 11u;
}

static uint8_t icon_height(
        uint8_t icon) {
    return icon == ICON_FINISH
            ? 7u
            : 11u;
}

static uint8_t icon_role(
        uint8_t icon,
        uint8_t x,
        uint8_t y) {
    if (icon == ICON_FINISH) {
        const uint8_t mask =
                (uint8_t)(
                        1u
                        << (6u - x));
        return (FINISH_FLAG[y] & mask) != 0u
                ? 1u
                : 0u;
    }

    const uint16_t mask =
            (uint16_t)(
                    1u
                    << (10u - x));

    if ((icon == ICON_WIFI
            || icon == ICON_LINK)
            && (WARNING_STRIKE[y] & mask) != 0u) {
        return 2u;
    }

    const uint16_t *glyph = NULL;

    if (icon == ICON_WIFI) {
        glyph = WARNING_WIFI;
    } else if (icon == ICON_LINK) {
        glyph = WARNING_LINK;
    } else if (icon == ICON_SYNC) {
        glyph = STATUS_SYNC;
    }

    if (glyph != NULL
            && (glyph[y] & mask) != 0u) {
        return 1u;
    }

    return 0u;
}

static bool draw_icon(
        MatrixSerialDisplay *display,
        uint8_t icon) {
    if (icon_color_count(icon) == 0u) {
        return false;
    }

    MatrixSerialPage *page =
            &display->pages[
                    display->selected_page];
    const uint8_t width = icon_width(icon);
    const uint8_t height = icon_height(icon);

    for (uint8_t y = 0u; y < height; y++) {
        for (uint8_t x = 0u; x < width; x++) {
            const uint8_t role =
                    icon_role(icon, x, y);

            page_set(
                    page,
                    (uint16_t)(display->cursor_x + x),
                    (uint16_t)(display->cursor_y + y),
                    display->icon_colors[icon][role]);
        }
    }

    return true;
}

static bool set_notification(
        MatrixSerialDisplay *display,
        uint8_t notification,
        bool active) {
    if (notification < NOTIFICATION_WIFI
            || notification > NOTIFICATION_LINK) {
        return false;
    }

    display->notifications[notification - 1u] =
            active;
    return true;
}

static void compose_icon_at(
        const MatrixSerialDisplay *display,
        MatrixFramebuffer *framebuffer,
        uint8_t icon,
        uint16_t start_x,
        uint16_t start_y) {
    const uint8_t width = icon_width(icon);
    const uint8_t height = icon_height(icon);

    for (uint8_t y = 0u; y < height; y++) {
        for (uint8_t x = 0u; x < width; x++) {
            const uint8_t role =
                    icon_role(icon, x, y);
            const uint16_t target_x =
                    (uint16_t)(start_x + x);
            const uint16_t target_y =
                    (uint16_t)(start_y + y);

            if (!valid_coordinate(
                    target_x,
                    target_y)) {
                continue;
            }

            matrix_framebuffer_set(
                    framebuffer,
                    target_x,
                    target_y,
                    display->palette[
                            display->icon_colors[
                                    icon][role]]);
        }
    }
}

static void compose_notifications(
        const MatrixSerialDisplay *display,
        MatrixFramebuffer *framebuffer) {
    uint8_t active_count = 0u;

    for (uint8_t index = 0u;
            index < MATRIX_SERIAL_NOTIFICATION_COUNT;
            index++) {
        if (display->notifications[index]) {
            active_count++;
        }
    }

    if (active_count == 0u) {
        return;
    }

    uint16_t x =
            (uint16_t)(
                    MATRIX_DISPLAY_WIDTH
                    - active_count * NOTIFICATION_WIDTH
                    - (active_count - 1u)
                            * NOTIFICATION_GAP);

    if (display->notifications[
            NOTIFICATION_WIFI - 1u]) {
        compose_icon_at(
                display,
                framebuffer,
                ICON_WIFI,
                x,
                NOTIFICATION_Y);
        x =
                (uint16_t)(
                        x
                        + NOTIFICATION_WIDTH
                        + NOTIFICATION_GAP);
    }

    if (display->notifications[
            NOTIFICATION_LINK - 1u]) {
        compose_icon_at(
                display,
                framebuffer,
                ICON_LINK,
                x,
                NOTIFICATION_Y);
    }
}

static bool place_sprite(
        MatrixSerialDisplay *display,
        uint8_t kind) {
    if (kind > (uint8_t)MATRIX_SPRITE_WARMUP) {
        return false;
    }

    MatrixSerialPage *page =
            &display->pages[display->selected_page];

    if (page->sprite_count
            >= MATRIX_SERIAL_SPRITE_CAPACITY) {
        return false;
    }

    MatrixSerialSprite *sprite =
            &page->sprites[page->sprite_count++];

    sprite->kind = kind;
    sprite->x = display->cursor_x;
    sprite->y = display->cursor_y;
    sprite->color = display->foreground;
    return true;
}

static bool decimal_pair(
        const char *text,
        uint8_t *value) {
    if (!isdigit((unsigned char)text[0])
            || !isdigit((unsigned char)text[1])) {
        return false;
    }

    *value =
            (uint8_t)(
                    (text[0] - '0') * 10
                    + text[1] - '0');
    return true;
}

static bool coordinate(
        const char *text,
        uint16_t *y,
        uint16_t *x) {
    for (uint8_t index = 0u;
            index < 5u;
            index++) {
        if (!isdigit(
                (unsigned char)text[index])) {
            return false;
        }
    }

    *y =
            (uint16_t)(
                    (text[0] - '0') * 10
                    + text[1] - '0');
    *x =
            (uint16_t)(
                    (text[2] - '0') * 100
                    + (text[3] - '0') * 10
                    + text[4] - '0');

    return *y < MATRIX_DISPLAY_HEIGHT
            && *x < MATRIX_DISPLAY_WIDTH;
}

static bool configure_icon(
        MatrixSerialDisplay *display) {
    if (display->command_length < 3u
            || !isdigit(
                    (unsigned char)display->command[2])) {
        return false;
    }

    const uint8_t icon =
            (uint8_t)(display->command[2] - '0');
    const uint8_t colors =
            icon_color_count(icon);

    if (colors == 0u) {
        return false;
    }

    for (uint8_t index = 0u;
            index < colors;
            index++) {
        const uint8_t offset =
                (uint8_t)(3u + index * 3u);

        if (display->command[offset] != 'C') {
            return false;
        }

        uint8_t color = 0u;

        if (!decimal_pair(
                &display->command[offset + 1u],
                &color)
                || color >= MATRIX_SERIAL_PALETTE_SIZE) {
            return false;
        }

        display->icon_colors[icon][index] =
                color;
    }

    return true;
}

static bool execute_command(
        MatrixSerialDisplay *display) {
    if (display->command_length == 2u
            && memcmp(
                    display->command,
                    "UB",
                    2u) == 0) {
        if (display->update_active) {
            return false;
        }

        display->update_active = true;
        return true;
    }

    if (display->command_length == 2u
            && memcmp(
                    display->command,
                    "UE",
                    2u) == 0) {
        if (!display->update_active) {
            return false;
        }

        display->update_active = false;
        return true;
    }

    if (display->command_length == 2u
            && memcmp(
                    display->command,
                    "PC",
                    2u) == 0) {
        page_clear(
                &display->pages[
                        display->selected_page]);

        display->cursor_x = 0u;
        display->cursor_y = 0u;
        return true;
    }

    if (display->command_length == 3u
            && memcmp(
                    display->command,
                    "PS",
                    2u) == 0
            && display->command[2] >= '0'
            && display->command[2] <= '2') {
        display->selected_page =
                (uint8_t)(
                        display->command[2] - '0');
        return true;
    }

    if (display->command_length == 6u
            && display->command[0] == 'L') {
        uint16_t y = 0u;
        uint16_t x = 0u;

        if (!coordinate(
                &display->command[1],
                &y,
                &x)) {
            return false;
        }

        display->cursor_x = x;
        display->cursor_y = y;
        return true;
    }

    if (display->command_length == 7u
            && (memcmp(
                    display->command,
                    "DL",
                    2u) == 0
                || memcmp(
                    display->command,
                    "DR",
                    2u) == 0
                || memcmp(
                    display->command,
                    "DF",
                    2u) == 0)) {
        uint16_t y = 0u;
        uint16_t x = 0u;

        if (!coordinate(
                &display->command[2],
                &y,
                &x)) {
            return false;
        }

        if (memcmp(
                display->command,
                "DL",
                2u) == 0) {
            draw_line(display, x, y);
        } else {
            draw_rectangle(
                    display,
                    x,
                    y,
                    display->command[1] == 'F');
        }

        return true;
    }

    if (display->command_length == 3u
            && memcmp(
                    display->command,
                    "BI",
                    2u) == 0
            && isdigit(
                    (unsigned char)display->command[2])) {
        return draw_icon(
                display,
                (uint8_t)(
                        display->command[2] - '0'));
    }

    if (display->command_length >= 3u
            && memcmp(
                    display->command,
                    "CI",
                    2u) == 0) {
        return configure_icon(display);
    }

    if (display->command_length == 2u
            && display->command[0] == 'S'
            && isdigit(
                    (unsigned char)display->command[1])) {
        return place_sprite(
                display,
                (uint8_t)(
                        display->command[1] - '0'));
    }

    if (display->command_length == 3u
            && memcmp(
                    display->command,
                    "SV",
                    2u) == 0
            && display->command[2] >= '1'
            && display->command[2] <= '9') {
        display->sprite_velocity =
                (uint8_t)(
                        display->command[2] - '0');
        display->sprite_elapsed_ticks = 0u;
        return true;
    }

    if (display->command_length == 3u
            && display->command[0] == 'N'
            && display->command[1] >= '1'
            && display->command[1] <= '2'
            && (display->command[2] == '0'
                    || display->command[2] == '1')) {
        return set_notification(
                display,
                (uint8_t)(
                        display->command[1] - '0'),
                display->command[2] == '1');
    }

    if (display->command_length == 4u) {
        uint8_t value = 0u;

        if (!decimal_pair(
                &display->command[2],
                &value)) {
            return false;
        }

        if (memcmp(
                display->command,
                "FS",
                2u) == 0) {
            return set_font_size(
                    display,
                    value);
        }

        if (memcmp(
                display->command,
                "CF",
                2u) == 0
                && value < MATRIX_SERIAL_PALETTE_SIZE) {
            display->foreground = value;
            return true;
        }

        if (memcmp(
                display->command,
                "CB",
                2u) == 0
                && value <= MATRIX_SERIAL_PALETTE_SIZE) {
            display->background =
                    value == MATRIX_SERIAL_PALETTE_SIZE
                            ? MATRIX_SERIAL_TRANSPARENT
                            : value;
            return true;
        }

        if (memcmp(
                display->command,
                "XB",
                2u) == 0
                && value <= 99u) {
            display->brightness = value;
            return true;
        }
    }

    return false;
}

static uint8_t expected_command_length(
        const MatrixSerialDisplay *display) {
    const uint8_t length =
            display->command_length;
    const char *command =
            display->command;

    if (length == 0u) {
        return 0u;
    }

    if (length == 1u) {
        switch (command[0]) {
            case 'P':
            case 'D':
            case 'F':
            case 'C':
            case 'X':
            case 'B':
            case 'L':
            case 'N':
            case 'S':
            case 'U':
                return 0u;

            default:
                return 0xffu;
        }
    }

    if (command[0] == 'L') {
        return 6u;
    }

    if (length >= 2u) {
        if (memcmp(command, "UB", 2u) == 0
                || memcmp(command, "UE", 2u) == 0) {
            return 2u;
        }

        if (memcmp(command, "PC", 2u) == 0) {
            return 2u;
        }

        if (memcmp(command, "PS", 2u) == 0) {
            return 3u;
        }

        if (memcmp(command, "DL", 2u) == 0
                || memcmp(command, "DR", 2u) == 0
                || memcmp(command, "DF", 2u) == 0) {
            return 7u;
        }

        if (memcmp(command, "FS", 2u) == 0
                || memcmp(command, "CF", 2u) == 0
                || memcmp(command, "CB", 2u) == 0
                || memcmp(command, "XB", 2u) == 0) {
            return 4u;
        }

        if (memcmp(command, "BI", 2u) == 0) {
            return 3u;
        }

        if (memcmp(command, "CI", 2u) == 0) {
            if (length < 3u) {
                return 0u;
            }

            if (!isdigit(
                    (unsigned char)command[2])) {
                return 0xffu;
            }

            const uint8_t icon =
                    (uint8_t)(command[2] - '0');
            const uint8_t colors =
                    icon_color_count(icon);

            if (colors == 0u) {
                return 0xffu;
            }

            return (uint8_t)(
                    3u + colors * 3u);
        }

        if (command[0] == 'S') {
            if (command[1] == 'V') {
                return 3u;
            }

            if (isdigit(
                    (unsigned char)command[1])) {
                return 2u;
            }

            return 0xffu;
        }

        if (command[0] == 'N') {
            if (command[1] == '1'
                    || command[1] == '2') {
                return 3u;
            }

            return 0xffu;
        }
    }

    return 0xffu;
}

void matrix_serial_init(
        MatrixSerialDisplay *display) {
    memset(display, 0, sizeof(*display));

    for (uint8_t page = 0u;
            page < MATRIX_SERIAL_PAGE_COUNT;
            page++) {
        page_clear(&display->pages[page]);
    }

    display->palette[0] =
            matrix_pixel_rgb565_create(0, 0, 0);
    display->palette[1] =
            matrix_pixel_rgb565_create(238, 238, 238);
    display->palette[2] =
            matrix_pixel_rgb565_create(255, 255, 255);
    display->palette[3] =
            matrix_pixel_rgb565_create(51, 51, 51);
    display->palette[4] =
            matrix_pixel_rgb565_create(128, 128, 128);
    display->palette[5] =
            matrix_pixel_rgb565_create(255, 0, 0);
    display->palette[6] =
            matrix_pixel_rgb565_create(0, 255, 0);
    display->palette[7] =
            matrix_pixel_rgb565_create(0, 128, 0);
    display->palette[8] =
            matrix_pixel_rgb565_create(255, 255, 0);
    display->palette[9] =
            matrix_pixel_rgb565_create(255, 165, 0);
    display->palette[10] =
            matrix_pixel_rgb565_create(0, 96, 255);
    display->palette[11] =
            matrix_pixel_rgb565_create(0, 255, 255);
    display->palette[12] =
            matrix_pixel_rgb565_create(255, 0, 255);
    display->palette[13] =
            matrix_pixel_rgb565_create(255, 204, 102);
    display->palette[14] =
            matrix_pixel_rgb565_create(128, 0, 128);
    display->palette[15] =
            matrix_pixel_rgb565_create(0, 0, 255);

    display->icon_colors[ICON_FINISH][0] = 0u;
    display->icon_colors[ICON_FINISH][1] = 1u;
    display->icon_colors[ICON_WIFI][0] = 10u;
    display->icon_colors[ICON_WIFI][1] = 1u;
    display->icon_colors[ICON_WIFI][2] = 5u;
    display->icon_colors[ICON_LINK][0] = 10u;
    display->icon_colors[ICON_LINK][1] = 1u;
    display->icon_colors[ICON_LINK][2] = 5u;
    display->icon_colors[ICON_SYNC][0] = 6u;
    display->icon_colors[ICON_SYNC][1] = 0u;

    display->foreground = 1u;
    display->background = MATRIX_SERIAL_TRANSPARENT;
    display->font_size = 9u;
    display->brightness = 99u;
    display->sprite_velocity = 4u;
}

bool matrix_serial_feed(
        MatrixSerialDisplay *display,
        const uint8_t *bytes,
        size_t length) {
    bool valid = true;

    for (size_t index = 0u;
            index < length;
            index++) {
        const uint8_t byte = bytes[index];

        if (!display->in_command) {
            if (byte == (uint8_t)'/') {
                display->in_command = true;
                display->command_length = 0u;
                continue;
            }

            if (byte >= 32u && byte <= 126u) {
                draw_character(display, byte);
            }

            continue;
        }

        if (byte == (uint8_t)'/') {
            display->command_length = 0u;
            valid = false;
            continue;
        }

        if (display->command_length
                >= MATRIX_SERIAL_COMMAND_MAX) {
            display->in_command = false;
            display->command_length = 0u;
            valid = false;
            continue;
        }

        display->command[
                display->command_length++] =
                (char)byte;

        const uint8_t expected =
                expected_command_length(display);

        if (expected == 0xffu) {
            display->in_command = false;
            display->command_length = 0u;
            valid = false;
            continue;
        }

        if (expected != 0u
                && display->command_length == expected) {
            valid =
                    execute_command(display)
                    && valid;
            display->in_command = false;
            display->command_length = 0u;
        }
    }

    return valid;
}

void matrix_serial_compose(
        const MatrixSerialDisplay *display,
        MatrixFramebuffer *framebuffer) {
    matrix_framebuffer_clear(framebuffer);

    for (uint8_t page_index = 0u;
            page_index < MATRIX_SERIAL_PAGE_COUNT;
            page_index++) {
        const MatrixSerialPage *page =
                &display->pages[page_index];

        for (size_t index = 0u;
                index < MATRIX_DISPLAY_PIXEL_COUNT;
                index++) {
            const uint8_t color =
                    page->pixels[index];

            if (color == MATRIX_SERIAL_TRANSPARENT) {
                continue;
            }

            framebuffer->pixels[index] =
                    display->palette[color];
        }

        for (uint8_t sprite_index = 0u;
                sprite_index < page->sprite_count;
                sprite_index++) {
            const MatrixSerialSprite *sprite =
                    &page->sprites[sprite_index];

            const uint8_t frame =
                    sprite->kind == (uint8_t)MATRIX_SPRITE_WARMUP
                            ? (uint8_t)(display->sprite_frame / 2u)
                            : (uint8_t)(
                                    display->sprite_frame
                                    % (MATRIX_SPRITE_FRAME_COUNT * 2u));

            matrix_sprite_draw(
                    framebuffer,
                    (MatrixSpriteKind)sprite->kind,
                    frame,
                    sprite->x,
                    sprite->y,
                    display->palette[sprite->color]);
        }
    }

    compose_notifications(
            display,
            framebuffer);
}

bool matrix_serial_tick(
        MatrixSerialDisplay *display) {
    if (!matrix_serial_has_sprites(display)) {
        return false;
    }

    display->sprite_elapsed_ticks++;

    if (display->sprite_elapsed_ticks
            < display->sprite_velocity) {
        return false;
    }

    display->sprite_elapsed_ticks = 0u;
    display->sprite_frame =
            (uint8_t)(
                    (display->sprite_frame + 1u)
                    % MATRIX_SPRITE_FRAME_COUNT);
    return true;
}

bool matrix_serial_has_sprites(
        const MatrixSerialDisplay *display) {
    for (uint8_t page = 0u;
            page < MATRIX_SERIAL_PAGE_COUNT;
            page++) {
        if (display->pages[page].sprite_count > 0u) {
            return true;
        }
    }

    return false;
}

bool matrix_serial_update_active(
        const MatrixSerialDisplay *display) {
    return display->update_active;
}

uint8_t matrix_serial_brightness(
        const MatrixSerialDisplay *display) {
    return display->brightness;
}
