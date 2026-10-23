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
- three passed-runner rows on the left;
- two large-value rows on the right;
- a 10×12 runner sprite at the far left of the first passed-runner row;
- a 10×12 warmup/stretch sprite before the first ready-runner value on the right;
- one footer text line;
- configurable separator lines;
- no `Prepare` text.

The current values are demo data in `matrix_host_main.c`. They will later come from the synchronized application-data model.

The first passed-runner row reserves the far-left 10 pixels for the runner
sprite, so its number starts at x=12. The runner keeps the original six-frame
legacy SpriteMap1 gait for body and legs, scaled from 7×7 to 10×12. The extra
matrix pixels add an explicit arm swing that reverses through the gait cycle.

The first ready-runner row reserves the first 10 pixels after the vertical
divider for the warmup sprite. Its body and legs stay fixed while the arms move
through the stretch cycle; the warmup animation advances at half the runner
rate.

Sprite bitmaps, frame selection and drawing remain native C responsibilities.
JavaFX only sends a periodic animation tick to the native host and displays the
resulting RGB565 frame.

## Theme configuration

Colors and fonts are configured through `MatrixTheme`, not in the layout code.

Each text role has its own `MatrixTextStyle`:

- `header`;
- `row_number`;
- `row_time`;
- `large_value`;
- `footer`.

A style contains both a color and a font specification. The theme separately owns `background_color`, `line_color` and `sprite_color`. Runner and warmup sprites share `sprite_color` and are independent of all text colors.

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
| `MATRIX_FONT_VBZ` | 9; 12 for `large_value` | Original VBZ 9px glyphs plus a dedicated 12px numeric font for large values |

The VBZ family is adapted from `vbzfont` in `sschueller/vbz-fahrgastinformation` (Copyright 2022 Stefan Schüller, MIT). The 9px variant preserves the original 1-bpp bitmap glyphs and proportional advances. Large numeric values can use a separate hand-drawn 12px bitmap font; it is intentionally numeric-only rather than a mechanically stretched general-purpose VBZ font.

The JavaFX simulator exposes both families for comparison. VBZ is the default: 9px for header, rows and footer, with the dedicated 12px numeric font for large values.

## Physical HUB75 output

The renderer works only in logical 160×64 coordinates. Five-panel ordering, scan pattern and multiplexing remain responsibilities of the later HUB75 hardware adapter.
