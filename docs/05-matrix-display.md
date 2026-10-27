# 160×64 RGB matrix display

The matrix display is a separate native C display implementation. It uses a logical 160×64 RGB565 framebuffer and LVGL for text/layout rendering. JavaFX only displays the resulting RGB565 pixels.

## LVGL build scope

`native/lv_conf.h` is the single LVGL feature-configuration source for this experiment. It explicitly disables themes, flex/grid layouts, complex software drawing and every widget except `label`.

LVGL's upstream CMake target recursively adds the complete `src` tree even when many features are disabled. The experiment therefore removes whole optional source groups that are disabled by `lv_conf.h`, including platform drivers, non-software draw backends, optional libraries, unused widgets, unused themes/layouts and non-CLIB standard-library backends.

For LVGL v9.6.0 this reduces the source set from 551 to 124 C/C++/ASM files (about 77.5%). The retained code remains upstream LVGL; no LVGL sources are forked or modified.

## LVGL boundary

LVGL is internal to `matrix-display`. Application-facing screen data and theme structures do not expose `lv_obj_t`, `lv_font_t` or other LVGL types.

The experiment pins LVGL **v9.6.0** through CMake FetchContent. The first native configure therefore needs access to GitHub.

## Startup screen

The matrix simulator starts with a dedicated 160×64 splash rendered entirely by portable C in `matrix_startup_screen.c`. It is also selectable as **Startup** in the Demo layout control so the artwork can be reviewed without a timed transition.

The current design uses a logical black/off background, a two-line slanted pixel wordmark `RELAY` / `RACE`, lime and warm-yellow lettering, and short cyan/blue plus red/magenta diagonal speed streaks. The wordmark uses an explicit 5×7 block alphabet at 4× scale rather than a JavaFX font, so every pixel is deterministic and the same renderer can later be reused for the ESP32 boot image.

The splash is intentionally a separate renderer rather than another `MatrixRaceLayout`: it has no race data, theme state, connection status or sprite animation.

The Demo selector also exposes **Startup sequence**. This keeps the static splash available for artwork review while the second variant loops through the intended startup states: both warnings, WiFi scan, WiFi connected, link connecting and ready. WiFi/link indicators use the exact same shared 11×11 glyphs and top-right grouping as the normal race screen. Two indicators therefore occupy x=137 and x=149; a single remaining indicator uses x=149. Warning states remain blue/white with a red strike. Active connection work keeps the same blue background and white glyph but removes the red strike; only the glyph shape animates. This keeps WiFi/link visually consistent while still distinguishing unavailable from actively connecting. The sequence is simulator-driven at 250 ms per animation step; it does not yet change ESP32 runtime behavior.

The startup renderer receives one `MatrixStartupInfo` configuration containing the software version, one-character display ID and one-character SSID/profile ID. The simulator currently uses `A` for both example config values, matching the expected common case where display ID and SSID/profile ID use the same letter. Directly below the top-right 11×11 status indicators, two gray 3×5 lines are right-aligned to the same screen edge: `ID: A` at y=13 and `SSID: A` at y=20. The compact software version remains gray and right-aligned at the bottom-right. The mini alphabet supports uppercase A–Z and digits so the configured character identifiers are not tied to the demo values.

For the desktop simulator, CMake receives the version rather than the artwork owning it. For the desktop simulator, CMake reads the Maven project version from `pom.xml` (currently `0.1.0-SNAPSHOT`) and displays the compact release part as `v0.1.0`. This keeps `pom.xml` as the current version authority while allowing the ESP32 target to pass its own build/version value later.

## First race screen

The current demo screen supports five preview layouts:

- `Registraties`: the default preview shows three full-width registration
  rows with the same roomy 13-pixel vertical cadence as the mixed left side.
  Each row contains team number, main time, hundredths and ranking. The ranking
  suffix is a separate raised VBZ `e`; the footer spans the full 160-pixel
  width;
- `Registraties (>3)`: the fourth registration uses the former footer space
  while keeping the same 13-pixel row cadence. The footer disappears
  completely;
- `Registraties + klaarstaan`: three compact incoming-team rows on the left
  and up to four ready teams on the right. The footer remains full width;
- `Registraties + klaarstaan (>4 klaar)`: the right side extends into the
  footer area for additional ready teams. The footer remains only on the left
  and is therefore 81 pixels wide. The three right-side rows keep the first
  row at y=14 and use y=31 and y=48 for the extra rows, consuming the free
  pixels at the bottom instead of shifting the whole group upward;
- `Font test`: a dedicated non-race screen that shows `0123456789` in the
  original VBZ 9px font, the 12px numeric font and the 14px numeric font.
  Every digit uses the same fixed 14-pixel column in all three rows, so `0`
  is directly above `0`, `1` above `1`, and so on, with visible spacing
  between adjacent glyphs. The rows start at y=1, y=14 and y=30 to keep the
  comparison vertically compact. The race header, footer and sprites are
  omitted.

The two mixed layouts show the runner and warmup sprites. Registration-only
layouts have no middle divider and no sprites. All layouts keep the header with
location, adaptive clock, static finish flag, counter and conditional
connection/sync statuses. There is no `Prepare` text.

