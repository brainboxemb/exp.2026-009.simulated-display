package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import org.junit.jupiter.api.Test;

final class MatrixDisplayProcessTest {

    @Test
    void receivesRgb565TestPatternFromNativeHost()
            throws Exception {
        var frames =
                new LinkedBlockingQueue<MatrixDisplayFrame>();
        var errors =
                new LinkedBlockingQueue<String>();

        try (var process =
                new MatrixDisplayProcess(
                        frames::add,
                        errors::add)) {
            MatrixDisplayFrame frame =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    frame,
                    "matrix host did not emit initial frame");
            assertEquals(160, frame.width());
            assertEquals(64, frame.height());

            assertEquals(255, frame.red(10, 10));
            assertEquals(0, frame.green(10, 10));
            assertEquals(0, frame.blue(10, 10));

            assertEquals(0, frame.red(30, 10));
            assertEquals(255, frame.green(30, 10));
            assertEquals(0, frame.blue(30, 10));

            process.clear();

            MatrixDisplayFrame cleared =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    cleared,
                    "matrix host did not emit cleared frame");
            assertEquals(0, cleared.red(10, 10));
            assertEquals(0, cleared.green(10, 10));
            assertEquals(0, cleared.blue(10, 10));
        }
    }
}
