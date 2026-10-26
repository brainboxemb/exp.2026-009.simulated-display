package io.github.brainboxemb.experimental.display;

import java.util.List;

final class MatrixSerialExamples {

    static final String CURRENT_RACE_SCREEN =
            "Current race screen";
    static final String PAGES_LINES =
            "Pages + lines";
    static final String FONTS_COLORS =
            "Fonts + colors";
    static final String BUILT_IN_ICONS =
            "Built-in icons";
    static final String SPRITES_SPEED =
            "Sprites + speed";
    static final String NOTIFICATIONS =
            "Notifications";
    static final String BRIGHTNESS =
            "Brightness";

    private static final String BEGIN =
            "/UB/PS0/PC/PS1/PC/PS2/PC/XB99/CB16/N10/N20";

    private MatrixSerialExamples() {
    }

    static List<String> names() {
        return List.of(
                CURRENT_RACE_SCREEN,
                PAGES_LINES,
                FONTS_COLORS,
                BUILT_IN_ICONS,
                SPRITES_SPEED,
                NOTIFICATIONS,
                BRIGHTNESS);
    }

    static String byName(String name) {
        return switch (name) {
            case CURRENT_RACE_SCREEN -> currentRaceScreen();
            case PAGES_LINES -> pagesAndLines();
            case FONTS_COLORS -> fontsAndColors();
            case BUILT_IN_ICONS -> builtInIcons();
            case SPRITES_SPEED -> spritesAndSpeed();
            case NOTIFICATIONS -> notifications();
            case BRIGHTNESS -> brightness();
            default -> throw new IllegalArgumentException(
                    "Unknown matrix preset: " + name);
        };
    }

    static String currentRaceScreen() {
        return String.join(
                "\n",
                BEGIN,
                "/CI0C00C01/CI1C10C01C05/CI2C10C01C05/CI3C06C00",
                "/PS0/CF04/L11000/DL11159/L11081/DL52081/L52000/DL52159",
                "/PS1/CF06/L00000/DF10015",
                "/FS09/CF00/L00001"
                        + "99",
                "/CF13/L00056"
                        + "99:99:99"
                        + "/L00139"
                        + "999",
                "/CF01/L13012"
                        + "999"
                        + "/L13048"
                        + "99:99"
                        + "/L26012"
                        + "999"
                        + "/L26048"
                        + "99:99"
                        + "/L39012"
                        + "999"
                        + "/L39048"
                        + "99:99",
                "/FS14/CF06/L14094"
                        + "999"
                        + "/L14128"
                        + "999"
                        + "/L36094"
                        + "999"
                        + "/L36128"
                        + "999",
                "/FS09/CF13/L53001"
                        + "520 234 346 12",
                "/PS2/L02132/BI0"
                        + "/CF13/L13001/S0"
                        + "/L14082/S1"
                        + "/SV4",
                "/UE");
    }

    private static String pagesAndLines() {
        return String.join(
                "\n",
                BEGIN,
                "/PS0/CF04/L08010/DL08149/L55010/DL55149",
                "/PS1/CF06/L08040/DL55040/L08080/DL55080",
                "/PS2/CF13/L08120/DL55120/L32010/DL32149",
                "/UE");
    }

    private static String fontsAndColors() {
        return String.join(
                "\n",
                BEGIN,
                "/PS1/CB03/FS09/CF01/L04005FONT 09  OPAQUE"
                        + "/CB16/L18005TRANSPARENT",
                "/FS09/CF13/L18005COLOR 13",
                "/FS12/CF06/L32005"
                        + "1234567890",
                "/FS14/CF06/L47005"
                        + "1234567890",
                "/UE");
    }

    private static String builtInIcons() {
        return String.join(
                "\n",
                BEGIN,
                "/CI0C00C01"
                        + "/CI1C10C01C05"
                        + "/CI2C10C01C05"
                        + "/CI3C06C00",
                "/PS1/L08010/BI0"
                        + "/L08035/BI1"
                        + "/L08065/BI2"
                        + "/L08095/BI3",
                "/FS09/CF01/CB16/L30005"
                        + "FLAG  WIFI  LINK  SYNC",
                "/UE");
    }

    private static String spritesAndSpeed() {
        return String.join(
                "\n",
                BEGIN,
                "/PS1/CF13/L18025/S0"
                        + "/L18085/S1"
                        + "/SV4",
                "/FS09/CF01/CB16/L42015"
                        + "RUNNER      WARMUP",
                "/UE");
    }

    private static String notifications() {
        return String.join(
                "\n",
                BEGIN,
                "/CI1C10C01C05/CI2C10C01C05",
                "/PS1/FS09/CF13/L00056"
                        + "999",
                "/N11/N21",
                "/UE");
    }

    private static String brightness() {
        return String.join(
                "\n",
                BEGIN,
                "/XB35",
                "/PS1/FS09/CF13/CB16/L18020"
                        + "BRIGHTNESS 35",
                "/CF01/L36020"
                        + "XB00..XB99",
                "/UE");
    }
}
