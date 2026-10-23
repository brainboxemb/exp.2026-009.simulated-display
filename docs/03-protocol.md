# Serial display protocol

The 90×35 simulator uses one small streaming protocol based on the supplied legacy display behavior. There is no separate SparkFun compatibility layer.

Ordinary character bytes are rendered through the C-owned custom character map. Slash-prefixed ASCII commands provide display control.

## Commands

| Command | Meaning |
| --- | --- |
| `/C?` | Clear framebuffer and reset cursor to `(0,0)`; the third byte is required and ignored |
| `/Lyyxx` | Set cursor position with decimal `yy=00..34` and `xx=00..89` |
| `/B0`..`/B9` | Draw one imported 13×18 digit bitmap at the current cursor position |
| `/NVv` | Set sprite velocity, `v=1..9`; interval becomes `2*v` general-timer ticks |
| `FE FE` | Place a sprite marker using the imported `SpriteMap1` data |
| `CR` | Move to the next 8-pixel text row |
| `BS` | Erase the previous rendered character cell and restore its cursor position |

The parser is streaming. Commands may be split across arbitrary input chunks.

## Clear

Clear has the legacy minimum-length form:

```text
/C?
```

The byte after `C` is consumed but ignored. Therefore `/CX`, `/C0` and `/C?` all have identical behavior. The documented form is `/C?`.

## Cursor position

```text
/Lyyxx
```

Coordinates are decimal ASCII and use **Y first, then X**.

Examples:

```text
/L0000
/L0510
/L3489
```

All four characters must be decimal digits. Both coordinates are validated as one command. Invalid input leaves the cursor unchanged.

## Command-letter note

In the legacy dispatcher, **`B` selects predefined bitmap** and **`G` selects graph burst**. The old `Instruction.asm` contains a stale comment inside `GRAPHBURST` that says `/Bxxbbb`; that comment does not reflect the dispatcher mapping.

Graph burst is not implemented in this PoP yet.

## Large digit bitmap

```text
/Bn
```

where `n` is `0` through `9`.

The command renders the corresponding imported 13×18 `Bitmap_n` at the current cursor position. Rendering clips at framebuffer edges and does not move the cursor.

## Character map

The simulator uses the supplied legacy `CharMap1`:

- character codes 31..255 are retained;
- each entry has its original proportional cursor advance;
- each entry contains eight vertical bitmap columns;
- bit 0 is the top pixel;
- all eight row bits are rendered.

`/` is reserved as the command prefix and therefore is not rendered as an ordinary character byte.

## Sprite

The supplied legacy sprite code documents that a sprite is placed by sending character 254 twice consecutively. fileciteturn323file0L12-L13

The current simulator recognizes `FE FE` across input chunk boundaries and places one sprite at the current cursor position. The visible sprite is seven pixels wide; character 254 advances the cursor by eight pixels so one blank separator column remains.

The legacy display stores sprite metadata in bit 7. That bit is **not a visible eighth pixel row**. The simulator therefore renders only bits 0..6.

After placement, the C core updates every existing sprite in place and advances the shared six-frame `SpriteMap1` sequence.

The legacy `/NVv` command is implemented for `v=1..9`. As in the supplied firmware, the stored interval is `2*v` general-timer ticks. The firmware initialization sets `4<<1`, so the native display core starts at **v=4**.

The old desktop control sheet starts its editable **Sprite velocity** field at **2**. JavaFX mirrors that visible start value, but it does not send `/NV2` automatically; the firmware-default v=4 remains active until **Set** is pressed.

The supplied Timer1 initialization gives a Timer1 clock of 921.6 kHz in normal mode with prescaler 8. If the general timer advances once per 16-bit Timer1 overflow, one base tick is `65536 / 921600 = 71.111... ms`. The desktop runtime currently uses that **71.111 ms** base. This Timer1-overflow mapping should still be verified against the original ISR when that source is available.

## JavaFX protocol input

The JavaFX **Protocol** field sends its readable ASCII directly to the C parser:

```text
/C?/L0510HELLO/B7
```

The field supports `\xNN` only for bytes that cannot be typed directly, for example:

```text
\xFE\xFE
```

JavaFX does not interpret the display commands; it only converts optional `\xNN` escapes to bytes and forwards the stream.

## Host framing

Java to native:

```text
uint32 big-endian byte_count
byte[byte_count] serial_chunk

byte_count = FFFFFFFF means: host-runtime general-timer tick (no serial payload)
```

Native to Java:

```text
'S' 'D' 'F' '1'
uint16 big-endian width
uint16 big-endian height
uint32 big-endian payload_size
byte[payload_size] pixels
```

The full uncompressed frame is intentional for this first 90×35 slice.
