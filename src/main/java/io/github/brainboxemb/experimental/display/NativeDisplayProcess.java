package io.github.brainboxemb.experimental.display;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

final class NativeDisplayProcess implements AutoCloseable {

    private static final byte[] MAGIC = {'S', 'D', 'F', '1'};
    private static final int MAX_FRAME_PIXELS = 1_000_000;
    private static final int ANIMATION_TICK_REQUEST = -1;
    private static final long ANIMATION_TICK_MILLIS = 200;

    private final Process process;
    private final DataOutputStream serialInput;
    private final ScheduledExecutorService animationClock;

    NativeDisplayProcess(
            Consumer<DisplayFrame> frameConsumer,
            Consumer<String> errorConsumer) throws IOException {
        Objects.requireNonNull(frameConsumer);
        Objects.requireNonNull(errorConsumer);

        Path executable = findExecutable();
        process =
                new ProcessBuilder(
                        executable.toString())
                        .start();

        serialInput =
                new DataOutputStream(
                        new BufferedOutputStream(
                                process.getOutputStream()));

        Thread.ofPlatform()
                .name("display-host-frame-reader")
                .daemon(true)
                .start(() -> readFrames(
                        frameConsumer,
                        errorConsumer));

        Thread.ofPlatform()
                .name("display-host-error-reader")
                .daemon(true)
                .start(() -> {
                    try {
                        process.getErrorStream()
                                .transferTo(System.err);
                    } catch (IOException exception) {
                        errorConsumer.accept(
                                "Native stderr: "
                                        + exception.getMessage());
                    }
                });

        animationClock =
                Executors.newSingleThreadScheduledExecutor(
                        runnable ->
                                Thread.ofPlatform()
                                        .name("display-animation-clock")
                                        .daemon(true)
                                        .unstarted(runnable));

        animationClock.scheduleAtFixedRate(
                () -> tick(errorConsumer),
                ANIMATION_TICK_MILLIS,
                ANIMATION_TICK_MILLIS,
                TimeUnit.MILLISECONDS);
    }

    synchronized void send(byte[] serialBytes) throws IOException {
        serialInput.writeInt(serialBytes.length);
        serialInput.write(serialBytes);
        serialInput.flush();
    }

    @Override
    public void close() {
        animationClock.shutdownNow();

        try {
            serialInput.close();
        } catch (IOException ignored) {
        }

        process.destroy();
    }

    private synchronized void sendAnimationTick() throws IOException {
        serialInput.writeInt(ANIMATION_TICK_REQUEST);
        serialInput.flush();
    }

    private void tick(Consumer<String> errorConsumer) {
        if (!process.isAlive()) {
            return;
        }

        try {
            sendAnimationTick();
        } catch (IOException exception) {
            if (process.isAlive()) {
                errorConsumer.accept(
                        "Animation clock: "
                                + exception.getMessage());
            }
        }
    }

    private void readFrames(
            Consumer<DisplayFrame> frameConsumer,
            Consumer<String> errorConsumer) {
        try (DataInputStream input =
                new DataInputStream(
                        new BufferedInputStream(
                                process.getInputStream()))) {
            while (true) {
                verifyMagic(input);

                int width = input.readUnsignedShort();
                int height = input.readUnsignedShort();
                int payloadSize = input.readInt();
                int expectedSize =
                        Math.multiplyExact(width, height);

                if (payloadSize != expectedSize
                        || payloadSize < 0
                        || payloadSize > MAX_FRAME_PIXELS) {
                    throw new IOException(
                            "Invalid native frame size "
                                    + payloadSize
                                    + " for "
                                    + width
                                    + "x"
                                    + height);
                }

                byte[] pixels =
                        input.readNBytes(payloadSize);

                if (pixels.length != payloadSize) {
                    throw new EOFException(
                            "Native display process ended mid-frame");
                }

                frameConsumer.accept(
                        new DisplayFrame(
                                width,
                                height,
                                pixels));
            }
        } catch (EOFException eof) {
            if (process.isAlive()) {
                errorConsumer.accept(eof.getMessage());
            }
        } catch (IOException | ArithmeticException exception) {
            errorConsumer.accept(exception.getMessage());
        }
    }

    private static void verifyMagic(DataInputStream input)
            throws IOException {
        for (byte expected : MAGIC) {
            if (input.readByte() != expected) {
                throw new IOException(
                        "Invalid frame header from native display process");
            }
        }
    }

    private static Path findExecutable() throws IOException {
        String configured =
                System.getProperty("display.native");

        if (configured != null && !configured.isBlank()) {
            Path path =
                    Path.of(configured)
                            .toAbsolutePath()
                            .normalize();

            if (Files.isRegularFile(path)) {
                return path;
            }

            throw new IOException(
                    "Configured native display executable does not exist: "
                            + path);
        }

        List<Path> candidates =
                isWindows()
                        ? List.of(
                                Path.of(
                                        "native",
                                        "build",
                                        "Release",
                                        "display-host.exe"),
                                Path.of(
                                        "native",
                                        "build",
                                        "display-host.exe"))
                        : List.of(
                                Path.of(
                                        "native",
                                        "build",
                                        "display-host"));

        for (Path candidate : candidates) {
            if (Files.isRegularFile(candidate)) {
                return candidate
                        .toAbsolutePath()
                        .normalize();
            }
        }

        throw new IOException(
                "Native display host not found. Build it first with "
                        + "cmake -S native -B native/build && "
                        + "cmake --build native/build --config Release");
    }

    private static boolean isWindows() {
        return System.getProperty(
                        "os.name",
                        "")
                .toLowerCase()
                .contains("win");
    }
}
