package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import org.junit.jupiter.api.Test;

final class MatrixDisplayProcessTest {

    @Test
    void supportsDirectDemoMode()
            throws Exception {
        var frames =
                new LinkedBlockingQueue<MatrixDisplayFrame>();
        var errors =
                new LinkedBlockingQueue<String>();

        try (var process =
                new MatrixDisplayProcess(
                        frames::add,
                        errors::add)) {
            MatrixDisplayFrame initial =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(initial);
            assertEquals(160, initial.width());
            assertEquals(64, initial.height());

            assertEquals(206, initial.red(24, 2));
            assertEquals(255, initial.green(24, 2));
            assertEquals(0, initial.blue(24, 2));

            process.startupSequence(0, 0);
            MatrixDisplayFrame boot =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);
            assertNotNull(boot);
            assertEquals(0, boot.red(137, 10));
            assertEquals(97, boot.green(137, 10));
            assertEquals(255, boot.blue(137, 10));

            process.startupSequence(1, 0);
            MatrixDisplayFrame scanning =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);
            assertNotNull(scanning);
            assertEquals(0, scanning.red(137, 10));
            assertEquals(97, scanning.green(137, 10));
            assertEquals(255, scanning.blue(137, 10));

            process.startupSequence(4, 0);
            MatrixDisplayFrame ready =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);
            assertNotNull(ready);
            assertArrayEquals(
                    initial.pixels(),
                    ready.pixels(),
                    "READY startup state must equal the static splash");

            process.requestDemoScreen();
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));

            process.layout(4);

            MatrixDisplayFrame fontTest =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(fontTest);

            process.applyTheme(
                    new MatrixDisplayThemeConfig(
                            0x0000,
                            0x07e0,
                            0xffff,
                            0xffff,
                            0xffff,
                            0xffff,
                            0xf800,
                            0x001f,
                            0xffff,
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

            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));

            process.layout(2);
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));

            process.connectionStatus(
                    false,
                    false,
                    true);
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));
        }
    }

    @Test
    void serialTransactionsPublishOnlyAtUpdateEnd()
            throws Exception {
        var frames =
                new LinkedBlockingQueue<MatrixDisplayFrame>();
        var errors =
                new LinkedBlockingQueue<String>();

        try (var process =
                new MatrixDisplayProcess(
                        frames::add,
                        errors::add)) {
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));

            process.serial(
                    SerialBytes.parseProtocolText(
                            "/UB/PS0/PC/CF06/L00000/DL00159"));

            assertNull(
                    frames.poll(
                            250,
                            TimeUnit.MILLISECONDS),
                    "transaction became visible before /UE");

            process.serial(
                    SerialBytes.parseProtocolText(
                            "/UE"));

            MatrixDisplayFrame committed =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(committed);
            assertEquals(0, committed.red(159, 0));
            assertEquals(255, committed.green(159, 0));
            assertEquals(0, committed.blue(159, 0));

            process.requestDemoScreen();

            MatrixDisplayFrame demo =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    demo,
                    "switching back to demo did not publish a frame");

            process.requestStartupScreen();
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS),
                    "switching to startup did not publish a frame");
        }
    }

    @Test
    void currentScreenProfileAndSpriteTickUseSerialMode()
            throws Exception {
        var frames =
                new LinkedBlockingQueue<MatrixDisplayFrame>();
        var errors =
                new LinkedBlockingQueue<String>();

        try (var process =
                new MatrixDisplayProcess(
                        frames::add,
                        errors::add)) {
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));

            process.requestDemoScreen();

            MatrixDisplayFrame demo =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);
            assertNotNull(demo);

            process.serial(
                    SerialBytes.parseProtocolText(
                            MatrixSerialExamples.currentRaceScreen()));

            MatrixDisplayFrame screen =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(screen);
            assertArrayEquals(
                    demo.pixels(),
                    screen.pixels(),
                    "Current race screen serial profile must match Demo pixel-for-pixel");

            process.serial(
                    SerialBytes.parseProtocolText(
                            "/SV1"));
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));

            process.serialTick();
            assertNotNull(
                    frames.poll(
                            5,
                            TimeUnit.SECONDS));
        }
    }
}
