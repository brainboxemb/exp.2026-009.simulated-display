#ifndef EXP_009_HUB75_HARDWARE_H
#define EXP_009_HUB75_HARDWARE_H

/*
 * Hardware configuration for Experiment 009.
 *
 * Original ESP32-WROOM-32 wiring comes from the previously working
 * InstructionSender implementation. The physical chain-entry corner is
 * described as seen from the BACK of the complete 160x64 display.
 */

#define HUB75_PIN_R1  18
#define HUB75_PIN_G1  26
#define HUB75_PIN_B1  27
#define HUB75_PIN_R2  14
#define HUB75_PIN_G2  12
#define HUB75_PIN_B2  13

#define HUB75_PIN_A   23
#define HUB75_PIN_B   19
#define HUB75_PIN_C    5
#define HUB75_PIN_D   17
#define HUB75_PIN_E   -1
#define HUB75_PIN_CLK 16
#define HUB75_PIN_LAT  4
#define HUB75_PIN_OE  15

#define HUB75_PANEL_WIDTH        64
#define HUB75_PANEL_HEIGHT       32
#define HUB75_PANEL_COUNT         5
#define HUB75_LAYOUT_ROWS         5
#define HUB75_LAYOUT_COLS         1

/* Match the previously working ESP32-WROOM timing for first qualification. */
#define HUB75_INITIAL_BRIGHTNESS 64
#define HUB75_MIN_REFRESH_RATE   60

typedef enum {
    HUB75_CHAIN_ENTRY_BACK_LEFT_BOTTOM = 0,
    HUB75_CHAIN_ENTRY_BACK_LEFT_TOP,
    HUB75_CHAIN_ENTRY_BACK_RIGHT_TOP,
    HUB75_CHAIN_ENTRY_BACK_RIGHT_BOTTOM
} Hub75ChainEntryCorner;

/*
 * Current physical hardware stays unchanged for first bring-up.
 * A future controller/P1 move to the back-right-top is represented here so
 * it remains a hardware-adapter change rather than a framebuffer change.
 */
#define HUB75_CHAIN_ENTRY_CURRENT HUB75_CHAIN_ENTRY_BACK_LEFT_BOTTOM
#define HUB75_CHAIN_ENTRY_FUTURE  HUB75_CHAIN_ENTRY_BACK_RIGHT_TOP

/*
 * Each electrical panel is 64x32 and mounted in portrait orientation, so it
 * contributes 32x64 logical pixels. Five panels therefore form 160x64.
 *
 * E is not driven; it was tied to GND in the known-working installation.
 * GPIO12 is an ESP32 strapping pin and must not be externally forced HIGH
 * during reset on a normal 3.3V-flash WROOM module.
 */
#if HUB75_PANEL_WIDTH * HUB75_PANEL_HEIGHT * HUB75_PANEL_COUNT != 160 * 64
#error "The HUB75 panel pixel count no longer matches the logical framebuffer"
#endif

#endif
