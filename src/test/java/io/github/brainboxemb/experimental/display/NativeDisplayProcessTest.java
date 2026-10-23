package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.time.Duration;
import java.util.concurrent.LinkedBlockingQueue;
import org.junit.jupiter.api.Test;

final class NativeDisplayProcessTest {

    @Test
    void exchangesBinarySerialBytesWithNativeHost() throws Exception {
        var frames = new LinkedBlockingQueue<DisplayFrame>();
        var errors = new LinkedBlockingQueue<String>();

        try (var process = new NativeDisplayProcess(frames::add, errors::add)) {
            DisplayFrame initial = frames.poll(Duration.ofSeconds(5));
            assertNotNull(initial, "native host did not emit its initial frame");
            assertEquals(90, initial.width());
            assertEquals(35, initial.height());

            process.send(new byte[]{0x7c, 0x10, 0x1a, 0x05, 0x01});

            DisplayFrame updated = frames.poll(Duration.ofSeconds(5));
            assertNotNull(updated, "native host did not emit the changed frame");
            assertTrue(updated.isOn(26, 5), "0x1A must survive the Windows process pipe unchanged");
            assertTrue(errors.isEmpty(), () -> "native host reported: " + errors.peek());
        }
    }
}
