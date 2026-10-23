package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.Test;

final class MatrixDisplayFrameTest {

    @Test
    void convertsRgb565PrimaryColorsToRgb888() {
        MatrixDisplayFrame frame =
                new MatrixDisplayFrame(
                        3,
                        1,
                        new short[]{
                            (short) 0xf800,
                            (short) 0x07e0,
                            (short) 0x001f
                        });

        assertEquals(255, frame.red(0, 0));
        assertEquals(0, frame.green(0, 0));
        assertEquals(0, frame.blue(0, 0));

        assertEquals(0, frame.red(1, 0));
        assertEquals(255, frame.green(1, 0));
        assertEquals(0, frame.blue(1, 0));

        assertEquals(0, frame.red(2, 0));
        assertEquals(0, frame.green(2, 0));
        assertEquals(255, frame.blue(2, 0));
    }
}
