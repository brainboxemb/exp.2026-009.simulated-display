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
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.control.TextArea;
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
    private final Timeline demoSpriteAnimation;
    private final Timeline serialSpriteAnimation;
    private final Label spriteFrameStatus =
            new Label();

    private final ColorPicker offPixelColor =
            new ColorPicker(Color.web("#333333"));
    private final ColorPicker lineColor =
            new ColorPicker(Color.GRAY);
    private final ColorPicker headerColor =
            new ColorPicker(Color.web("#FFCC66"));
    private final ColorPicker rowColor =
            new ColorPicker(Color.web("#EEEEEE"));
    private final ColorPicker largeColor =
            new ColorPicker(Color.LIME);
    private final ColorPicker footerColor =
            new ColorPicker(Color.web("#FFCC66"));
    private final ColorPicker spriteColor =
            new ColorPicker(Color.web("#FFCC66"));
    private final ColorPicker exchangePointBackgroundColor =
            new ColorPicker(Color.LIME);
    private final ColorPicker exchangePointTextColor =
            new ColorPicker(Color.BLACK);
    private final ColorPicker flagColor =
            new ColorPicker(Color.web("#EEEEEE"));
    private final ColorPicker warningBackgroundColor =
            new ColorPicker(Color.web("#0060FF"));
    private final ColorPicker warningTextColor =
            new ColorPicker(Color.web("#EEEEEE"));
    private final ColorPicker warningStrikeColor =
            new ColorPicker(Color.RED);
    private final ColorPicker syncBackgroundColor =
            new ColorPicker(Color.LIME);
    private final ColorPicker syncIconColor =
            new ColorPicker(Color.BLACK);

    private final CheckBox wifiConnected =
            new CheckBox("WiFi connected");
    private final CheckBox registrationServerConnected =
            new CheckBox("Registration server connected");
    private final CheckBox synchronizing =
            new CheckBox("Synchronizing");

    private final ComboBox<String> layoutMode =
            new ComboBox<>();
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

    private final ComboBox<String> serialPreset =
            new ComboBox<>();
    private final TextArea serialProtocol =
            new TextArea();

    private int animationFrame;
    private boolean demoSpriteAnimationRunning = true;
    private boolean serialMode;

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
        configureLayoutControl();
        configureSerialControls();

        ToggleButton scale2x =
                new ToggleButton("2×");
        scale2x.setOnAction(
                event -> {
                    view.setDisplayScale(
                            scale2x.isSelected()
                                    ? 2
                                    : 1);
                    stage.sizeToScene();
                });

        offPixelColor.setOnAction(
                event -> view.setOffPixelColor(
                        offPixelColor.getValue()));

        HBox viewerControls =
                new HBox(
                        8,
                        scale2x,
                        new Label("Off pixel:"),
                        offPixelColor,
                        status);

        Tab demoTab =
                new Tab(
                        "Demo",
                        createDemoControls());
        demoTab.setClosable(false);

        Tab serialTab =
                new Tab(
                        "Serial",
                        createSerialControls());
        serialTab.setClosable(false);

        TabPane modeTabs =
                new TabPane(
                        demoTab,
                        serialTab);
        modeTabs.getSelectionModel()
                .select(demoTab);

        VBox controls =
                new VBox(
                        8,
                        viewerControls,
                        modeTabs);
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
                                                    + " RGB565 / "
                                                    + (serialMode
                                                            ? "serial"
                                                            : "demo"));
                                }),
                        error -> Platform.runLater(
                                () -> status.setText(
                                        "Matrix native error: "
                                                + error)));

        demoSpriteAnimation =
                new Timeline(
                        new KeyFrame(
                                Duration.millis(160),
                                event -> animateDemoSprites()));
        demoSpriteAnimation.setCycleCount(
                Animation.INDEFINITE);
        demoSpriteAnimation.play();

        serialSpriteAnimation =
                new Timeline(
                        new KeyFrame(
                                Duration.millis(20),
                                event -> tickSerialSprites()));
        serialSpriteAnimation.setCycleCount(
                Animation.INDEFINITE);
        serialSpriteAnimation.play();

        modeTabs.getSelectionModel()
                .selectedItemProperty()
                .addListener(
                        (observable, previous, selected) -> {
                            if (selected == serialTab) {
                                activateSerialMode();
                            } else {
                                activateDemoMode();
                            }
                        });

        stage.setOnHidden(
                event -> close());

        applyDemoStyle();
        sendDemoLayout();
        sendConnectionStatus();
        activateDemoMode();
    }

    void show() {
        stage.show();
        stage.toFront();
    }

    @Override
    public void close() {
        demoSpriteAnimation.stop();
        serialSpriteAnimation.stop();
        process.close();
    }

    private VBox createDemoControls() {
        Button previousSpriteFrame =
                new Button("◀");
        Button nextSpriteFrame =
                new Button("▶");
        Button runPauseSprites =
                new Button("Pause");

        updateSpriteFrameStatus();

        wifiConnected.setSelected(true);
        registrationServerConnected.setSelected(true);
        synchronizing.setSelected(false);

        HBox layoutControls =
                new HBox(
                        8,
                        new Label("Layout:"),
                        layoutMode);

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
                        registrationServerConnected,
                        synchronizing);

        Button applyStyle =
                new Button("Apply style");

        GridPane styleControls =
                createStyleControls(
                        applyStyle);

        previousSpriteFrame.setOnAction(
                event -> stepDemoSpriteFrame(
                        -1,
                        runPauseSprites));
        nextSpriteFrame.setOnAction(
                event -> stepDemoSpriteFrame(
                        1,
                        runPauseSprites));
        runPauseSprites.setOnAction(
                event -> toggleDemoSpriteAnimation(
                        runPauseSprites));

        wifiConnected.setOnAction(
                event -> sendConnectionStatus());
        registrationServerConnected.setOnAction(
                event -> sendConnectionStatus());
        synchronizing.setOnAction(
                event -> sendConnectionStatus());
        layoutMode.setOnAction(
                event -> sendDemoLayout());
        applyStyle.setOnAction(
                event -> applyDemoStyle());

        VBox box =
                new VBox(
                        8,
                        layoutControls,
                        spriteControls,
                        connectionControls,
                        styleControls);
        box.setPadding(new Insets(8, 0, 0, 0));
        return box;
    }

    private VBox createSerialControls() {
        Button loadPreset =
                new Button("Load preset");
        Button sendPreset =
                new Button("Send preset");
        Button sendEdited =
                new Button("Send edited");

        loadPreset.setOnAction(
                event -> serialProtocol.setText(
                        MatrixSerialExamples.byName(
                                serialPreset.getValue())));

        sendPreset.setOnAction(
                event -> {
                    String script =
                            MatrixSerialExamples.byName(
                                    serialPreset.getValue());
                    serialProtocol.setText(script);
                    sendSerialProtocol(script);
                });

        sendEdited.setOnAction(
                event -> sendSerialProtocol(
                        serialProtocol.getText()));

        HBox presetControls =
                new HBox(
                        8,
                        new Label("Profile:"),
                        serialPreset,
                        loadPreset,
                        sendPreset,
                        sendEdited);

        VBox box =
                new VBox(
                        8,
                        presetControls,
                        serialProtocol);
        box.setPadding(new Insets(8, 0, 0, 0));
        return box;
    }

    private GridPane createStyleControls(
            Button applyStyle) {
        GridPane grid = new GridPane();
        grid.setHgap(8);
        grid.setVgap(6);

        grid.add(new Label("Font"), 0, 0);
        grid.add(fontFamily, 1, 0);

        grid.add(new Label("Lines"), 2, 0);
        grid.add(lineColor, 3, 0);

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

        grid.add(new Label("Exchange bg"), 2, 3);
        grid.add(exchangePointBackgroundColor, 3, 3);

        grid.add(new Label("Exchange text"), 4, 3);
        grid.add(exchangePointTextColor, 5, 3);

        grid.add(new Label("Flag"), 0, 4);
        grid.add(flagColor, 1, 4);

        grid.add(new Label("Warning bg"), 2, 4);
        grid.add(warningBackgroundColor, 3, 4);

        grid.add(new Label("Warning text"), 4, 4);
        grid.add(warningTextColor, 5, 4);

        grid.add(new Label("Warning slash"), 0, 5);
        grid.add(warningStrikeColor, 1, 5);

        grid.add(new Label("Sync bg"), 2, 5);
        grid.add(syncBackgroundColor, 3, 5);

        grid.add(new Label("Sync icon"), 4, 5);
        grid.add(syncIconColor, 5, 5);

        grid.add(applyStyle, 6, 5);
        return grid;
    }

    private void configureLayoutControl() {
        layoutMode.getItems().addAll(
                "Startup",
                "Registraties",
                "Registraties (>3)",
                "Registraties + klaarstaan",
                "Registraties + klaarstaan (>4 klaar)",
                "Font test");
        layoutMode.setValue(
                "Startup");
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

    private void configureSerialControls() {
        serialPreset.getItems().addAll(
                MatrixSerialExamples.names());
        serialPreset.setValue(
                MatrixSerialExamples.CURRENT_RACE_SCREEN);

        serialProtocol.setText(
                MatrixSerialExamples.byName(
                        serialPreset.getValue()));
        serialProtocol.setPrefColumnCount(68);
        serialProtocol.setPrefRowCount(8);
        serialProtocol.setWrapText(false);
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
                            12,
                            14),
                    14);
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

    private void activateDemoMode() {
        serialMode = false;
        sendDemoLayout();
    }

    private void activateSerialMode() {
        serialMode = true;
        sendSerialProtocol(
                serialProtocol.getText());
    }

    private void applyDemoStyle() {
        if (serialMode) {
            return;
        }

        try {
            process.applyTheme(
                    new MatrixDisplayThemeConfig(
                            0x0000,
                            rgb565(lineColor.getValue()),
                            rgb565(headerColor.getValue()),
                            rgb565(rowColor.getValue()),
                            rgb565(largeColor.getValue()),
                            rgb565(footerColor.getValue()),
                            rgb565(spriteColor.getValue()),
                            rgb565(exchangePointBackgroundColor.getValue()),
                            rgb565(exchangePointTextColor.getValue()),
                            rgb565(flagColor.getValue()),
                            rgb565(warningBackgroundColor.getValue()),
                            rgb565(warningTextColor.getValue()),
                            rgb565(warningStrikeColor.getValue()),
                            rgb565(syncBackgroundColor.getValue()),
                            rgb565(syncIconColor.getValue()),
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

    private void sendDemoLayout() {
        if (serialMode) {
            return;
        }

        try {
            if ("Startup".equals(
                    layoutMode.getValue())) {
                process.requestStartupScreen();
                return;
            }

            process.layout(
                    switch (layoutMode.getValue()) {
                        case "Registraties" -> 0;
                        case "Registraties (>3)" -> 1;
                        case "Registraties + klaarstaan" -> 2;
                        case "Registraties + klaarstaan (>4 klaar)" -> 3;
                        case "Font test" -> 4;
                        default -> throw new IllegalStateException(
                                "Unknown matrix layout");
                    });
        } catch (IOException exception) {
            status.setText(
                    "Matrix layout failed: "
                            + exception.getMessage());
        }
    }

    private void sendConnectionStatus() {
        if (serialMode) {
            return;
        }

        try {
            process.connectionStatus(
                    wifiConnected.isSelected(),
                    registrationServerConnected.isSelected(),
                    synchronizing.isSelected());
        } catch (IOException exception) {
            status.setText(
                    "Matrix status failed: "
                            + exception.getMessage());
        }
    }

    private void animateDemoSprites() {
        if (serialMode) {
            return;
        }

        animationFrame =
                (animationFrame + 1)
                % ANIMATION_FRAME_COUNT;
        updateSpriteFrameStatus();

        try {
            process.animationFrame(
                    animationFrame);
        } catch (IOException exception) {
            demoSpriteAnimation.stop();
            demoSpriteAnimationRunning = false;
            status.setText(
                    "Matrix demo animation failed: "
                            + exception.getMessage());
        }
    }

    private void stepDemoSpriteFrame(
            int delta,
            Button runPauseSprites) {
        demoSpriteAnimation.pause();
        demoSpriteAnimationRunning = false;
        runPauseSprites.setText("Run");

        animationFrame =
                Math.floorMod(
                        animationFrame + delta,
                        ANIMATION_FRAME_COUNT);
        updateSpriteFrameStatus();

        if (serialMode) {
            return;
        }

        try {
            process.animationFrame(
                    animationFrame);
        } catch (IOException exception) {
            status.setText(
                    "Matrix demo animation failed: "
                            + exception.getMessage());
        }
    }

    private void toggleDemoSpriteAnimation(
            Button runPauseSprites) {
        if (demoSpriteAnimationRunning) {
            demoSpriteAnimation.pause();
            demoSpriteAnimationRunning = false;
            runPauseSprites.setText("Run");
            return;
        }

        demoSpriteAnimation.play();
        demoSpriteAnimationRunning = true;
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

    private void sendSerialProtocol(
            String text) {
        try {
            process.serial(
                    SerialBytes.parseProtocolText(
                            text));
        } catch (IOException | IllegalArgumentException exception) {
            status.setText(
                    "Matrix serial failed: "
                            + exception.getMessage());
        }
    }

    private void tickSerialSprites() {
        if (!serialMode) {
            return;
        }

        try {
            process.serialTick();
        } catch (IOException exception) {
            serialSpriteAnimation.stop();
            status.setText(
                    "Matrix serial animation failed: "
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
}
