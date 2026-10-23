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
import java.util.function.Consumer;

final class MatrixDisplayProcess implements AutoCloseable {

    private static final byte[] MAGIC = {'S', 'D', 'F', '2'};
    private static final int REQUEST_DEMO_SCREEN = 0;
    private static final int REQUEST_CLEAR = 1;
    private static final int MAX_PIXELS = 1_000_000;

    private final Process process;
    private final DataOutputStream input;

    MatrixDisplayProcess(
            Consumer<MatrixDisplayFrame> frameConsumer,
            Consumer<String> errorConsumer) throws IOException {
        Objects.requireNonNull(frameConsumer);
        Objects.requireNonNull(errorConsumer);

        process =
                new ProcessBuilder(
                        findExecutable().toString())
                        .start();

        input =
                new DataOutputStream(
                        new BufferedOutputStream(
                                process.getOutputStream()));

        Thread.ofPlatform()
                .name("matrix-display-frame-reader")
                .daemon(true)
                .start(() -> readFrames(
                        frameConsumer,
                        errorConsumer));

        Thread.ofPlatform()
                .name("matrix-display-error-reader")
                .daemon(true)
                .start(() -> {
                    try {
                        process.getErrorStream()
                                .transferTo(System.err);
                    } catch (IOException exception) {
                        errorConsumer.accept(
                                "Matrix native stderr: "
                                        + exception.getMessage());
                    }
                });
    }

    synchronized void requestDemoScreen()
            throws IOException {
        sendRequest(REQUEST_DEMO_SCREEN);
    }

    synchronized void clear()
            throws IOException {
        sendRequest(REQUEST_CLEAR);
    }

    @Override
    public void close() {
        try {
            input.close();
        } catch (IOException ignored) {
        }

        process.destroy();
    }

    private void sendRequest(int request)
            throws IOException {
        input.writeInt(request);
        input.flush();
    }

    private void readFrames(
            Consumer<MatrixDisplayFrame> frameConsumer,
            Consumer<String> errorConsumer) {
        try (DataInputStream output =
                new DataInputStream(
                        new BufferedInputStream(
                                process.getInputStream()))) {
            while (true) {
                verifyMagic(output);

                int width = output.readUnsignedShort();
                int height = output.readUnsignedShort();
                int payloadSize = output.readInt();
                int pixelCount =
                        Math.multiplyExact(
                                width,
                                height);
                int expectedSize =
                        Math.multiplyExact(
                                pixelCount,
                                2);

                if (payloadSize != expectedSize
                        || pixelCount < 0
                        || pixelCount > MAX_PIXELS) {
                    throw new IOException(
                            "Invalid matrix frame size "
                                    + payloadSize
                                    + " for "
                                    + width
                                    + "x"
                                    + height);
                }

                short[] pixels =
                        new short[pixelCount];

                for (int index = 0;
                        index < pixelCount;
                        index++) {
                    pixels[index] =
                            output.readShort();
                }

                frameConsumer.accept(
                        new MatrixDisplayFrame(
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

    private static void verifyMagic(
            DataInputStream output)
            throws IOException {
        for (byte expected : MAGIC) {
            if (output.readByte() != expected) {
                throw new IOException(
                        "Invalid frame header from matrix display process");
            }
        }
    }

    private static Path findExecutable()
            throws IOException {
        String configured =
                System.getProperty(
                        "matrix.display.native");

        if (configured != null
                && !configured.isBlank()) {
            Path path =
                    Path.of(configured)
                            .toAbsolutePath()
                            .normalize();

            if (Files.isRegularFile(path)) {
                return path;
            }

            throw new IOException(
                    "Configured matrix display executable does not exist: "
                            + path);
        }

        List<Path> candidates =
                isWindows()
                        ? List.of(
                                Path.of(
                                        "native",
                                        "build",
                                        "Release",
                                        "matrix-display-host.exe"),
                                Path.of(
                                        "native",
                                        "build",
                                        "matrix-display-host.exe"))
                        : List.of(
                                Path.of(
                                        "native",
                                        "build",
                                        "matrix-display-host"));

        for (Path candidate : candidates) {
            if (Files.isRegularFile(candidate)) {
                return candidate
                        .toAbsolutePath()
                        .normalize();
            }
        }

        throw new IOException(
                "Matrix display host not found. Build native code first.");
    }

    private static boolean isWindows() {
        return System.getProperty(
                        "os.name",
                        "")
                .toLowerCase()
                .contains("win");
    }
}
