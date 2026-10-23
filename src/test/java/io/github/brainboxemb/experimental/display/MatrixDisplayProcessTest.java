package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import org.junit.jupiter.api.Test;

final class MatrixDisplayProcessTest {

    @Test
    void receivesLvglDemoScreenAndAppliesTheme()
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

            assertEquals(132, frame.red(0, 11));
            assertEquals(130, frame.green(0, 11));
            assertEquals(132, frame.blue(0, 11));

            process.animationFrame(2);

            MatrixDisplayFrame stepped =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    stepped,
                    "matrix host did not render selected sprite frame");
            assertEquals(255, stepped.red(1, 22));
            assertEquals(0, stepped.green(1, 22));
            assertEquals(0, stepped.blue(1, 22));

            process.applyTheme(
                    new MatrixDisplayThemeConfig(
                            0x0000,
                            0x07e0,
                            0xffff,
                            0xffff,
                            0xffff,
                            0xffff,
                            0xf800,
                            0x07e0,
                            0x001f,
                            0xffff,
                            0xf800,
                            0x07e0,
                            0xffff,
                            0,
                            10,
                            10,
                            16,
                            10));

            MatrixDisplayFrame themed =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    themed,
                    "matrix host did not emit themed frame");
            assertEquals(0, themed.red(0, 11));
            assertEquals(255, themed.green(0, 11));
            assertEquals(0, themed.blue(0, 11));

            process.connectionStatus(
                    false,
                    false,
                    true);

            MatrixDisplayFrame statuses =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    statuses,
                    "matrix host did not render header statuses");
            assertEquals(0, statuses.red(125, 10));
            assertEquals(0, statuses.green(125, 10));
            assertEquals(255, statuses.blue(125, 10));
            assertEquals(0, statuses.red(97, 1));
            assertEquals(255, statuses.green(97, 1));
            assertEquals(0, statuses.blue(97, 1));
            assertEquals(255, statuses.red(125, 0));
            assertEquals(0, statuses.green(125, 0));
            assertEquals(0, statuses.blue(125, 0));
            assertEquals(0, statuses.red(149, 0));
            assertEquals(255, statuses.green(149, 0));
            assertEquals(0, statuses.blue(149, 0));
            assertEquals(255, statuses.red(151, 2));
            assertEquals(255, statuses.green(151, 2));
            assertEquals(255, statuses.blue(151, 2));

            process.clear();

            MatrixDisplayFrame cleared =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    cleared,
                    "matrix host did not emit cleared frame");
            assertEquals(0, cleared.red(0, 11));
            assertEquals(0, cleared.green(0, 11));
            assertEquals(0, cleared.blue(0, 11));

            process.requestDemoScreen();

            MatrixDisplayFrame restored =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    restored,
                    "matrix host did not restore demo screen");
            assertEquals(255, restored.green(90, 30));
        }
    }
}
