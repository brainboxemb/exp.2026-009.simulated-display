package io.github.brainboxemb.experimental.display;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

public final class SimulatedDisplayFxApplication extends Application {

    private static final int WIDTH = 90;
    private static final int HEIGHT = 35;

    private NativeDisplayProcess nativeProcess;
    private LedMatrixView display;
    private Label status;

    @Override
    public void start(Stage stage) {
        display = new LedMatrixView(WIDTH, HEIGHT);
        status = new Label("Starting native display...");

        BorderPane root = new BorderPane();
        root.setPadding(new Insets(12));
        root.setCenter(display);
        root.setBottom(createControls());

        stage.setTitle("Experiment 009 — 90×35 Serial LED Display");
        stage.setScene(new Scene(root));
        stage.setResizable(false);
        stage.show();

        try {
            nativeProcess = new NativeDisplayProcess(
                    frame -> Platform.runLater(() -> {
                        display.render(frame);
                        status.setText("Native C display: " + frame.width() + "×" + frame.height());
                    }),
                    error -> Platform.runLater(() -> status.setText("Native error: " + error)));
            send(SerialBytes.clear());
            send("HELLO WORLD".getBytes(StandardCharsets.US_ASCII));
        } catch (IOException exception) {
            status.setText(exception.getMessage());
        }
    }

    @Override
    public void stop() {
        if (nativeProcess != null) {
            nativeProcess.close();
        }
    }

    private VBox createControls() {
        TextField text = new TextField("HELLO WORLD");
        text.setPrefColumnCount(22);
        Button sendText = new Button("Send text");
        sendText.setOnAction(event -> send(text.getText().getBytes(StandardCharsets.US_ASCII)));
        text.setOnAction(event -> sendText.fire());

        Button home = new Button("Home");
        home.setOnAction(event -> send(SerialBytes.home()));
        Button clear = new Button("Clear");
        clear.setOnAction(event -> send(SerialBytes.clear()));
        Button corners = new Button("Corner pixels");
        corners.setOnAction(event -> {
            send(SerialBytes.pixel(0, 0, true));
            send(SerialBytes.pixel(WIDTH - 1, 0, true));
            send(SerialBytes.pixel(0, HEIGHT - 1, true));
            send(SerialBytes.pixel(WIDTH - 1, HEIGHT - 1, true));
        });

        Button sprite = new Button("Sprite");
        sprite.setOnAction(event -> send(SerialBytes.sprite()));

        HBox textControls = new HBox(
                8,
                new Label("ASCII:"),
                text,
                sendText,
                home,
                clear,
                corners,
                sprite);

        TextField hex = new TextField("7C 10 0A 05 01");
        hex.setPrefColumnCount(25);
        Button sendHex = new Button("Send bytes");
        sendHex.setOnAction(event -> {
            try {
                send(SerialBytes.parseHex(hex.getText()));
            } catch (IllegalArgumentException exception) {
                status.setText("Hex input: " + exception.getMessage());
            }
        });
        hex.setOnAction(event -> sendHex.fire());
        HBox byteControls = new HBox(8, new Label("Hex:"), hex, sendHex, status);

        VBox controls = new VBox(8, textControls, byteControls);
        controls.setPadding(new Insets(12, 0, 0, 0));
        return controls;
    }

    private void send(byte[] bytes) {
        if (nativeProcess == null) {
            status.setText("Native display process is not running");
            return;
        }
        try {
            nativeProcess.send(bytes);
        } catch (IOException exception) {
            status.setText("Send failed: " + exception.getMessage());
        }
    }

}
