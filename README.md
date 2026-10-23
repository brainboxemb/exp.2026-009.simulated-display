# Experiment 009 — Simulated LED displays

Proof of Principle for running **native C display software** behind a **Java 21 / JavaFX 21** desktop simulator, with the portable C core intended to remain usable later on ESP32 / ESP-IDF.

Status: **active — first 90×35 LCD/LED vertical slice implemented on PR #2; qualification in progress**.

- [Cross-project record](https://github.com/brainboxemb/brainboxemb.meta/blob/main/experiments/009-simulated-display/README.md)
- [Meta tracking issue #188](https://github.com/brainboxemb/brainboxemb.meta/issues/188)
- [Implementation issue #1](https://github.com/brainboxemb/exp.2026-009.simulated-display/issues/1)

## Reference displays

| Mode | Pixels | Output | Interpretation |
| --- | --- | --- | --- |
| Serial LED | 90 × 35 | Red monochrome LEDs | SparkFun Graphic Serial LCD-style byte stream; C-owned custom character rendering |
| HUB75 | 160 × 64 (five panels) | RGB LEDs | Synchronized application data with a separate native C interpreter |

The SparkFun command set is a protocol reference. The simulated hardware is intentionally a red LED matrix rather than a physical SparkFun LCD.

## Current architecture

```text
serial bytes
    |
    v
native C streaming parser
    |
    v
custom character + sprite + graphics renderer
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

JavaFX does not interpret LCD commands or rasterize characters. It launches the native process, sends raw serial chunks and paints the pixels returned by C.

The portable C core has no Java, JavaFX, libuv, ESP-IDF or Arduino dependency.

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

Then open the repository root as a Maven project in NetBeans and use **Run Project**. From a terminal with Maven available:

```powershell
mvn javafx:run
```

`run.ps1` performs both steps.

The Java application looks for:

- Windows: `native/build/Release/display-host.exe`, then `native/build/display-host.exe`;
- Unix-like hosts: `native/build/display-host`.

Override with `-Ddisplay.native=<path>` if required.

On startup the simulator sends `HELLO WORLD` as ordinary ASCII bytes. The UI also accepts raw hexadecimal bytes, for example:

```text
7C 10 0A 05 01
```

which sends SparkFun-style **set pixel (10,5)**.

The JavaFX simulator also has a **Protocol** field. Ordinary ASCII is sent unchanged and `\\xNN` inserts one raw byte, so this can be entered directly:

```text
|\\x00|\\x1B\\x05\\x0AHELLO|\\x1A7
```

That clears the display, sets `(Y,X)=(5,10)` in one command, writes `HELLO`, and invokes the custom large-digit bitmap command for digit 7.

## Implemented slice

The C parser currently understands:

- legacy custom character codes 31..255, carriage return and backspace;
- proportional character advances from the original custom map;
- `FE FE` sprite marker using the six imported `SpriteMap1` appearances;
- custom SparkFun-framed `7C 1A <digit>` command for the imported 13×18 `Bitmap_0..Bitmap_9` digits;
- custom `7C 1B yy xx` command to set cursor Y and X atomically;
- clear;
- set X / set Y;
- set/reset pixel;
- line;
- circle;
- box;
- erase block.

Demo, reverse mode, splash, backlight and baud are recognized at the stream-framing level but are not simulated yet.

See [the experiment plan](docs/01-plan.md), [architecture](docs/02-design.md), [protocol slice](docs/03-protocol.md) and [development setup](docs/04-development.md).
