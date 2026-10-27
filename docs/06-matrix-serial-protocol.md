# 160×64 matrix serial protocol

The RGB matrix has a readable slash-command input as a first control phase before the synchronized race-data interpreter. It is implemented by `matrix-display`; JavaFX forwards bytes and renders returned RGB565 frames.

The parser is streaming: commands can be split across arbitrary input chunks.

## Drawing state

The interpreter retains the selected page, cursor position, font size, foreground palette index, text-background mode/color, global brightness, built-in icon color mappings, sprite placements, sprite velocity and the two serial notification states. Text background is transparent by default; `/CB00`..`/CB15` explicitly make character cells opaque and `/CB16` restores transparency.

Three transparent pages are composited in order 0 → 1 → 2. A page stores palette indexes rather than RGB565 pixels; `255` is internal transparency. Black therefore remains a normal visible palette color.

## Commands

| Command | Meaning |
| --- | --- |
| `/UB` | Begin atomic display update; subsequent changes remain off-screen |
| `/UE` | End atomic display update and publish the composed frame |
| `/PS0`..`/PS2` | Select page 0..2 |
| `/PC` | Clear selected page, its sprites and home the cursor |
| `/Lyyxxx` | Move cursor, Y=00..63 and X=000..159 |
| `/DLyyxxx` | Draw foreground-colored line from cursor to target; target becomes cursor |
| `/DRyyxxx` | Draw rectangle outline using cursor and target as opposite corners |
| `/DFyyxxx` | Draw filled rectangle using cursor and target as opposite corners |
| `/FSnn` | Select supported font size: 08, 09, 10, 12, 14 or 16 |
| `/CFnn` | Foreground palette index 00..15 |
| `/CB00`..`/CB15` | Opaque text background palette index |
| `/CB16` | Transparent text background (default) |
| `/XBnn` | Display brightness 00..99 |
| `/BIn` | Draw built-in icon n at the current cursor on the selected page |
| `/CInCnn...` | Configure persistent icon palette indexes |
| `/S0` | Place runner sprite at cursor |
| `/S1` | Place warmup sprite at cursor |
| `/SVv` | Set global sprite velocity v=1..9 |
| `/N10`, `/N11` | WiFi notification off/on |
| `/N20`, `/N21` | link/server notification off/on |

Ordinary printable ASCII is drawn with the selected font. The slash byte starts a command.

## Atomic updates

Outside an update transaction, accepted serial input is composed and published immediately. Between `/UB` and `/UE`, page, text, icon, sprite and style changes only modify the internal drawing state. The previously published frame remains visible. `/UE` composes pages 0 → 1 → 2 and publishes one complete frame.

Transactions are not nestable. A second `/UB` while an update is active, or `/UE` without an active update, is invalid. This is an atomic visibility boundary, not a rollback mechanism.

On the ESP32 the same boundary can be used to trigger one framebuffer transfer after `/UE`; exact tear-free HUB75 frame switching remains a driver/double-buffering responsibility.

The matrix uses a three-digit X field because its width is 160 pixels; the legacy 90×35 `/Lyyxx` form cannot address the rightmost 60 columns.

## Built-in icons

`/BIn` is a drawing primitive: it writes the selected built-in icon into the current page at the current cursor. It is not notification state. Clearing that page therefore removes the drawn icon like other page content.

The initial built-in set is:

| n | Icon | Color roles |
| ---: | --- | --- |
| 0 | finish flag | background/checker, foreground/checker |
| 1 | WiFi warning | background, glyph, strike |
| 2 | registration-server/link warning | background, glyph, strike |
| 3 | sync | background, glyph |

Icon colors are persistent palette references. The syntax uses one `Cnn` group per role:

```text
/CI0C03C01
/CI1C10C01C05
/CI2C10C01C05
/CI3C06C00
```

`/BI3` then draws the sync icon using the stored colors. Changing `/CFnn` does not change an icon's configured colors.

## Notifications

Serial notifications are global overlays composed after pages 0 → 1 → 2. They are intentionally not attached to a page and do not move or restore underlying text.

Only two notifications are currently defined:

| n | State | Meaning |
| ---: | --- | --- |
| 1 | `/N10` off, `/N11` on | WiFi warning |
| 2 | `/N20` off, `/N21` on | local registration-server/link warning |

Active notifications are packed against the right edge with the same 11×11 icon artwork and one-pixel gap used by the Demo renderer. With both active, WiFi is left of link. With one active, that notification occupies the rightmost 11 pixels.

They deliberately overwrite whatever composed content is under them. Turning a notification off simply omits the overlay on the next frame, so the original page content becomes visible again without any repair drawing.

Synchronization is not a serial notification at this stage. Icon 3 remains available as a normal built-in icon through `/BI3`, but there is no `/N3x` command.

## Sprites

Sprites are separate from built-in icons because they have animation state.

`/S0` and `/S1` place a sprite on the selected page at the current cursor. The current `/CFnn` palette index is stored with the sprite. Later foreground changes therefore do not recolor an existing sprite, while a future change to the referenced palette entry can.

`/SV1`..`/SV9` controls the shared animation speed. The simulator ticks every 20 ms, so the resulting frame intervals are 20..180 ms, matching the legacy `2*v` × 10 ms timing.

## Default palette

The first 16-entry palette contains black, soft white, white, dark gray, gray, red, lime, green, yellow, orange, the current warning blue, cyan, magenta, warm yellow, purple and blue. Drawing state and icon/sprite state store indexes, not copied RGB values.

Logical black means an unlit LED. The JavaFX viewer may show those off pixels as dark gray for readability, but that simulator appearance is not framebuffer content.

## Simulator presets

The JavaFX matrix window has two primary tabs: **Demo** and **Serial**. Viewer controls for **2×** scale and the **Off pixel** display color are shared by both modes.

**Demo** uses the direct prebuilt race-screen renderer with static test data. It exposes the existing layout variants, connection-status toggles, sprite stepping/animation and the color/font controls.

**Serial** uses the matrix serial interpreter. It exposes editable command profiles and sends those bytes through the same serial parser used by the protocol tests. Switching tabs also switches the active native display mode; the two input paths are not mixed on screen.

The available presets are **Current race screen**, **Pages + lines**, **Fonts + colors**, **Built-in icons**, **Sprites + speed**, **Notifications** and **Brightness**. Every preset begins with `/UB/PS0/PC/PS1/PC/PS2/PC/XB99/CB16/N10/N20` and ends with `/UE`, so it starts from three known empty pages, restores normal brightness and transparent text, and becomes visible atomically. The brightness preset then deliberately changes `/XB` to its test value.

The current-screen preset reconstructs the normal mixed race layout through this serial protocol rather than calling the direct demo renderer:

- page 0 contains the structural divider lines;
- page 1 contains the filled exchange badge and transparent text;
- page 2 contains the finish flag and animated runner/warmup sprites.

The exchange badge uses one filled rectangle command (`/L00000/DF10015`) rather than eleven horizontal lines.

The Current race screen profile is verified against the direct Demo renderer pixel-for-pixel.

All unused pixels remain logical black/off. No dark-gray background is transmitted.

A serial command for redefining palette entries is intentionally not assigned yet; its syntax remains a protocol-design decision rather than being invented by the simulator.
