# Experiment plan

Status: active. Implementation issue: [#1](https://github.com/brainboxemb/exp.2026-009.simulated-display/issues/1).

## Stage 1 — LCD vertical slice

Current implementation on PR #2:

- [x] 90×35 bit-packed monochrome C framebuffer;
- [x] native legacy custom character map (codes 31..255) with proportional advances;
- [x] six-frame legacy `SpriteMap1` support via `FE FE`;
- [x] chunk-independent serial command state machine;
- [x] readable `/C?`, `/Lyyxx` and `/B0..9` slash commands;
- [x] native host process with explicit framed stdin/stdout protocol;
- [x] Java 21 / JavaFX 21 Maven viewer with red LED rendering;
- [x] readable protocol text input plus raw `\\xNN` byte escapes when needed;
- [x] deterministic C unit tests and local host-protocol smoke test;
- [x] GitHub Linux/Windows CI qualification on exact head `58a624579c7c6649b9e45d74f478f95a22228338`;
- [ ] human NetBeans/JavaFX visual review on Windows.

## Stage 2 — LCD qualification

Verify split/combined input buffers, invalid slash commands, clipping, cursor wrapping, clear, bitmap placement, sprite markers and backspace. Compare behavior with the supplied legacy firmware/data and record deliberate 90×35 adaptations rather than hiding them.

## Stage 3 — HUB75 PoP

- [x] split native code into explicit `legacy-display` and `matrix-display` libraries;
- [x] add a 160×64 RGB565 logical framebuffer with C tests;
- [x] add a matrix-native host process and RGB JavaFX view;
- [x] integrate LVGL v9.6.0 for matrix text/layout rendering;
- [x] render the first 160×64 race screen with configurable colors and font family/size;
- [x] add a first matrix serial drawing/input phase with pages, palette-indexed drawing, fonts, icons, notifications and sprites;
- [x] add a portable `RELAY RACE` startup splash with compact software-version display;
- [x] select an Arduino-free ESP-IDF HUB75 backend for the PoP: `esphome/esp-hub75`;
- [x] model the proven five-panel baseline as 5×1 `TOP_RIGHT_DOWN` plus 90° rotation at the hardware-adapter boundary;
- [ ] design the synchronized application-data input model;
- [ ] implement the synchronized matrix-data interpreter.

The five-panel physical topology belongs at the hardware adapter boundary and is not encoded in the logical 160×64 framebuffer.

## Stage 4 — ESP32 qualification

- [x] add a standalone ESP-IDF PlatformIO firmware target for classic ESP32-WROOM;
- [x] compile the same shared C/LVGL renderer and matrix serial interpreter under ESP-IDF;
- [x] reuse the shared startup splash renderer as the ESP32 boot image;
- [x] add a narrow C/C++ HUB75 adapter using RGB565 bulk transfer;
- [x] receive the matrix serial protocol over UART0 at 115200 baud;
- [x] preserve `/UB`…`/UE` as the firmware-visible update boundary;
- [x] map `/XB00..99` to HUB75 driver/OE brightness without changing framebuffer pixels;
- [x] retain GPIO, chain-entry and panel mapping as hardware configuration outside the renderer;
- [x] add exact-head PlatformIO CI for the ESP32 target;
- [ ] flash the current WROOM hardware and verify driver initialization / available DMA memory;
- [ ] verify five-panel image orientation and chain mapping on hardware;
- [ ] verify scan wiring / shift-driver selection, refresh, brightness and low-brightness behavior;
- [ ] record power and signal-integrity observations;
- [ ] qualify the later ESP32-S3 target separately.

The WROOM baseline keeps the application/transfer framebuffer at RGB565 and uses six HUB75 BCM planes per channel with a single driver buffer. Physical-panel evidence remains separate from simulator/build evidence.

## Completion

Retain exact CI/test evidence and report the conclusion to the [meta experiment record](https://github.com/brainboxemb/brainboxemb.meta/blob/main/experiments/009-simulated-display/README.md). A successful experiment does not itself authorize production adoption.
