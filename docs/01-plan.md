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

Implement a 160×64 RGB logical framebuffer and a five-panel JavaFX view. Design synchronized data input; evaluate a small C renderer against LVGL on a representative layout. Investigate an Arduino-free ESP-IDF HUB75 driver and panel mapping.

## Stage 4 — ESP32 qualification

Compile shared C sources under ESP-IDF and validate the driver/output boundary on actual hardware when available. Keep physical-panel evidence separate from simulator evidence.

## Completion

Retain exact CI/test evidence and report the conclusion to the [meta experiment record](https://github.com/brainboxemb/brainboxemb.meta/blob/main/experiments/009-simulated-display/README.md). A successful experiment does not itself authorize production adoption.
