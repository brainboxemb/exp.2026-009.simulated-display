package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

final class SerialBytesTest {

    @Test
    void protocolTextKeepsReadableCommandsUnchanged() {
        assertArrayEquals(
                new byte[]{
                    '/', 'C', '?',
                    '/', 'L', '0', '5', '1', '0',
                    'H', 'E', 'L', 'L', 'O',
                    '/', 'B', '7'
                },
                SerialBytes.parseProtocolText(
                        "/C?/L0510HELLO/B7"));
    }

    @Test
    void clearUsesMinimumLengthLegacyForm() {
        assertArrayEquals(
                new byte[]{'/', 'C', '?'},
                SerialBytes.clear());
    }

    @Test
    void positionUsesReadableYyXxForm() {
        assertArrayEquals(
                new byte[]{
                    '/', 'L',
                    '0', '5',
                    '1', '0'
                },
                SerialBytes.position(5, 10));
    }

    @Test
    void positionRejectsCoordinatesOutsideDisplay() {
        assertThrows(
                IllegalArgumentException.class,
                () -> SerialBytes.position(35, 0));
        assertThrows(
                IllegalArgumentException.class,
                () -> SerialBytes.position(0, 90));
    }

    @Test
    void protocolTextSupportsRawSpriteBytes() {
        assertArrayEquals(
                new byte[]{
                    (byte) 0xfe,
                    (byte) 0xfe
                },
                SerialBytes.parseProtocolText(
                        "\\xFE\\xFE"));
    }

    @Test
    void protocolTextSupportsCarriageReturnAndBackspace() {
        assertArrayEquals(
                new byte[]{'A', '\r', '\b'},
                SerialBytes.parseProtocolText(
                        "A\\r\\b"));
    }

    @Test
    void protocolTextRejectsMalformedHexEscape() {
        assertThrows(
                IllegalArgumentException.class,
                () -> SerialBytes.parseProtocolText(
                        "\\xG1"));
    }

    @Test
    void protocolTextRejectsNonAsciiInput() {
        assertThrows(
                IllegalArgumentException.class,
                () -> SerialBytes.parseProtocolText(
                        "é"));
    }
}
