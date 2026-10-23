package io.github.brainboxemb.experimental.display;

import java.io.ByteArrayOutputStream;

final class SerialBytes {

    private SerialBytes() {
    }

    static byte[] clear() {
        return new byte[]{0x7c, 0x00};
    }

    static byte[] home() {
        return new byte[]{0x7c, 0x18, 0x00, 0x7c, 0x19, 0x00};
    }

    static byte[] pixel(int x, int y, boolean on) {
        return new byte[]{0x7c, 0x10, (byte) x, (byte) y, (byte) (on ? 1 : 0)};
    }

    static byte[] sprite() {
        return new byte[]{(byte) 0xfe, (byte) 0xfe};
    }

    static byte[] parseProtocolText(String text) {
        ByteArrayOutputStream output = new ByteArrayOutputStream();

        for (int index = 0; index < text.length();) {
            char value = text.charAt(index);

            if (value != '\\') {
                if (value > 0x7f) {
                    throw new IllegalArgumentException(
                            "Protocol text must be ASCII at position " + index);
                }

                output.write(value);
                index++;
                continue;
            }

            if (index + 1 >= text.length()) {
                throw new IllegalArgumentException(
                        "Incomplete escape at end of protocol text");
            }

            char escape = text.charAt(index + 1);

            if (escape == '\\') {
                output.write('\\');
                index += 2;
                continue;
            }

            if (escape == 'r') {
                output.write('\r');
                index += 2;
                continue;
            }

            if (escape == 'b') {
                output.write('\b');
                index += 2;
                continue;
            }

            if (escape != 'x' && escape != 'X') {
                throw new IllegalArgumentException(
                        "Unsupported escape \\" + escape + " at position " + index);
            }

            if (index + 3 >= text.length()) {
                throw new IllegalArgumentException(
                        "Incomplete \\xNN escape at position " + index);
            }

            int high = Character.digit(text.charAt(index + 2), 16);
            int low = Character.digit(text.charAt(index + 3), 16);
            if (high < 0 || low < 0) {
                throw new IllegalArgumentException(
                        "Invalid \\xNN escape at position " + index);
            }

            output.write((high << 4) | low);
            index += 4;
        }

        return output.toByteArray();
    }

    static byte[] parseHex(String text) {
        String normalized = text.trim().replace(',', ' ');
        if (normalized.isEmpty()) {
            return new byte[0];
        }

        ByteArrayOutputStream output = new ByteArrayOutputStream();

        for (String part : normalized.split("\\s+")) {
            String token =
                    part.startsWith("0x") || part.startsWith("0X")
                            ? part.substring(2)
                            : part;

            int value = Integer.parseInt(token, 16);
            if (value < 0 || value > 255) {
                throw new IllegalArgumentException(
                        "Byte outside 00..FF: " + part);
            }

            output.write(value);
        }

        return output.toByteArray();
    }
}
