# 160×64 RGB matrix display

The matrix display is a separate native C display implementation. It uses a logical 160×64 RGB565 framebuffer and LVGL for text/layout rendering. JavaFX only displays the resulting RGB565 pixels.

## LVGL build scope

`native/lv_conf.h` is the single LVGL feature-configuration source for this experiment. It explicitly disables themes, flex/grid layouts, complex software drawing and every widget except `label`.

LVGL's upstream CMake target recursively adds the complete `src` tree even when many features are disabled. The experiment therefore removes whole optional source groups that are disabled by `lv_conf.h`, including platform drivers, non-software draw backends, optional libraries, unused widgets, unused themes/layouts and non-CLIB standard-library backends.

For LVGL v9.6.0 this reduces the source set from 551 to 124 C/C++/ASM files (about 77.5%). The retained code remains upstream LVGL; no LVGL sources are forked or modified.

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
        MATRIX_FONT_UNSCII;
theme.large_value.font.size = 16;
```

The screen layout does not change.

## Pixel-exact font rendering

The active matrix fonts are 1-bpp bitmap fonts. No antialiasing or intermediate alpha levels are used.

The font registry currently exposes two 1-bpp families:

| Family | Sizes | Notes |
| --- | --- | --- |
| `MATRIX_FONT_UNSCII` | 8, 10, 16 | LVGL UNSCII 8/16 plus local 10px variant |
| `MATRIX_FONT_VBZ` | 9, 10 | Original VBZ pixel glyphs plus derived 10px variant |

The VBZ family is adapted from `vbzfont` in `sschueller/vbz-fahrgastinformation` (Copyright 2022 Stefan Schüller, MIT). The 9px variant preserves the original 1-bpp bitmap glyphs and proportional advances. The 10px variant uses the same glyph shapes with nearest-neighbor vertical expansion and remains strictly 1-bpp.

The JavaFX simulator exposes both families so the same race screen can be compared visually before a default is chosen.

## Physical HUB75 output

The renderer works only in logical 160×64 coordinates. Five-panel ordering, scan pattern and multiplexing remain responsibilities of the later HUB75 hardware adapter.
