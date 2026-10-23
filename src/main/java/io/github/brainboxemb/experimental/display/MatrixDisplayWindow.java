package io.github.brainboxemb.experimental.display;

import java.io.IOException;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.stage.Stage;
import javafx.stage.Window;

final class MatrixDisplayWindow implements AutoCloseable {

    private static final int WIDTH = 160;
    private static final int HEIGHT = 64;

    private final Stage stage;
    private final RgbMatrixView view;
    private final Label status;
    private final MatrixDisplayProcess process;

    MatrixDisplayWindow(Window owner)
            throws IOException {
        stage = new Stage();
        stage.initOwner(owner);
        stage.setTitle(
                "Experiment 009 — 160×64 RGB Matrix");

        view =
                new RgbMatrixView(
                        WIDTH,
                        HEIGHT);
        status =
                new Label(
                        "Starting native matrix display...");

        Button demoScreen =
                new Button("Demo screen");
        Button clear =
                new Button("Clear");

        HBox controls =
                new HBox(
                        8,
                        demoScreen,
                        clear,
                        status);
        controls.setPadding(
                new Insets(12, 0, 0, 0));

        BorderPane root =
                new BorderPane();
        root.setPadding(new Insets(12));
        root.setCenter(view);
        root.setBottom(controls);

        stage.setScene(new Scene(root));
        stage.setResizable(false);

        process =
                new MatrixDisplayProcess(
                        frame -> Platform.runLater(
                                () -> {
                                    view.render(frame);
                                    status.setText(
                                            "Native C matrix: "
                                                    + frame.width()
                                                    + "×"
                                                    + frame.height()
                                                    + " RGB565 / LVGL");
                                }),
                        error -> Platform.runLater(
                                () -> status.setText(
                                        "Matrix native error: "
                                                + error)));

        demoScreen.setOnAction(
                event -> sendDemoScreen());
        clear.setOnAction(
                event -> clear());
        stage.setOnHidden(
                event -> close());
    }

    void show() {
        stage.show();
        stage.toFront();
    }

    boolean isShowing() {
        return stage.isShowing();
    }

    void toFront() {
        stage.toFront();
    }

    @Override
    public void close() {
        process.close();
    }

    private void sendDemoScreen() {
        try {
            process.requestDemoScreen();
        } catch (IOException exception) {
            status.setText(
                    "Matrix send failed: "
                            + exception.getMessage());
        }
    }

    private void clear() {
        try {
            process.clear();
        } catch (IOException exception) {
            status.setText(
                    "Matrix send failed: "
                            + exception.getMessage());
        }
    }
}
