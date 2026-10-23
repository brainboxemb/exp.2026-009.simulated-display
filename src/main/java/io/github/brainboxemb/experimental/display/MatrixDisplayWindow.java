package io.github.brainboxemb.experimental.display;

import java.io.IOException;
import java.util.List;
import javafx.animation.Animation;
import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ColorPicker;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.ToggleButton;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.stage.Stage;
import javafx.stage.Window;
import javafx.util.Duration;

final class MatrixDisplayWindow implements AutoCloseable {

    private static final int WIDTH = 160;
    private static final int HEIGHT = 64;
    private static final int ANIMATION_FRAME_COUNT = 12;
    private static final int RUNNER_FRAME_COUNT = 6;

    private final Stage stage;
    private final RgbMatrixView view;
    private final Label status;
    private final MatrixDisplayProcess process;
    private final Timeline spriteAnimation;
    private final Label spriteFrameStatus =
            new Label();

    private int animationFrame;
    private boolean spriteAnimationRunning = true;

    private final ColorPicker backgroundColor =
            new ColorPicker(Color.web("#333333"));
    private final ColorPicker lineColor =
            new ColorPicker(Color.GRAY);
    private final ColorPicker headerColor =
            new ColorPicker(Color.RED);
    private final ColorPicker rowColor =
            new ColorPicker(Color.RED);
    private final ColorPicker largeColor =
            new ColorPicker(Color.RED);
    private final ColorPicker footerColor =
            new ColorPicker(Color.RED);
    private final ColorPicker spriteColor =
            new ColorPicker(Color.RED);
    private final ColorPicker flagColor =
            new ColorPicker(Color.WHITE);
    private final ColorPicker warningBackgroundColor =
            new ColorPicker(Color.web("#0060FF"));
    private final ColorPicker warningTextColor =
            new ColorPicker(Color.WHITE);

    private final CheckBox wifiConnected =
            new CheckBox("WiFi connected");
    private final CheckBox registrationServerConnected =
            new CheckBox("Registration server connected");

    private final ComboBox<String> fontFamily =
            new ComboBox<>();
    private final ComboBox<Integer> headerSize =
            new ComboBox<>();
    private final ComboBox<Integer> rowSize =
            new ComboBox<>();
    private final ComboBox<Integer> largeSize =
            new ComboBox<>();
    private final ComboBox<Integer> footerSize =
            new ComboBox<>();

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

        configureFontControls();

        Button demoScreen =
                new Button("Demo screen");
        Button clear =
                new Button("Clear");
        ToggleButton scale2x =
                new ToggleButton("2×");
        Button previousSpriteFrame =
                new Button("◀");
        Button nextSpriteFrame =
                new Button("▶");
        Button runPauseSprites =
                new Button("Pause");

        updateSpriteFrameStatus();

        wifiConnected.setSelected(true);
        registrationServerConnected.setSelected(true);

        scale2x.setOnAction(
                event -> {
                    view.setDisplayScale(
                            scale2x.isSelected()
                                    ? 2
                                    : 1);
                    stage.sizeToScene();
                });

        HBox displayControls =
                new HBox(
                        8,
                        demoScreen,
                        clear,
                        scale2x,
                        status);

        HBox spriteControls =
                new HBox(
                        8,
                        new Label("Sprite frame:"),
                        previousSpriteFrame,
                        spriteFrameStatus,
                        nextSpriteFrame,
                        runPauseSprites);

        HBox connectionControls =
                new HBox(
                        12,
                        wifiConnected,
                        registrationServerConnected);

        Button applyStyle =
                new Button("Apply style");
        applyStyle.setOnAction(
                event -> applyStyle());

        GridPane styleControls =
                createStyleControls(
                        applyStyle);

        VBox controls =
                new VBox(
                        8,
                        displayControls,
                        spriteControls,
                        connectionControls,
                        styleControls);
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
        wifiConnected.setOnAction(
                event -> sendConnectionStatus());
        registrationServerConnected.setOnAction(
                event -> sendConnectionStatus());
        stage.setOnHidden(
                event -> close());

        spriteAnimation =
                new Timeline(
                        new KeyFrame(
                                Duration.millis(160),
                                event -> animateSprites()));
        spriteAnimation.setCycleCount(
                Animation.INDEFINITE);
        spriteAnimation.play();

        previousSpriteFrame.setOnAction(
                event -> stepSpriteFrame(
                        -1,
                        runPauseSprites));
        nextSpriteFrame.setOnAction(
                event -> stepSpriteFrame(
                        1,
                        runPauseSprites));
        runPauseSprites.setOnAction(
                event -> toggleSpriteAnimation(
                        runPauseSprites));

