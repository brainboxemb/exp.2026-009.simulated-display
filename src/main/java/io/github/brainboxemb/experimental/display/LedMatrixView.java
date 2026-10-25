package io.github.brainboxemb.experimental.display;

import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;

final class LedMatrixView extends Canvas {

    private static final double PITCH = 8.0;
    private static final double LED_DIAMETER = 6.0;
    private static final Color BACKGROUND = Color.rgb(8, 8, 8);
    private static final Color LED_OFF = Color.rgb(42, 8, 8);
    private static final Color LED_ON = Color.rgb(255, 38, 24);

    LedMatrixView(int width, int height) {
        super(width * PITCH, height * PITCH);
        render(new DisplayFrame(width, height, new byte[width * height]));
    }

    void render(DisplayFrame frame) {
        if (getWidth() != frame.width() * PITCH || getHeight() != frame.height() * PITCH) {
            setWidth(frame.width() * PITCH);
            setHeight(frame.height() * PITCH);
        }

        GraphicsContext graphics = getGraphicsContext2D();
        graphics.setFill(BACKGROUND);
        graphics.fillRect(0, 0, getWidth(), getHeight());

        double inset = (PITCH - LED_DIAMETER) / 2.0;
        for (int y = 0; y < frame.height(); y++) {
            for (int x = 0; x < frame.width(); x++) {
                graphics.setFill(frame.isOn(x, y) ? LED_ON : LED_OFF);
                graphics.fillOval(x * PITCH + inset, y * PITCH + inset, LED_DIAMETER, LED_DIAMETER);
            }
        }
    }
}
