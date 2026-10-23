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

The current demo screen supports four preview layouts:

- `Registraties`: the default preview shows three full-width registration
  rows with team number, main time, hundredths and ranking. The ranking suffix
  is a separate raised VBZ `e`; the footer spans the full 160-pixel width;
- `Registraties (>4)`: a fifth registration uses the footer area and the
  footer disappears completely;
- `Registraties + klaarstaan`: three compact incoming-team rows on the left
  and up to four ready teams on the right. The footer remains full width;
- `Registraties + klaarstaan (>4 klaar)`: the right side extends into the
  footer area for additional ready teams. The footer remains only on the left
  and is therefore 81 pixels wide.

The two mixed layouts show the runner and warmup sprites. Registration-only
layouts have no middle divider and no sprites. All layouts keep the header with
location, adaptive clock, static finish flag, counter and conditional
connection/sync statuses. There is no `Prepare` text.

The current values are demo data in `matrix_host_main.c`. Because the real
race-data feed is not connected yet, JavaFX exposes a temporary `Layout`
dropdown for selecting these four preview modes. Later the same native layout
selection will be derived from the real registration and ready-team counts.

The main vertical divider is at x=81. This keeps the 160-pixel display visually centered while assigning the one-pixel separator itself to the middle boundary. Layout coordinates for mixed-mode content are derived from this divider instead of from the previous x=90 split.

The finished-team counter in the upper-right header is preceded by a small
static 7×7 finish flag drawn as literal 1×1 checker pixels. It deliberately has
no pole and has its own `flag_color`, independent of the runner/warmup
`sprite_color`.

Two optional 11×11 warning blocks can appear to the right of the counter. The
WiFi warning uses two white horizontal signal bars, each two pixels high, plus
a separate diagonal strike-through. The local registration-server warning uses
two diagonally offset white chain links based on the conventional link icon,
with the same strike-through. The extra 11×11 resolution gives the two links
enough separation to remain recognizable.

A third 11×11 status block appears while synchronization is active. It uses a green background with two compact white curved arrows centered inside
the 11×11 block, leaving a visible green margin around the symbol. The upper arc
ends in a right-side arrowhead and the lower arc ends in a left-side arrowhead. Sync is an
active state rather than an error, so it has no red strike-through.

A warning is not rendered while its connection is healthy. The strike-through
runs from outer corner to outer corner and is two pixels thick across the
diagonal, has its own configurable `warning_strike_color` and defaults to red;
the icon remains white on the blue warning background. Visible status blocks
own the right edge of the header; the counter and flag shift left only by the
space needed for the active blocks. Adjacent status blocks have a one-pixel
separator. The warning/status blocks start at y=0; the finish flag starts one
pixel lower at y=1. When the status group grows into the normal clock area, the
clock field shrinks from the right and its centered text therefore moves left.

The first passed-runner row reserves the far-left 10 pixels for the runner
sprite. Its number starts at x=12 with a 21-pixel field, so three-digit values
such as `999` fit before the time field starts at x=33. The runner uses the original six-frame
legacy SpriteMap1 gait, nearest-neighbour scaled from 7×7 to 10×12. No extra
arm layer or independently designed gait is added; preserving the recognizable
legacy running motion is the current priority.

The first ready-runner row starts immediately after the divider: x=81 is the
separator itself and x=82 is the first usable pixel on the right. The first 10
pixels there are reserved for the warmup sprite. Its body and legs stay fixed while the arms move
through the stretch cycle; the warmup animation advances at half the runner
rate.

Sprite bitmaps, frame selection and drawing remain native C responsibilities.
JavaFX sends the exact master animation frame to the native host and displays
the resulting RGB565 frame.

For visual sprite debugging, JavaFX exposes manual frame controls. The automatic
animation can be paused and stepped one master frame backward or forward. The
status label shows the corresponding runner and warmup frame numbers, making
arm/leg phase and accidental pixel thickening inspectable on the real 160×64
layout.

## Theme configuration

Colors and fonts are configured through `MatrixTheme`, not in the layout code.

Each text role has its own `MatrixTextStyle`:

- `header`;
- `row_number`;
- `row_time`;
- `large_value`;
- `footer`.

A style contains both a color and a font specification. The theme separately owns
`background_color`, `line_color`, `sprite_color`, `flag_color`,
`warning_background_color`, `warning_text_color`,
`warning_strike_color`, `sync_background_color` and `sync_icon_color`.
Runner and warmup
sprites share `sprite_color`. The finish flag and warning blocks are therefore
independently configurable; warning defaults are blue with white glyphs.

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