        /*
         * Keep the visible controls and the native renderer in sync from
         * the first frame onward.
         */
        applyStyle();
        sendConnectionStatus();
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
        spriteAnimation.stop();
        process.close();
    }

    private GridPane createStyleControls(
            Button applyStyle) {
        GridPane grid = new GridPane();
        grid.setHgap(8);
        grid.setVgap(6);

        grid.add(new Label("Font"), 0, 0);
        grid.add(fontFamily, 1, 0);

        grid.add(new Label("Background"), 2, 0);
        grid.add(backgroundColor, 3, 0);

        grid.add(new Label("Lines"), 4, 0);
        grid.add(lineColor, 5, 0);

        grid.add(new Label("Header"), 0, 1);
        grid.add(headerSize, 1, 1);
        grid.add(headerColor, 2, 1);

        grid.add(new Label("Rows"), 3, 1);
        grid.add(rowSize, 4, 1);
        grid.add(rowColor, 5, 1);

        grid.add(new Label("Large"), 0, 2);
        grid.add(largeSize, 1, 2);
        grid.add(largeColor, 2, 2);

        grid.add(new Label("Footer"), 3, 2);
        grid.add(footerSize, 4, 2);
        grid.add(footerColor, 5, 2);

        grid.add(new Label("Sprites"), 0, 3);
        grid.add(spriteColor, 1, 3);

        grid.add(new Label("Flag"), 2, 3);
        grid.add(flagColor, 3, 3);

        grid.add(new Label("Warning bg"), 4, 3);
        grid.add(warningBackgroundColor, 5, 3);

        grid.add(new Label("Warning text"), 0, 4);
        grid.add(warningTextColor, 1, 4);

        grid.add(applyStyle, 6, 4);
        return grid;
    }

    private void configureFontControls() {
        fontFamily.getItems().addAll(
                "UNSCII",
                "VBZ");
        fontFamily.setValue("VBZ");

        refreshFontSizes();
        fontFamily.setOnAction(
                event -> refreshFontSizes());
    }

    private void refreshFontSizes() {
        boolean vbz =
                "VBZ".equals(
                        fontFamily.getValue());

        if (vbz) {
            List<Integer> regularSizes =
                    List.of(9);

            setSizes(
                    headerSize,
                    regularSizes,
                    9);
            setSizes(
                    rowSize,
                    regularSizes,
                    9);
            setSizes(
                    largeSize,
                    List.of(
                            9,
                            12),
                    12);
            setSizes(
                    footerSize,
                    regularSizes,
                    9);
            return;
        }

        List<Integer> sizes =
                List.of(
                        8,
                        10,
                        16);

        setSizes(
                headerSize,
                sizes,
                10);
        setSizes(
                rowSize,
                sizes,
                10);
        setSizes(
                largeSize,
                sizes,
                16);
        setSizes(
                footerSize,
                sizes,
                10);
    }

    private static void setSizes(
            ComboBox<Integer> control,
            List<Integer> sizes,
            int preferred) {
        Integer previous =
                control.getValue();

        control.getItems().setAll(sizes);

        if (previous != null
                && sizes.contains(previous)) {
            control.setValue(previous);
        } else if (sizes.contains(preferred)) {
            control.setValue(preferred);
        } else {
            control.setValue(sizes.get(0));
        }
    }

    private void applyStyle() {
        try {
            process.applyTheme(
                    new MatrixDisplayThemeConfig(
                            rgb565(backgroundColor.getValue()),
                            rgb565(lineColor.getValue()),
                            rgb565(headerColor.getValue()),
                            rgb565(rowColor.getValue()),
                            rgb565(largeColor.getValue()),
                            rgb565(footerColor.getValue()),
                            rgb565(spriteColor.getValue()),
                            rgb565(flagColor.getValue()),
                            rgb565(warningBackgroundColor.getValue()),
                            rgb565(warningTextColor.getValue()),
                            "VBZ".equals(
                                    fontFamily.getValue())
                                    ? 1
                                    : 0,
                            headerSize.getValue(),
                            rowSize.getValue(),
                            largeSize.getValue(),
                            footerSize.getValue()));
        } catch (IOException exception) {
            status.setText(
                    "Matrix style failed: "
                            + exception.getMessage());
        }
    }

    private static int rgb565(Color color) {
        int red =
                (int)Math.round(
                        color.getRed() * 255.0);
        int green =
                (int)Math.round(
                        color.getGreen() * 255.0);
        int blue =
                (int)Math.round(
                        color.getBlue() * 255.0);

        return ((red >> 3) << 11)
                | ((green >> 2) << 5)
                | (blue >> 3);
    }

    private void animateSprites() {
        animationFrame =
                (animationFrame + 1)
                % ANIMATION_FRAME_COUNT;
        updateSpriteFrameStatus();

        try {
            process.animationFrame(
                    animationFrame);
        } catch (IOException exception) {
            spriteAnimation.stop();
            spriteAnimationRunning = false;
            status.setText(
                    "Matrix animation failed: "
                            + exception.getMessage());
        }
    }

    private void stepSpriteFrame(
            int delta,
            Button runPauseSprites) {
        spriteAnimation.pause();
        spriteAnimationRunning = false;
        runPauseSprites.setText("Run");

        animationFrame =
                Math.floorMod(
                        animationFrame + delta,
                        ANIMATION_FRAME_COUNT);
        updateSpriteFrameStatus();

        try {
            process.animationFrame(
                    animationFrame);
        } catch (IOException exception) {
            status.setText(
                    "Matrix animation failed: "
                            + exception.getMessage());
        }
    }

    private void toggleSpriteAnimation(
            Button runPauseSprites) {
        if (spriteAnimationRunning) {
            spriteAnimation.pause();
            spriteAnimationRunning = false;
            runPauseSprites.setText("Run");
            return;
        }

        spriteAnimation.play();
        spriteAnimationRunning = true;
        runPauseSprites.setText("Pause");
    }

    private void updateSpriteFrameStatus() {
        spriteFrameStatus.setText(
                "master "
                        + animationFrame
                        + " · runner "
                        + animationFrame % RUNNER_FRAME_COUNT
                        + " · warmup "
                        + animationFrame / 2);
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

    private void sendConnectionStatus() {
        try {
            process.connectionStatus(
                    wifiConnected.isSelected(),
                    registrationServerConnected.isSelected());
        } catch (IOException exception) {
            status.setText(
                    "Matrix status failed: "
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
