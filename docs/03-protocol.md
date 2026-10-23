# Serial LCD protocol slice

The experiment uses the SparkFun **Graphic Serial LCD Backpack** framing as the protocol reference: ordinary printable ASCII is rendered as text, while `0x7C` (`|`) introduces a command. The original firmware also recognizes carriage return and backspace as text controls.

References:

- <https://learn.sparkfun.com/tutorials/serial-graphic-lcd-hookup/ascii-commands>
- <https://github.com/sparkfun/GraphicLCD_Serial_Backpack/tree/master/Firmware/AVR_Firmware>

## Implemented commands

| Bytes after `0x7C` | Parameters | Experiment behavior |
| --- | ---: | --- |
| `00` | 0 | Clear framebuffer and reset cursor to `(0,0)` |
| `18` | 1 | Set cursor X (`0..89`) |
| `19` | 1 | Set cursor Y (`0..34`) |
| `1A` | 1 | **Experiment extension:** draw large 13×18 digit bitmap; parameter is ASCII `0`..`9` |
| `1B` | 2 | **Experiment extension:** set cursor position as `yy xx` (`y=0..34`, `x=0..89`) |
| `10` | 3 | Set/reset pixel: `x y value` |
| `0C` | 5 | Draw/erase line: `x0 y0 x1 y1 value` |
| `03` | 4 | Draw/erase circle: `x y radius value` |
| `0F` | 5 | Draw/erase box: `x0 y0 x1 y1 value` |
| `05` | 4 | Erase block: `x0 y0 x1 y1` |

Known but deliberately not simulated yet: demo (`04`), reverse (`12`), splash (`13`), backlight (`02 value`) and baud (`07 value`). Parameter bytes are still consumed so subsequent stream parsing remains aligned.

## Custom character map

The simulator uses the legacy `CharMap1` supplied for this experiment rather than the temporary 5×7 font.

- character codes **31..255** are retained;
- each entry stores its original proportional cursor advance plus eight bitmap columns;
- bit 0 is the top pixel and all eight row bits are rendered;
- carriage return advances to the next 8-pixel line;
- backspace returns to the previous proportional character position;
- `0x7C` remains the serial command prefix and therefore cannot be emitted as an ordinary character byte in this protocol.

`CharMap1`, `SpriteMap1` and `Bitmap_0..Bitmap_9` were converted from the supplied legacy include file. `Bitmap_Batavieren` and the other legacy `Bitmap_*` tables remain deliberately excluded.

## Sprite marker

Two consecutive bytes `FE FE` render a sprite at the current text cursor. The six imported `SpriteMap1` appearances are used in sequence and wrap after frame six. The sprite advances the cursor by the original `wSprite1 = 7` pixels.

The marker detection is streaming: `FE FE` still works when the two bytes arrive in separate host chunks. A single `FE` followed by another byte falls back to legacy character 254 before the following byte is processed.

Stepping to the next sprite appearance on every `FE FE` is the deterministic simulator behavior for this PoP.

## Combined cursor position command

The original SparkFun X and Y commands remain available for compatibility, but the experiment adds one preferred command for setting the complete cursor position atomically:

```text
7C 1B yy xx
```

The parameter order deliberately follows the legacy display convention: **Y first, then X**. For example, `7C 1B 05 0A` sets the cursor to `(Y,X)=(5,10)`.

Both coordinates are validated together. If either value is outside the 90×35 display, the cursor remains unchanged.

## Large digit bitmap command

The original large numeric bitmaps are exposed through an experiment-specific command while retaining SparkFun framing:

```text
7C 1A <ASCII digit>
```

For example, `7C 1A 37` draws the old 13×18 bitmap for digit `7` at the current cursor position. The command clips naturally at the framebuffer edge and does not move the cursor.

## JavaFX protocol text input

The JavaFX **Protocol** field is a convenience editor for the raw serial stream. Ordinary ASCII is emitted unchanged and `\\xNN` inserts one byte. This makes control-byte commands readable without inventing a second display protocol.

Example:

```text
|\\x00|\\x1B\\x05\\x0AHELLO|\\x1A7
```

Here `|` is the literal SparkFun command prefix `0x7C`; only the non-printable command/value bytes need escaping.

## 90×35 geometry

The reference SparkFun firmware targets 128×64 and 160×128 displays. This experiment intentionally targets **90×35**. Text wrapping follows each custom character's stored advance rather than assuming fixed 6-pixel cells.

## Host framing

Java to native:

```text
uint32 big-endian byte_count
byte[byte_count] serial_chunk
```

Native to Java:

```text
'S' 'D' 'F' '1'
uint16 big-endian width
uint16 big-endian height
uint32 big-endian payload_size
byte[payload_size] pixels       # one byte per logical pixel, 0 or 1
```

A serial command may be split across multiple host requests; that chunk independence is tested in C. Full uncompressed frames are intentional for the first 90×35 slice.
