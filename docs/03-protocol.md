# Serial LCD protocol slice

The experiment uses the SparkFun **Graphic Serial LCD Backpack** framing as the protocol reference: ordinary printable ASCII is rendered as text, while `0x7C` (`|`) introduces a command. The original firmware also recognizes carriage return and backspace as text controls.

References:

- <https://learn.sparkfun.com/tutorials/serial-graphic-lcd-hookup/ascii-commands>
- <https://github.com/sparkfun/GraphicLCD_Serial_Backpack/tree/master/Firmware/AVR_Firmware>

## Implemented commands

| Bytes after `0x7C` | Parameters | Experiment behavior |
| --- | ---: | --- |
| `00` | 0 | Clear framebuffer and reset cursor to text origin |
| `18` | 1 | Set text X origin and cursor |
| `19` | 1 | Set text Y origin and cursor |
| `10` | 3 | Set/reset pixel: `x y value` |
| `0C` | 5 | Draw/erase line: `x0 y0 x1 y1 value` |
| `03` | 4 | Draw/erase circle: `x y radius value` |
| `0F` | 5 | Draw/erase box: `x0 y0 x1 y1 value` |
| `05` | 4 | Erase block: `x0 y0 x1 y1` |

Known but deliberately not simulated yet: demo (`04`), reverse (`12`), splash (`13`), backlight (`02 value`) and baud (`07 value`). Parameter bytes are still consumed so subsequent stream parsing remains aligned.

## Text

Printable ASCII `0x20..0x7E` uses a native C 5×7 bitmap font in a 6×8 character cell. `\r` advances to the next 8-pixel line and `\b` erases the previous character cell.

The glyph artwork is experiment-owned and is not claimed to be byte-identical to SparkFun's font. Command framing and relevant behavior are the compatibility target.

## 90×35 geometry

The reference SparkFun firmware targets 128×64 and 160×128 displays. This experiment intentionally targets **90×35**. Fifteen 6-pixel cells fit across a 90-pixel row, so X=84 is a valid final character position. This is a deliberate geometry adaptation.

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
