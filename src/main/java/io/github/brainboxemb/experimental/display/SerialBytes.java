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

    static byte[] parseHex(String text) {
        String normalized = text.trim().replace(',', ' ');
        if (normalized.isEmpty()) {
            return new byte[0];
        }
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        for (String part : normalized.split("\\s+")) {
            String token = part.startsWith("0x") || part.startsWith("0X") ? part.substring(2) : part;
            int value = Integer.parseInt(token, 16);
            if (value < 0 || value > 255) {
                throw new IllegalArgumentException("Byte outside 00..FF: " + part);
            }
            output.write(value);
        }
        return output.toByteArray();
    }
}
