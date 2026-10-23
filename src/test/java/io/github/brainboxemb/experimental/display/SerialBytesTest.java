package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

final class SerialBytesTest {

    @Test
    void protocolTextKeepsAsciiAndExpandsHexEscapes() {
        byte[] bytes = SerialBytes.parseProtocolText(
                "|\\x00|\\x18\\x0A|\\x19\\x05HELLO|\\x1A7");

        assertArrayEquals(
                new byte[]{
                    0x7c, 0x00,
                    0x7c, 0x18, 0x0a,
                    0x7c, 0x19, 0x05,
                    'H', 'E', 'L', 'L', 'O',
                    0x7c, 0x1a, '7'
                },
                bytes);
    }

    @Test
    void protocolTextSupportsBackslashCarriageReturnAndBackspace() {
        assertArrayEquals(
                new byte[]{'A', '\\', '\r', '\b'},
                SerialBytes.parseProtocolText("A\\\\\\r\\b"));
    }

    @Test
    void protocolTextRejectsMalformedHexEscape() {
        assertThrows(
                IllegalArgumentException.class,
                () -> SerialBytes.parseProtocolText("\\xG1"));
    }

    @Test
    void protocolTextRejectsNonAsciiInput() {
        assertThrows(
                IllegalArgumentException.class,
                () -> SerialBytes.parseProtocolText("é"));
    }
}
