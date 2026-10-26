# Initial architecture and design decisions

## Native library boundaries

The two displays are intentionally separate C libraries because they have different pixel models and different input semantics:

```text
legacy-display
  90x35
  bit-packed monochrome
  legacy serial/slash protocol
  custom characters / sprites / digit bitmaps

matrix-display
  160x64
  RGB565 logical framebuffer
  serial drawing interpreter (first input phase)
  synchronized application data interpreter (later phase)
  no physical HUB75 panel topology
```

The libraries should only share a third component when genuinely generic code appears. The framebuffer is deliberately **not** shared: forcing monochrome bit-packing and RGB565 into one abstraction would make both APIs less explicit.

## Ownership boundaries

- **legacy-display (C):** owns the 90×35 serial parser, custom character map, sprites, large digit bitmaps and bit-packed monochrome framebuffer.
- **matrix-display (C):** owns the 160×64 RGB565 logical framebuffer, the matrix serial drawing interpreter and later the synchronized-data interpreter.
- **Host runtime (C):** adapts desktop process framing to one display library and emits logical frames.
- **JavaFX 21:** starts native host processes, supplies development controls and paints returned logical pixels. It owns no display protocol or font behavior.
- **HUB75 hardware adapter (future):** maps the 160×64 logical framebuffer to five physical panels, scan order and multiplexing.
- **ESP32 / ESP-IDF (future):** platform adapter / hardware driver only. A narrow C bridge may hide a C++ HUB75 driver if needed.

## Process boundary

The initial simulator deliberately uses a child process instead of JNI/JNA:

```text
JavaFX
  | uint32 length + raw serial bytes
  v
display-host (C)
  | parser / renderer
  v
framebuffer
  | SDF1 header + full pixel frame
  v
JavaFX
```

This keeps the native code executable and testable by itself, avoids a Java/native ABI dependency, and makes the same parser usable with a later serial or embedded transport.

At 90×35, an unpacked output frame is only 3150 bytes. Full frames are therefore preferable to premature dirty-rectangle complexity in the first slice.

The 160×64 matrix PoP uses a second desktop host. It emits `SDF2` frames containing big-endian RGB565 pixels. The current Clear/Demo Screen requests are host-only qualification controls; they are deliberately not presented as the future synchronized application-data protocol.

LVGL v9.6.0 is internal to `matrix-display`. Public screen data and theme structures do not expose LVGL types. The first screen uses a semantic `MatrixTheme`: background and line colors are independent, while header, row-number, row-time, large-value and footer text styles each select their own color, font family and font size.

## Experimental invariants

1. Rendering a byte sequence does not depend on how it is divided into reads.
2. Font and graphic rendering are C responsibilities.
3. JavaFX receives logical pixels; it never reconstructs display commands.
4. The 90×35 LED simulation uses red pixels on black and deliberately follows the target display geometry.
5. HUB75 physical panel order belongs at the hardware/output boundary, not in the 160×64 logical application framebuffer.
6. No Arduino dependency in the application core.
7. LVGL is used only by the 160×64 matrix renderer; the legacy 90×35 path remains independent.

## Qualified vs open decisions

Chosen for the first slice:

- local process transport instead of TCP/libuv;
- full logical frames instead of dirty rectangles;
- C-owned legacy custom character map for codes 31..255 plus `SpriteMap1`;
- Java 21 / JavaFX 21 / Maven for NetBeans.

Chosen for the matrix foundation:

- logical geometry is 160×64;
- framebuffer format is RGB565 (`uint16_t`, 20,480 bytes);
- physical five-panel layout is outside the framebuffer API.

Still open:

- synchronized application-data message model;
- ESP32 variant, HUB75 driver, scan pattern, brightness and memory strategy.
