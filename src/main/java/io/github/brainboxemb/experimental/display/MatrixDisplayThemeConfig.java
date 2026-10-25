package io.github.brainboxemb.experimental.display;

record MatrixDisplayThemeConfig(
        int backgroundColor,
        int lineColor,
        int headerColor,
        int rowColor,
        int largeColor,
        int footerColor,
        int spriteColor,
        int exchangePointBackgroundColor,
        int exchangePointTextColor,
        int flagColor,
        int warningBackgroundColor,
        int warningTextColor,
        int warningStrikeColor,
        int syncBackgroundColor,
        int syncIconColor,
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
        requireUnsigned16(spriteColor);
        requireUnsigned16(exchangePointBackgroundColor);
        requireUnsigned16(exchangePointTextColor);
        requireUnsigned16(flagColor);
        requireUnsigned16(warningBackgroundColor);
        requireUnsigned16(warningTextColor);
        requireUnsigned16(warningStrikeColor);
        requireUnsigned16(syncBackgroundColor);
        requireUnsigned16(syncIconColor);

        if (fontFamily < 0 || fontFamily > 1) {
            throw new IllegalArgumentException(
                    "fontFamily must be UNSCII or VBZ");
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
