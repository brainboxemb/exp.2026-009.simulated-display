# Initial architecture and design decisions

## Ownership boundaries

- **Serial LCD parser (C):** consumes arbitrary byte chunks and maintains command state across calls.
- **LCD renderer (C):** owns the custom character map, sprite appearances, large digit bitmaps and text cursor.
- **Framebuffer (C):** owns logical monochrome pixels; the current 90×35 representation is bit-packed.
- **Host runtime (C):** adapts framed stdin requests to serial chunks and emits complete logical frames.
- **JavaFX 21:** starts the host process, supplies development controls and paints returned pixels. It owns no font/protocol behavior.
- **HUB75 interpreter (future C):** will consume synchronized application data through a separate model rather than pretending HUB75 uses the serial LCD protocol.
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

## Experimental invariants

1. Rendering a byte sequence does not depend on how it is divided into reads.
2. Font and graphic rendering are C responsibilities.
3. JavaFX receives logical pixels; it never reconstructs display commands.
4. The 90×35 LED simulation uses red pixels on black and deliberately follows the target display geometry.
5. HUB75 physical panel order belongs at the hardware/output boundary, not in the 160×64 logical application framebuffer.
6. No Arduino dependency in the application core.
7. Add libuv or LVGL only when a measured/representative requirement justifies them.

## Qualified vs open decisions

Chosen for the first slice:

- local process transport instead of TCP/libuv;
- full logical frames instead of dirty rectangles;
- C-owned legacy custom character map for codes 31..255 plus `SpriteMap1`;
- Java 21 / JavaFX 21 / Maven for NetBeans.

Still open:

- HUB75 pixel format (RGB565 is the first candidate);
- LVGL versus a small native renderer for the synchronized HUB75 application;
- ESP32 variant, HUB75 driver, scan pattern, brightness and memory strategy.
