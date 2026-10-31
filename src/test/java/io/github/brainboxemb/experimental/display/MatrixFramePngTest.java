package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.awt.image.BufferedImage;
import java.nio.file.Path;
import javax.imageio.ImageIO;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

final class MatrixFramePngTest {

    @TempDir
    Path tempDirectory;

    @Test
    void writesLogicalRgb565PixelsWithoutViewerStyling()
            throws Exception {
        MatrixDisplayFrame frame =
                new MatrixDisplayFrame(
                        2,
                        2,
                        new short[] {
                            (short)0xf800,
                            (short)0x07e0,
                            (short)0x001f,
                            (short)0x0000
                        });
        Path output =
                tempDirectory.resolve(
                        "matrix.png");

        MatrixFramePng.write(
                frame,
                output);

        BufferedImage image =
                ImageIO.read(
                        output.toFile());

        assertEquals(2, image.getWidth());
        assertEquals(2, image.getHeight());
        assertEquals(0xffff0000, image.getRGB(0, 0));
        assertEquals(0xff00ff00, image.getRGB(1, 0));
        assertEquals(0xff0000ff, image.getRGB(0, 1));
        assertEquals(0xff000000, image.getRGB(1, 1));
    }
}
