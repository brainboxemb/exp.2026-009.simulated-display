package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import org.junit.jupiter.api.Test;

final class MatrixDisplayProcessTest {

    @Test
    void receivesLvglDemoScreenFromNativeHost()
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

            assertEquals(255, frame.red(0, 10));
            assertEquals(0, frame.green(0, 10));
            assertEquals(0, frame.blue(0, 10));

            process.clear();

            MatrixDisplayFrame cleared =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    cleared,
                    "matrix host did not emit cleared frame");
            assertEquals(0, cleared.red(0, 10));
            assertEquals(0, cleared.green(0, 10));
            assertEquals(0, cleared.blue(0, 10));

            process.requestDemoScreen();

            MatrixDisplayFrame restored =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    restored,
                    "matrix host did not restore demo screen");
            assertEquals(255, restored.red(90, 30));
        }
    }
}
