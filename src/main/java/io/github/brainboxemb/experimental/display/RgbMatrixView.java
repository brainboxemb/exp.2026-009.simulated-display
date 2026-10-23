package io.github.brainboxemb.experimental.display;

import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;

final class RgbMatrixView extends Canvas {

    private static final double BASE_PITCH = 4.0;
    private static final double BASE_LED_DIAMETER = 3.0;
    private static final Color BACKGROUND = Color.rgb(8, 8, 8);

    private final int logicalWidth;
    private final int logicalHeight;

    private int displayScale = 1;
    private MatrixDisplayFrame lastFrame;

    RgbMatrixView(int width, int height) {
        logicalWidth = width;
        logicalHeight = height;
        updateCanvasSize();
    }

    void setDisplayScale(int scale) {
        if (scale != 1 && scale != 2) {
            throw new IllegalArgumentException(
                    "display scale must be 1 or 2");
        }

        displayScale = scale;
        updateCanvasSize();

        if (lastFrame != null) {
            render(lastFrame);
        }
    }

    void render(MatrixDisplayFrame frame) {
        lastFrame = frame;

        GraphicsContext graphics =
                getGraphicsContext2D();

        graphics.setFill(BACKGROUND);
        graphics.fillRect(
                0,
                0,
                getWidth(),
                getHeight());

        double pitch =
                BASE_PITCH * displayScale;
        double ledDiameter =
                BASE_LED_DIAMETER * displayScale;
        double inset =
                (pitch - ledDiameter) / 2.0;

        for (int y = 0; y < frame.height(); y++) {
            for (int x = 0; x < frame.width(); x++) {
                graphics.setFill(
                        Color.rgb(
                                frame.red(x, y),
                                frame.green(x, y),
                                frame.blue(x, y)));

                graphics.fillOval(
                        x * pitch + inset,
                        y * pitch + inset,
                        ledDiameter,
                        ledDiameter);
            }
        }
    }

    private void updateCanvasSize() {
        setWidth(
                logicalWidth
                        * BASE_PITCH
                        * displayScale);
        setHeight(
                logicalHeight
                        * BASE_PITCH
                        * displayScale);
    }
}
