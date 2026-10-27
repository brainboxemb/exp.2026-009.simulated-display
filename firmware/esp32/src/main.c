#include "hub75_output.h"
#include "matrix_framebuffer.h"
#include "matrix_serial.h"
#include "matrix_startup_screen.h"

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SERIAL_UART UART_NUM_0
#define SERIAL_BAUD_RATE 115200
#define SERIAL_RX_BUFFER_SIZE 2048
#define SERIAL_READ_CHUNK_SIZE 256
#define SERIAL_TICK_MS 20

static const char *TAG = "matrix-esp32";

static MatrixFramebuffer framebuffer;
static MatrixSerialDisplay serial_display;


static uint32_t frame_hash(void) {
    uint32_t hash = UINT32_C(2166136261);

    for (size_t index = 0u;
            index < MATRIX_DISPLAY_PIXEL_COUNT;
            index++) {
        const uint16_t pixel =
                framebuffer.pixels[index];

        hash =
                (hash ^ (uint8_t)pixel)
                * UINT32_C(16777619);
        hash =
                (hash ^ (uint8_t)(pixel >> 8u))
                * UINT32_C(16777619);
    }

    return hash;
}

static uint8_t driver_brightness(
        uint8_t serial_brightness) {
    return (uint8_t)(
            ((uint16_t)serial_brightness * 255u + 49u)
            / 99u);
}

static bool present_frame(
        bool hub75_ready,
        const char *reason,
        bool verbose) {
    int64_t hub75_us = 0;

    if (hub75_ready) {
        const int64_t output_start =
                esp_timer_get_time();

        if (!hub75_output_present(&framebuffer)) {
            ESP_LOGE(
                    TAG,
                    "HUB75 frame transfer failed");
            return false;
        }

        hub75_us =
                esp_timer_get_time()
                - output_start;
    }

    if (verbose) {
        ESP_LOGI(
                TAG,
                "%s hash=%08lx hub75_us=%lld",
                reason,
                (unsigned long)frame_hash(),
                (long long)hub75_us);
    } else {
        ESP_LOGD(
                TAG,
                "%s hash=%08lx hub75_us=%lld",
                reason,
                (unsigned long)frame_hash(),
                (long long)hub75_us);
    }

    return true;
}

static bool present_serial(
        bool hub75_ready,
        const char *reason,
        bool verbose) {
    const int64_t compose_start =
            esp_timer_get_time();

    matrix_serial_compose(
            &serial_display,
            &framebuffer);

    const int64_t compose_us =
            esp_timer_get_time()
            - compose_start;
    const uint8_t brightness =
            matrix_serial_brightness(
                    &serial_display);
    const uint8_t hardware_brightness =
            driver_brightness(brightness);

    if (hub75_ready) {
        hub75_output_set_brightness(
                hardware_brightness);
    }

    if (!present_frame(
            hub75_ready,
            reason,
            false)) {
        return false;
    }

    if (verbose) {
        ESP_LOGI(
                TAG,
                "serial compose_us=%lld brightness=%u/99 -> OE=%u/255",
                (long long)compose_us,
                (unsigned)brightness,
                (unsigned)hardware_brightness);
    }

    return true;
}

static bool show_startup_screen(
        bool hub75_ready) {
    const int64_t render_start =
            esp_timer_get_time();

    if (!matrix_startup_screen_render(
            &framebuffer,
            MATRIX_SOFTWARE_VERSION)) {
        ESP_LOGE(
                TAG,
                "Startup screen rendering failed");
        return false;
    }

    const int64_t render_us =
            esp_timer_get_time()
            - render_start;

    if (!present_frame(
            hub75_ready,
            "startup-screen",
            true)) {
        return false;
    }

    ESP_LOGI(
            TAG,
            "startup screen %s render_us=%lld; serial /UB.../UE can replace it atomically",
            MATRIX_SOFTWARE_VERSION,
            (long long)render_us);
    return true;
}

static bool serial_uart_init(void) {
    esp_err_t result =
            uart_driver_install(
                    SERIAL_UART,
                    SERIAL_RX_BUFFER_SIZE,
                    0,
                    0,
                    NULL,
                    0);

    if (result != ESP_OK
            && result != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(
                TAG,
                "UART0 driver install failed: %s",
                esp_err_to_name(result));
        return false;
    }

    result =
            uart_set_baudrate(
                    SERIAL_UART,
                    SERIAL_BAUD_RATE);

    if (result != ESP_OK) {
        ESP_LOGE(
                TAG,
                "UART0 baud configuration failed: %s",
                esp_err_to_name(result));
        return false;
    }

    ESP_LOGI(
            TAG,
            "matrix serial ready on UART0 at %d baud",
            SERIAL_BAUD_RATE);
    return true;
}

void app_main(void) {
    const bool hub75_ready =
            hub75_output_init();

    if (!hub75_ready) {
        ESP_LOGW(
                TAG,
                "HUB75 unavailable; continuing parser/renderer qualification without output");
    }

    matrix_serial_init(&serial_display);

    if (!show_startup_screen(hub75_ready)) {
        hub75_output_shutdown();
        return;
    }

    if (!serial_uart_init()) {
        ESP_LOGE(
                TAG,
                "Serial input unavailable; leaving startup image active");

        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    TickType_t last_wake =
            xTaskGetTickCount();
    uint8_t bytes[SERIAL_READ_CHUNK_SIZE];

    for (;;) {
        bool received = false;
        bool valid = true;

        for (;;) {
            const int length =
                    uart_read_bytes(
                            SERIAL_UART,
                            bytes,
                            sizeof(bytes),
                            0);

            if (length <= 0) {
                break;
            }

            received = true;

            if (!matrix_serial_feed(
                    &serial_display,
                    bytes,
                    (size_t)length)) {
                valid = false;
            }
        }

        if (received && !valid) {
            ESP_LOGW(
                    TAG,
                    "Matrix serial input contained an invalid command");
        }

        if (received
                && valid
                && !matrix_serial_update_active(
                        &serial_display)) {
            if (!present_serial(
                    hub75_ready,
                    "serial",
                    true)) {
                break;
            }
        }

        if (!matrix_serial_update_active(
                &serial_display)
                && matrix_serial_tick(
                        &serial_display)) {
            if (!present_serial(
                    hub75_ready,
                    "sprite",
                    false)) {
                break;
            }
        }

        vTaskDelayUntil(
                &last_wake,
                pdMS_TO_TICKS(
                        SERIAL_TICK_MS));
    }

    hub75_output_shutdown();
}
