#include "hub75_output.h"

#include "hub75.h"
#include "hub75_hardware.h"

#include "esp_log.h"

#include <new>

namespace {

constexpr char TAG[] = "hub75-output";

Hub75Driver *driver = nullptr;

bool configure_layout(
        Hub75Config &config,
        Hub75ChainEntryCorner corner) {
    /*
     * The five 64x32 modules are modeled as the same 5x1 landscape stack as
     * the proven legacy VirtualMatrixPanel configuration and then rotated
     * 90 degrees clockwise to expose a 160x64 logical surface.
     *
     * Corner names below are from the BACK of the final display. Mirroring
     * to the LED/front side and undoing the 90-degree rotation yields the
     * corresponding esp-hub75 physical-layout origin.
     */
    config.rotation = Hub75Rotation::ROTATE_90;

    switch (corner) {
        case HUB75_CHAIN_ENTRY_BACK_LEFT_BOTTOM:
            config.layout = Hub75PanelLayout::TOP_RIGHT_DOWN;
            return true;

        case HUB75_CHAIN_ENTRY_BACK_LEFT_TOP:
            config.layout = Hub75PanelLayout::TOP_LEFT_DOWN;
            return true;

        case HUB75_CHAIN_ENTRY_BACK_RIGHT_TOP:
            config.layout = Hub75PanelLayout::BOTTOM_LEFT_UP;
            return true;

        case HUB75_CHAIN_ENTRY_BACK_RIGHT_BOTTOM:
            config.layout = Hub75PanelLayout::BOTTOM_RIGHT_UP;
            return true;

        default:
            return false;
    }
}

Hub75Config create_config(void) {
    Hub75Config config{};

    config.panel_width = HUB75_PANEL_WIDTH;
    config.panel_height = HUB75_PANEL_HEIGHT;
    config.scan_wiring = Hub75ScanWiring::STANDARD_TWO_SCAN;
    config.shift_driver = Hub75ShiftDriver::GENERIC;

    config.layout_rows = HUB75_LAYOUT_ROWS;
    config.layout_cols = HUB75_LAYOUT_COLS;
    (void)configure_layout(config, HUB75_CHAIN_ENTRY_CURRENT);

    config.pins.r1 = HUB75_PIN_R1;
    config.pins.g1 = HUB75_PIN_G1;
    config.pins.b1 = HUB75_PIN_B1;
    config.pins.r2 = HUB75_PIN_R2;
    config.pins.g2 = HUB75_PIN_G2;
    config.pins.b2 = HUB75_PIN_B2;
    config.pins.a = HUB75_PIN_A;
    config.pins.b = HUB75_PIN_B;
    config.pins.c = HUB75_PIN_C;
    config.pins.d = HUB75_PIN_D;
    config.pins.e = HUB75_PIN_E;
    config.pins.lat = HUB75_PIN_LAT;
    config.pins.oe = HUB75_PIN_OE;
    config.pins.clk = HUB75_PIN_CLK;

    config.output_clock_speed = Hub75ClockSpeed::HZ_10M;
    config.min_refresh_rate = HUB75_MIN_REFRESH_RATE;
    config.double_buffer = false;
    config.clk_phase_inverted = false;
    config.brightness = HUB75_INITIAL_BRIGHTNESS;

    return config;
}

}  // namespace

extern "C" bool hub75_output_init(void) {
    if (driver != nullptr) {
        return true;
    }

    Hub75Config config = create_config();
    if (!configure_layout(config, HUB75_CHAIN_ENTRY_CURRENT)) {
        ESP_LOGE(TAG, "Unsupported chain-entry corner: %d",
                 (int)HUB75_CHAIN_ENTRY_CURRENT);
        return false;
    }

    driver = new (std::nothrow) Hub75Driver(config);
    if (driver == nullptr) {
        ESP_LOGE(TAG, "Unable to allocate HUB75 driver");
        return false;
    }

    if (!driver->begin()) {
        ESP_LOGE(TAG, "HUB75 driver initialization failed");
        delete driver;
        driver = nullptr;
        return false;
    }

    if (driver->get_width() != MATRIX_DISPLAY_WIDTH
            || driver->get_height() != MATRIX_DISPLAY_HEIGHT) {
        ESP_LOGE(TAG,
                 "Unexpected logical size: %ux%u (expected %ux%u)",
                 (unsigned)driver->get_width(),
                 (unsigned)driver->get_height(),
                 (unsigned)MATRIX_DISPLAY_WIDTH,
                 (unsigned)MATRIX_DISPLAY_HEIGHT);
        driver->end();
        delete driver;
        driver = nullptr;
        return false;
    }

    ESP_LOGI(TAG,
             "ready: %ux%u, 5x 64x32 portrait, 10 MHz, brightness=%u, chain-entry(back)=%d",
             (unsigned)driver->get_width(),
             (unsigned)driver->get_height(),
             (unsigned)HUB75_INITIAL_BRIGHTNESS,
             (int)HUB75_CHAIN_ENTRY_CURRENT);
    return true;
}

extern "C" bool hub75_output_present(
        const MatrixFramebuffer *framebuffer) {
    if (driver == nullptr || framebuffer == nullptr) {
        return false;
    }

    static_assert(
            sizeof(MatrixPixel) == 2,
            "RGB565 output requires 16-bit MatrixPixel");

    driver->draw_pixels(
            0,
            0,
            MATRIX_DISPLAY_WIDTH,
            MATRIX_DISPLAY_HEIGHT,
            reinterpret_cast<const uint8_t *>(framebuffer->pixels),
            Hub75PixelFormat::RGB565,
            Hub75ColorOrder::RGB,
            false);

    return true;
}

extern "C" void hub75_output_set_brightness(
        uint8_t brightness) {
    if (driver != nullptr) {
        driver->set_brightness(brightness);
    }
}

extern "C" void hub75_output_shutdown(void) {
    if (driver == nullptr) {
        return;
    }

    driver->end();
    delete driver;
    driver = nullptr;
}
