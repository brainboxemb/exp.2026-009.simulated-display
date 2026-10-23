# Experiment 009 — Simulated LED displays

Proof of Principle for running **native C display software** behind a **Java 21 / JavaFX 21** desktop simulator, with the portable C core intended to remain usable later on ESP32 / ESP-IDF.

Status: **active — first 90×35 display vertical slice implemented on PR #2; qualification in progress**.

- [Cross-project record](https://github.com/brainboxemb/brainboxemb.meta/blob/main/experiments/009-simulated-display/README.md)
- [Meta tracking issue #188](https://github.com/brainboxemb/brainboxemb.meta/issues/188)
- [Implementation issue #1](https://github.com/brainboxemb/exp.2026-009.simulated-display/issues/1)

## Reference displays

| Mode | Pixels | Output | Interpretation |
| --- | --- | --- | --- |
| Serial LED | 90 × 35 | Red monochrome LEDs | Legacy serial character and slash-command stream interpreted in C |
| HUB75 | 160 × 64 (five panels) | RGB LEDs | Synchronized application data with a separate native C interpreter |

## Current architecture

```text
serial bytes
    |
    v
native C streaming parser
    |
    v
custom character + sprite + bitmap renderer
    |
    v
90x35 bit-packed framebuffer
    |
    v
native display-host process
    |
    | framed pixel updates
    v
JavaFX red LED matrix
```

JavaFX does not interpret display commands or rasterize characters. It launches the native process, sends raw serial chunks and paints the pixels returned by C.

The native side now has two explicit libraries and two desktop host processes:

- `legacy-display`: 90×35 bit-packed monochrome plus legacy protocol/renderer;
- `matrix-display`: 160×64 RGB565 framebuffer plus LVGL-based screen rendering;
- `display-host`: legacy 90×35 serial display host;
- `matrix-display-host`: RGB565/LVGL matrix host used by the JavaFX matrix window.

The matrix demo now renders the first race-screen layout in C/LVGL rather than a color-bar test. Colors and font family/size are supplied through `MatrixTheme`. The first registry supports UNSCII 8/16 and Montserrat 8/10/12/14/16/18/20.

The matrix framebuffer is intentionally independent of the five-panel physical HUB75 topology. Panel order, scan pattern and multiplexing belong in the later hardware adapter.

Neither native library depends on Java, JavaFX, libuv, ESP-IDF or Arduino.

## Build and run

Requirements:

- Java 21;
- Maven (NetBeans' normal Maven integration is sufficient);
- CMake 3.20+;
- a C11 compiler.

Build and test the native code:

```powershell
cmake -S native -B native/build
cmake --build native/build --config Release
ctest --test-dir native/build -C Release --output-on-failure
```

Then open the repository root as a Maven project in NetBeans and use **Run Project**. The 90×35 window contains a **160×64 RGB matrix** button that opens the second native matrix simulator. From a terminal with Maven available:

```powershell
mvn javafx:run
```

`run.ps1` performs both steps.

The Java application looks for:

- Windows: `native/build/Release/display-host.exe`, then `native/build/display-host.exe`;
- Unix-like hosts: `native/build/display-host`.

Override with `-Ddisplay.native=<path>` if required.

## Serial protocol

Ordinary character bytes are rendered using the imported custom character map. The main readable commands are:

```text
/C?       clear screen and home cursor
/Lyyxx    set cursor, yy=00..34 and xx=00..89
/B0..9    draw one large 13x18 digit bitmap
```

The `?` in `/C?` is a required filler byte and is deliberately ignored. It preserves the legacy minimum slash-command length.

Two consecutive byte values `FE FE` place a sprite. The JavaFX **Sprite** button sends those bytes directly. The sprite animates automatically in place through the six imported appearances. `/NV1`..`/NV9` controls velocity. The native core starts at firmware-default v=4; the JavaFX velocity field initially shows 2 like the old control sheet and only changes the native value when **Set** is pressed. The desktop runtime uses the legacy-derived **10 ms general-timer tick**.

The JavaFX **Protocol** field accepts the same stream as the native parser. For example:

```text
/C?/L0510HELLO/B7
```

This clears the display, positions the cursor at `(Y,X)=(5,10)`, writes `HELLO`, and draws large digit 7.

For non-printable bytes, the Protocol field accepts `\xNN`. For example `\xFE\xFE` sends the sprite marker.

## Implemented slice

The C parser currently understands:

- custom character codes 31..255;
- proportional character advances from the original custom map;
- carriage return and backspace;
- `/C?` clear;
- `/Lyyxx` cursor positioning;
- `/B0`..`/B9` large digit bitmaps;
- `FE FE` sprite marker using the imported `SpriteMap1` appearances.

`CharMap1`, `SpriteMap1` and `Bitmap_0..Bitmap_9` were imported from the supplied legacy data. `Bitmap_Batavieren` and the other unrelated legacy bitmaps were deliberately excluded.

See [the experiment plan](docs/01-plan.md), [architecture](docs/02-design.md), [protocol](docs/03-protocol.md), [development setup](docs/04-development.md) and [matrix display design](docs/05-matrix-display.md).


# Third-party font attribution

The optional VBZ pixel font in `native/src/matrix_vbz_font.c` is adapted from
`vbzfont` in `sschueller/vbz-fahrgastinformation`.

Copyright (c) 2022 Stefan Schüller

The source project is licensed under the MIT License. The bitmap shapes and
glyph metrics are used under that license; the LVGL adapter and the derived
10-pixel nearest-neighbor variant are part of this project.

