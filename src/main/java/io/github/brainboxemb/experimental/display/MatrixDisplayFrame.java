package io.github.brainboxemb.experimental.display;

record MatrixDisplayFrame(
        int width,
        int height,
        short[] pixels) {

    int red(int x, int y) {
        int pixel = pixel(x, y);
        int red5 = (pixel >> 11) & 0x1f;
        return (red5 * 255 + 15) / 31;
    }

    int green(int x, int y) {
        int pixel = pixel(x, y);
        int green6 = (pixel >> 5) & 0x3f;
        return (green6 * 255 + 31) / 63;
    }

    int blue(int x, int y) {
        int pixel = pixel(x, y);
        int blue5 = pixel & 0x1f;
        return (blue5 * 255 + 15) / 31;
    }

    private int pixel(int x, int y) {
        return Short.toUnsignedInt(
                pixels[y * width + x]);
    }
}
