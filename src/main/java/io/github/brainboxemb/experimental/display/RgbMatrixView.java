package io.github.brainboxemb.experimental.display;

import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;

final class RgbMatrixView extends Canvas {

    private static final double PITCH = 4.0;
    private static final double LED_DIAMETER = 3.0;
    private static final Color BACKGROUND = Color.rgb(8, 8, 8);

    RgbMatrixView(int width, int height) {
        super(width * PITCH, height * PITCH);
    }

    void render(MatrixDisplayFrame frame) {
        if (getWidth() != frame.width() * PITCH
                || getHeight() != frame.height() * PITCH) {
            setWidth(frame.width() * PITCH);
            setHeight(frame.height() * PITCH);
        }

        GraphicsContext graphics =
                getGraphicsContext2D();

        graphics.setFill(BACKGROUND);
        graphics.fillRect(
                0,
                0,
                getWidth(),
                getHeight());

        double inset =
                (PITCH - LED_DIAMETER) / 2.0;

        for (int y = 0; y < frame.height(); y++) {
            for (int x = 0; x < frame.width(); x++) {
                graphics.setFill(
                        Color.rgb(
                                frame.red(x, y),
                                frame.green(x, y),
                                frame.blue(x, y)));

                graphics.fillOval(
                        x * PITCH + inset,
                        y * PITCH + inset,
                        LED_DIAMETER,
                        LED_DIAMETER);
            }
        }
    }
}