The current values are demo data in `matrix_host_main.c`. Because the real
race-data feed is not connected yet, JavaFX exposes a temporary `Layout`
dropdown for selecting these five preview modes. Later the same native layout
selection will be derived from the real registration and ready-team counts.

The exchange-point number appears at x=0 as an inverted, square-cornered badge, without a `WP` prefix. The badge fills the 11-pixel header band and uses one pixel of horizontal padding. The header text baseline is shifted one pixel down relative to the earlier layout, which gives the digits, clock and counter more visual breathing room at the top. The badge has independent `exchange_point_background_color` and `exchange_point_text_color` values. The background defaults to lime, while the digits default to soft white (`#EEEEEE`). Its width follows the rendered number, up to the reserved 39-pixel header field; the clock keeps its existing position.

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

A third 11×11 status block appears while synchronization is active. It uses a
green background with two explicit white horizontal arrows: the upper arrow
points right and the lower arrow points left. This opposing-arrow shape is used
instead of a miniature circular sync icon because it remains recognizable at
11×11 pixels. Sync is an active state rather than an error, so it has no red
strike-through.

A warning is not rendered while its connection is healthy. The strike-through
runs from outer corner to outer corner and is two pixels thick across the
diagonal, has its own configurable `warning_strike_color` and defaults to red;
the icon remains white on the blue warning background. Visible status blocks
own the right edge of the header; the counter and flag shift left only by the
space needed for the active blocks. Adjacent status blocks have a one-pixel
separator. The warning/status blocks start at y=0; the finish flag starts two
pixels lower at y=2 so it follows the one-pixel downward header shift. When the status group grows into the normal clock area, the
clock field shrinks from the right and its centered text therefore moves left.

In the mixed layout, the left-side time field is right-aligned with an explicit two-pixel gap before the x=81 vertical divider. The gap is derived from `MAIN_COLUMN_X`, so it remains intentional if the left-side geometry changes. The footer label is positioned one pixel higher at y=53; with the VBZ glyph offset its visible pixels occupy y=54..62.

The first passed-runner row reserves the far-left 10 pixels for the runner
sprite. Its number starts at x=12 with a 21-pixel field, so three-digit values
such as `999` fit before the time field starts at x=33. The runner uses the original six-frame
legacy SpriteMap1 gait, nearest-neighbour scaled from 7×7 to 10×12. No extra
arm layer or independently designed gait is added; preserving the recognizable
legacy running motion is the current priority.

The first ready-runner row starts immediately after the divider: x=81 is the
separator itself and x=82 is the first usable pixel on the right. The first 10
pixels there are reserved for the warmup sprite. With the 14px large numeric
font, the warmup sprite ends at x=91. Pixels x=92..93 remain empty, then the
first three-digit ready-team number uses a 30-pixel field from x=94 and is
right-aligned. The second value also has a 30-pixel field. Its body and legs stay fixed while the arms move
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
`background_color`, `line_color`, `sprite_color`, `exchange_point_background_color`, `exchange_point_text_color`, `flag_color`,
`warning_background_color`, `warning_text_color`,
`warning_strike_color`, `sync_background_color` and `sync_icon_color`.
Runner and warmup
sprites share `sprite_color`. The reviewed logical display uses black (`#000000`) for unlit/background pixels, warm yellow (`#FFCC66`) for the header, footer and sprites; soft white (`#EEEEEE`) for row text, finish flag and warning glyphs; black for exchange text and the sync glyph; lime for large values, exchange background and sync background; blue (`#0060FF`) for warning backgrounds; red for the warning strike; and gray (`#808080`) for divider lines. JavaFX renders logical black/off pixels as dark gray (`#333333`) only to make the simulated LED matrix readable; that gray is not sent to the framebuffer.

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
| `MATRIX_FONT_VBZ` | 9; 12/14 for `large_value` | Original VBZ 9px glyphs plus dedicated 12px and 14px numeric fonts for large values |

The VBZ family is adapted from `vbzfont` in `sschueller/vbz-fahrgastinformation` (Copyright 2022 Stefan Schüller, MIT). The 9px variant preserves the original 1-bpp bitmap glyphs and proportional advances. Large numeric values use numeric-only bitmap fonts authored at their final
pixel sizes. The 12px variant uses 9×12 glyph cells and the 14px variant uses
10×14 glyph cells. Both follow the original VBZ digit topology, but they are
not produced by nearest-neighbour scaling: stems and curves are placed
explicitly so vertical strokes remain two pixels thick and symmetric. Three
14px digits still fit exactly inside a 30-pixel ready-team field.

The JavaFX simulator exposes both families for comparison. VBZ is the default: 9px for header, rows and footer, with the dedicated 14px numeric font for large values. The 12px numeric variant remains selectable for comparison. The dedicated
`Font test` layout shows the original 9px VBZ digits together with the 12px
and 14px numeric variants in aligned fixed-width columns. This is the preferred
visual check when adjusting individual glyphs such as the lower curve of the
14px `9`.

## Physical HUB75 output

The renderer works only in logical 160×64 coordinates. Five-panel ordering, scan pattern and multiplexing remain responsibilities of the later HUB75 hardware adapter.


Normal full-width footers use the VBZ font's native letter and word spacing. Only the 81-pixel ready-overflow footer uses `-1` letter spacing to keep the compact demo footer content inside its half-width field.
