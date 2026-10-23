package io.github.brainboxemb.experimental.display;

record DisplayFrame(int width, int height, byte[] pixels) {

    boolean isOn(int x, int y) {
        return pixels[y * width + x] != 0;
    }
}
