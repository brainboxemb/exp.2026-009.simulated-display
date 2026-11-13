#include <Arduino.h>

#include "hub75_output.h"
#include "matrix_framebuffer.h"
#include "matrix_serial.h"
#include "matrix_startup_screen.h"

#include <stddef.h>
#include <stdint.h>

#ifndef MATRIX_SOFTWARE_VERSION
#define MATRIX_SOFTWARE_VERSION "v0.1.0"
#endif
#ifndef MATRIX_DISPLAY_ID
#define MATRIX_DISPLAY_ID 'A'
#endif
#ifndef MATRIX_SSID_ID
#define MATRIX_SSID_ID 'A'
#endif

namespace {

constexpr uint32_t kBaudRate = 115200;
constexpr uint32_t kSerialTickMs = 20;
constexpr size_t kSerialChunk = 256;

MatrixFramebuffer framebuffer{};
MatrixSerialDisplay serialDisplay{};

const MatrixStartupInfo startupInfo = {
    MATRIX_SOFTWARE_VERSION, MATRIX_DISPLAY_ID, MATRIX_SSID_ID
};

bool hub75Ready = false;
bool startupReady = false;
uint32_t nextTick = 0;

uint32_t frameHash() {
    uint32_t hash = UINT32_C(2166136261);
    for (size_t i = 0; i < MATRIX_DISPLAY_PIXEL_COUNT; ++i) {
        const uint16_t pixel = framebuffer.pixels[i];
        hash = (hash ^ uint8_t(pixel)) * UINT32_C(16777619);
        hash = (hash ^ uint8_t(pixel >> 8)) * UINT32_C(16777619);
    }
    return hash;
}

uint8_t driverBrightness(uint8_t protocolBrightness) {
    return uint8_t((uint16_t(protocolBrightness) * 255u + 49u) / 99u);
}

bool presentFrame(const char *reason, bool verbose) {
    const uint32_t started = micros();
    if (hub75Ready && !hub75_output_present(&framebuffer)) {
        Serial.println("ERROR: HUB75 frame transfer failed");
        return false;
    }

    if (verbose) {
        Serial.printf("%s hash=%08lx hub75_us=%lu\n",
                      reason, (unsigned long)frameHash(),
                      (unsigned long)(micros() - started));
    }
    return true;
}

bool presentSerial(const char *reason, bool verbose) {
    const uint32_t composeStarted = micros();
    matrix_serial_compose(&serialDisplay, &framebuffer);
    const uint32_t composeUs = micros() - composeStarted;

    const uint8_t setting = matrix_serial_brightness(&serialDisplay);
    if (hub75Ready) {
        hub75_output_set_brightness(driverBrightness(setting));
    }
    if (!presentFrame(reason, verbose)) {
        return false;
    }

    if (verbose) {
        Serial.printf("compose_us=%lu brightness=%u/99 -> OE=%u/255\n",
                      (unsigned long)composeUs, unsigned(setting),
                      unsigned(driverBrightness(setting)));
    }
    return true;
}

bool showStartup() {
    if (!matrix_startup_sequence_render(&framebuffer, &startupInfo,
                                        MATRIX_STARTUP_BOOT, 0u)) {
        Serial.println("ERROR: startup rendering failed");
        return false;
    }
    if (!presentFrame("startup", true)) {
        return false;
    }
    Serial.printf("Startup %s ID=%c SSID=%c\n",
                  startupInfo.version, startupInfo.display_id,
                  startupInfo.ssid_id);
    return true;
}

void readSerial() {
    uint8_t bytes[kSerialChunk];
    bool received = false;
    bool valid = true;

    // Feed the same bytes into the same portable C parser used by JavaFX.
    while (Serial.available() > 0) {
        size_t n = 0;
        while (n < sizeof(bytes) && Serial.available() > 0) {
            const int byte = Serial.read();
            if (byte < 0) {
                break;
            }
            bytes[n++] = uint8_t(byte);
        }
        if (n == 0) {
            break;
        }

        received = true;
        if (!matrix_serial_feed(&serialDisplay, bytes, n)) {
            valid = false;
        }
    }

    if (received && !valid) {
        Serial.println("WARN: Matrix serial input contained an invalid command");
    }
    if (received && valid && !matrix_serial_update_active(&serialDisplay)) {
        presentSerial("serial", true);
    }
}

} // namespace

void setup() {
    Serial.begin(kBaudRate);
    delay(200);
    Serial.println("Experiment 009: Arduino + LVGL + HUB75 (5 panels)");

    hub75Ready = hub75_output_init();
    if (!hub75Ready) {
        Serial.println("WARN: HUB75 unavailable; serial parser remains active");
    }

    matrix_serial_init(&serialDisplay);
    startupReady = showStartup();
    if (!startupReady) {
        Serial.println("ERROR: startup frame unavailable");
    }

    nextTick = millis() + kSerialTickMs;
    Serial.printf("Arduino framework; free heap=%lu\n",
                  (unsigned long)ESP.getFreeHeap());
}

void loop() {
    if (!startupReady) {
        delay(1000);
        return;
    }

    readSerial();

    const uint32_t now = millis();
    if (int32_t(now - nextTick) >= 0) {
        // Keep sprite animation on the same 20-ms cadence as the simulator.
        nextTick = now + kSerialTickMs;
        if (!matrix_serial_update_active(&serialDisplay)
                && matrix_serial_tick(&serialDisplay)) {
            presentSerial("sprite", false);
        }
    }
    delay(1);
}
