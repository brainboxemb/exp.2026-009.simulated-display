# Experiment plan

Status: active. Implementation issue: [#1](https://github.com/brainboxemb/exp.2026-009.simulated-display/issues/1).

## Stage 1 — LCD vertical slice

1. Fix the LCD byte parser's first protocol subset from the SparkFun reference, including byte framing and cursor semantics.
2. Implement 90×35 monochrome C framebuffer, 5×7 glyphs with 6×8 cells, and deterministic tests.
3. Build a minimal native host process and frame transport.
4. Build a Java 21/JavaFX 21 Maven application; make NetBeans Run Project work with an explicit native build/bootstrap step.
5. Show HELLO WORLD and raw drawing primitives sent through serial bytes, with CI/CTest and manual UI verification evidence.

## Stage 2 — LCD qualification

Verify split/combined input buffers, invalid command boundaries, clipping, cursor wrapping, clear and drawing primitives. Confirm exact SparkFun command bytes before claiming protocol compatibility.

## Stage 3 — HUB75 PoP

Implement a 160×64 RGB logical framebuffer and a five-panel JavaFX view. Design synchronized data input; evaluate plain-C rendering against LVGL based on a representative example. Investigate an Arduino-free ESP-IDF driver and panel layout.

## Stage 4 — ESP32 qualification

Compile the shared C sources in ESP-IDF and validate driver updates on available hardware. Distinguish simulation evidence from real LED-panel evidence.

## Completion

Keep test fixtures and exact CI evidence. Report design conclusions to [the meta experiment record](https://github.com/brainboxemb/brainboxemb.meta/blob/main/experiments/009-simulated-display/README.md); do not imply production adoption.
