package io.github.brainboxemb.experimental.display;

record MatrixDisplayThemeConfig(
        int backgroundColor,
        int lineColor,
        int headerColor,
        int rowColor,
        int largeColor,
        int footerColor,
        int fontFamily,
        int headerSize,
        int rowSize,
        int largeSize,
        int footerSize) {

    MatrixDisplayThemeConfig {
        requireUnsigned16(backgroundColor);
        requireUnsigned16(lineColor);
        requireUnsigned16(headerColor);
        requireUnsigned16(rowColor);
        requireUnsigned16(largeColor);
        requireUnsigned16(footerColor);

        if (fontFamily < 0 || fontFamily > 1) {
            throw new IllegalArgumentException(
                    "fontFamily must be 0 or 1");
        }

        requireUnsigned8(headerSize);
        requireUnsigned8(rowSize);
        requireUnsigned8(largeSize);
        requireUnsigned8(footerSize);
    }

    private static void requireUnsigned16(int value) {
        if (value < 0 || value > 0xffff) {
            throw new IllegalArgumentException(
                    "color must fit in RGB565");
        }
    }

    private static void requireUnsigned8(int value) {
        if (value < 0 || value > 0xff) {
            throw new IllegalArgumentException(
                    "font size must fit in one byte");
        }
    }
}
