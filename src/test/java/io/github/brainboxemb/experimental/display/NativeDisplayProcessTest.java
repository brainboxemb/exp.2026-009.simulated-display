package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import org.junit.jupiter.api.Test;

final class NativeDisplayProcessTest {

    @Test
    void exchangesBinarySerialBytesWithNativeHost() throws Exception {
        var frames =
                new LinkedBlockingQueue<DisplayFrame>();
        var errors =
                new LinkedBlockingQueue<String>();

        try (var process =
                new NativeDisplayProcess(
                        frames::add,
                        errors::add)) {
            DisplayFrame initial =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    initial,
                    "native host did not emit its initial frame");
            assertEquals(90, initial.width());
            assertEquals(35, initial.height());

            /*
             * 0x1A is intentionally placed before a valid readable command.
             * In Windows text mode 0x1A may be treated as EOF. The native
             * host must therefore keep stdin binary so the following bytes
             * still reach the parser.
             */
            process.send(new byte[]{
                0x1a,
                '/',
                'L',
                '0',
                '5',
                '1',
                '0',
                'A'
            });

            DisplayFrame updated =
                    frames.poll(
                            5,
                            TimeUnit.SECONDS);

            assertNotNull(
                    updated,
                    "native host did not emit the changed frame");
            assertTrue(
                    updated.isOn(10, 7),
                    "bytes after 0x1A must survive the Windows process pipe");
            assertTrue(
                    errors.isEmpty(),
                    () -> "native host reported: " + errors.peek());
        }
    }
}
