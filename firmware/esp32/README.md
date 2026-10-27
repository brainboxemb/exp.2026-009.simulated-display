# ESP32 PlatformIO target — Experiment 009

This is an **additional firmware build target**, not a replacement for the JavaFX simulator. It compiles the real 160×64 RGB565 C/LVGL renderer and matrix serial interpreter from `native/`, then sends the resulting framebuffer through a narrow HUB75 hardware adapter.

## Board

The first target is a classic **ESP32-WROOM-32** (not an ESP32-S3):

| PlatformIO environment | Board |
| --- | --- |
| `esp32dev` (default) | `esp32dev` |
| `esp32doit-devkit-v1` | `esp32doit-devkit-v1` |

The PlatformIO Espressif platform is pinned to 8.0.0 (ESP-IDF 6.1.0).

```powershell
pio run -d firmware/esp32
pio run -d firmware/esp32 -e esp32doit-devkit-v1
```

A later target is ESP32-S3, including boards such as the Adafruit MatrixPortal S3. The shared renderer, serial interpreter and `MatrixFramebuffer` must not change for that move.

## Runtime path

```text
UART0 115200
    |
    v
MatrixSerialDisplay
    |
    | pages / text / icons / notifications / sprites
    v
160x64 RGB565 MatrixFramebuffer
    |
    v
hub75_output
    |
    v
esphome/esp-hub75 0.3.6
    |
    v
HUB75 DMA
```

The same `matrix_serial.c` used by the desktop simulator is compiled into the firmware. No second parser or renderer is maintained for ESP32.

At boot the firmware shows one static direct-rendered race demo so physical panel bring-up still has an immediate image. Serial state starts independently. A normal `/UB ... /UE` transaction builds the new serial frame off-screen and replaces the startup image only at `/UE`. Outside a transaction, accepted serial input is published immediately, matching the desktop host semantics.

The UART receive loop runs every 20 ms. That same cadence drives `/SV1..9` sprite animation, preserving the matrix serial timing used by the simulator.

## Serial input

The matrix protocol is received on **UART0 at 115200 baud**, the same port used by the normal PlatformIO serial monitor. CR/LF bytes are ignored by the matrix parser, so commands can be pasted from a terminal.

A minimal transaction is:

```text
/UB/PS0/PC/CF06/L00000/DF10015/FS09/CF00/CB16/L0000199/UE
```

The complete protocol is documented in `docs/06-matrix-serial-protocol.md`.

Serial notifications are global overlays:

```text
/N10  WiFi warning off
/N11  WiFi warning on
/N20  link/server warning off
/N21  link/server warning on
```

Sprites use `/S0`, `/S1` and `/SV1..9`.

## Brightness

The application framebuffer and driver input remain **RGB565**. `/XB00..99` does not scale RGB565 values on ESP32.

Instead, firmware maps the protocol value to the HUB75 driver's 8-bit OE brightness:

```text
XB00 ->   0 / 255
XB99 -> 255 / 255
```

Intermediate values are linearly rounded. Until a serial frame is presented, initial hardware bring-up keeps the conservative adapter default of 64/255.

The classic ESP32 uses **6 HUB75 BCM planes per color channel**. This is PWM/BCM depth, not an RGB666 framebuffer. Six planes preserve the full 5/6/5 source precision without paying the DMA cost of eight source-independent planes.

## HUB75 backend

The hardware adapter is a separate C++ ESP-IDF component behind a small C API. It uses `draw_pixels(... RGB565 ...)` for one bulk transfer instead of issuing 10,240 individual pixel calls.

The first hardware qualification deliberately keeps the previously working ESP32-WROOM wiring:

| Signal | ESP32 GPIO | Signal | ESP32 GPIO |
| --- | ---: | --- | ---: |
| R1 | 18 | G1 | 26 |
| B1 | 27 | R2 | 14 |
| G2 | 12 | B2 | 13 |
| A | 23 | B | 19 |
| C | 5 | D | 17 |
| CLK | 16 | LAT | 4 |
| OE | 15 | E | not driven; tied to GND |

GPIO12 (G2) is an ESP32 strapping pin. If it is externally held HIGH during reset it can select the wrong flash voltage on a normal 3.3V WROOM module. GPIO15 (OE) is also a strapping pin. Keep the known-good electrical arrangement for the first driver test.

## Panel geometry and chain

There are five electrical 64×32 panels. Every panel is physically mounted in portrait orientation, so it occupies 32×64 logical pixels. Together they form:

```text
[32x64][32x64][32x64][32x64][32x64] = 160x64
```

The adapter starts with the proven legacy-equivalent mapping:

- `layout_rows = 5`, `layout_cols = 1`;
- `Hub75PanelLayout::TOP_RIGHT_DOWN`;
- `Hub75Rotation::ROTATE_90`;
- standard two-scan wiring;
- generic shift driver;
- 10 MHz output clock;
- non-inverted clock phase;
- single HUB75 driver buffer.

This preserves the current physical chain entry at **back-left-bottom**. A future **back-right-top** entry remains represented as a hardware-only configuration choice.

## Verification boundary

CI proves that the shared matrix renderer, serial interpreter, LVGL and pinned HUB75 adapter compile together under ESP-IDF. It does **not** prove physical panel orientation, scan wiring, refresh behavior, low-brightness quality, DMA headroom, signal integrity or power behavior. Those remain hardware evidence for issue #3.
