package io.github.brainboxemb.experimental.display;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.nio.file.Path;
import java.util.Objects;
import javax.imageio.ImageIO;

final class MatrixFramePng {

    private MatrixFramePng() {
    }

    static void write(
            MatrixDisplayFrame frame,
            Path path)
            throws IOException {
        Objects.requireNonNull(frame);
        Objects.requireNonNull(path);

        BufferedImage image =
                new BufferedImage(
                        frame.width(),
                        frame.height(),
                        BufferedImage.TYPE_INT_RGB);

        for (int y = 0; y < frame.height(); y++) {
            for (int x = 0; x < frame.width(); x++) {
                int rgb =
                        (frame.red(x, y) << 16)
                        | (frame.green(x, y) << 8)
                        | frame.blue(x, y);

                image.setRGB(
                        x,
                        y,
                        rgb);
            }
        }

        if (!ImageIO.write(
                image,
                "png",
                path.toFile())) {
            throw new IOException(
                    "No PNG writer is available");
        }
    }
}
