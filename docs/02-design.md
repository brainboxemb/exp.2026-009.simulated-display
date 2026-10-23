# Initial architecture and design decisions

## Ownership boundaries

- **LCD parser (C):** consumes arbitrary stream chunks, recognizes ASCII text and validated SparkFun-style commands. No Java or transport dependencies.
- **LCD renderer (C):** converts 5×7 bitmap font and graphic primitives into monochrome framebuffer changes. Character cell is 6×8 pixels, with spacing.
- **HUB75 interpreter (C):** will consume synchronized application data in a separate domain-specific parser/model; no assumption that HUB75 uses SparkFun commands.
- **Framebuffer (C):** logical pixels, width/height and pixel format. Red monochrome may be bit-packed; RGB565 is a HUB75 candidate.
- **Output port:** reports complete or changed pixel regions, independent of output hardware.
- **Host runtime:** receives input, owns the native core and transfers pixel updates to a JavaFX client. The initial framed transport is intentionally small; libuv is not needed by default.
- **JavaFX 21:** paints output pixels, user controls and diagnostics; does not render fonts or interpret SparkFun bytes.
- **ESP32 / ESP-IDF:** platform-specific driver and transport integration. A narrow C adapter may hide a C++ HUB75 driver.

## Experimental invariants

1. Rendering a given byte sequence must not depend on how input arrives split into reads.
2. Identical native C test cases work independently of JavaFX and hardware.
3. The 90×35 LED simulation uses red pixels on black; the SparkFun protocol is adopted only as a documented command/behaviour reference.
4. HUB75 panel mapping is an output-adapter concern, distinct from the 160×64 logical framebuffer.
5. No Arduino dependency in the application core.
6. Font tables and cursor/raster semantics are validated with example bitmaps and protocol tests rather than inferred from UI appearance.

## Open decisions to qualify

- Exact SparkFun firmware revision / command compatibility subset, especially set-position and drawing opcodes.
- Pixel synchronization framing: full frames initially, dirty rectangles when justified by measurements.
- Host transport choice and lifecycle under NetBeans.
- LVGL value compared with a small renderer for the eventual HUB75 data model.
- ESP32 variant, HUB75 scan pattern, brightness, driver and memory allocation.

These are tracked experiment questions, not prematurely frozen production interfaces.
