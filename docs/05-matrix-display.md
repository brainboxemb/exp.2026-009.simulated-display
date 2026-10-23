# 160×64 RGB matrix display

The matrix display is a separate native C display implementation. It uses a logical 160×64 RGB565 framebuffer and LVGL for text/layout rendering. JavaFX only displays the resulting RGB565 pixels.

## LVGL boundary

LVGL is internal to `matrix-display`. Application-facing screen data and theme structures do not expose `lv_obj_t`, `lv_font_t` or other LVGL types.

The experiment pins LVGL **v9.6.0** through CMake FetchContent. The first native configure therefore needs access to GitHub.

## First race screen

The current demo screen follows the supplied layout reference:

- header with location, clock and counter;
- three rows on the left;
- two large-value rows on the right;
- four footer values;
- configurable separator lines;
- no `Prepare` text.

The current values are demo data in `matrix_host_main.c`. They will later come from the synchronized application-data model.

## Theme configuration

Colors and fonts are configured through `MatrixTheme`, not in the layout code.

Each text role has its own `MatrixTextStyle`:

- `header`;
- `row_number`;
- `row_time`;
- `large_value`;
- `footer`.

A style contains both a color and a font specification. The theme separately owns `background_color` and `line_color`.

Example:

```c
MatrixTheme theme = matrix_theme_default_create();

theme.line_color =
        matrix_pixel_rgb565_create(0, 80, 255);

theme.large_value.color =
        matrix_pixel_rgb565_create(255, 255, 255);

theme.large_value.font.family =
        MATRIX_FONT_MONTSERRAT;
theme.large_value.font.size = 20;
```

The screen layout does not change.

## Font families and sizes

The first font registry contains:

| Family | Sizes |
| --- | --- |
| `MATRIX_FONT_UNSCII` | 8, 16 |
| `MATRIX_FONT_MONTSERRAT` | 8, 10, 12, 14, 16, 18, 20 |

UNSCII is the default because its pixel-like appearance fits a low-resolution LED matrix. Unsupported family/size combinations are rejected instead of silently choosing another font.

Adding another font family only requires extending the font registry; the screen/theme API stays unchanged.

## Physical HUB75 output

The renderer works only in logical 160×64 coordinates. Five-panel ordering, scan pattern and multiplexing remain responsibilities of the later HUB75 hardware adapter.
