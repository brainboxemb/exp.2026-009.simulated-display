#include "hub75_output.h"

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <esp_heap_caps.h>

#include "hub75_geometry.h"

#include <new>

namespace {

constexpr uint8_t kInitialBrightness = 24; // ~9.4%, proven on five panels.
MatrixPanel_I2S_DMA *panel = nullptr;

} // namespace

extern "C" bool hub75_output_init(void) {
    if (panel != nullptr) {
        return true;
    }

    HUB75_I2S_CFG config(
        hub75_geometry::kPanelWidth,
        hub75_geometry::kPanelHeight,
        hub75_geometry::kPanelCount);

    // Known-working wiring from Experiment 012.
    config.gpio.r1 = 18;
    config.gpio.g1 = 26;
    config.gpio.b1 = 27;
    config.gpio.r2 = 14;
    config.gpio.g2 = 12;
    config.gpio.b2 = 13;
    config.gpio.a = 23;
    config.gpio.b = 19;
    config.gpio.c = 5;
    config.gpio.d = 17;
    config.gpio.e = -1;
    config.gpio.lat = 4;
    config.gpio.oe = 15;
    config.gpio.clk = 16;

    config.i2sspeed = HUB75_I2S_CFG::HZ_10M;
    config.clkphase = false;
    config.double_buff = false;

    Serial.printf("HUB75 DMA-capable free before=%u\n",
                  unsigned(heap_caps_get_free_size(MALLOC_CAP_DMA)));

    panel = new (std::nothrow) MatrixPanel_I2S_DMA(config);
    if (panel == nullptr || !panel->begin()) {
        Serial.println("ERROR: HUB75 DMA initialization failed");
        delete panel;
        panel = nullptr;
        return false;
    }

    panel->setBrightness8(kInitialBrightness);
    Serial.printf("HUB75 5x64x32 ready, 160x64 RGB565, 10 MHz, OE=%u/255\n",
                  unsigned(kInitialBrightness));
    Serial.printf("HUB75 DMA-capable free after=%u\n",
                  unsigned(heap_caps_get_free_size(MALLOC_CAP_DMA)));
    return true;
}

extern "C" bool hub75_output_present(const MatrixFramebuffer *framebuffer) {
    if (panel == nullptr || framebuffer == nullptr) {
        return false;
    }

    static_assert(sizeof(MatrixPixel) == sizeof(uint16_t),
                  "Frame buffer must contain RGB565 pixels");
    static_assert(MATRIX_DISPLAY_WIDTH == hub75_geometry::kLogicalWidth,
                  "Logical display width must match HUB75 mapping");
    static_assert(MATRIX_DISPLAY_HEIGHT == hub75_geometry::kLogicalHeight,
                  "Logical display height must match HUB75 mapping");

    for (uint16_t y = 0; y < MATRIX_DISPLAY_HEIGHT; ++y) {
        for (uint16_t x = 0; x < MATRIX_DISPLAY_WIDTH; ++x) {
            hub75_geometry::Point mapped{};
            if (hub75_geometry::logicalToDma(x, y, mapped)) {
                panel->drawPixel(
                    mapped.x, mapped.y,
                    framebuffer->pixels[size_t(y) * MATRIX_DISPLAY_WIDTH + x]);
            }
        }
    }
    return true;
}

extern "C" void hub75_output_set_brightness(uint8_t brightness) {
    if (panel != nullptr) {
        panel->setBrightness8(brightness);
    }
}

extern "C" void hub75_output_shutdown(void) {
    delete panel;
    panel = nullptr;
}
