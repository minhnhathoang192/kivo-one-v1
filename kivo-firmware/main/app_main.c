#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "board_init.h"
#include "diagnostics.h"

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "KIVO_MAIN";

#define KIVO_HEARTBEAT_INTERVAL_MS 5000U
#define KIVO_FULL_HEALTH_INTERVAL_COUNT 60U

static void run_safe_mode(void)
{
    ESP_LOGE(
        TAG,
        "Fatal boot condition detected");

    ESP_LOGE(
        TAG,
        "KIVO is entering SAFE MODE");

    ESP_LOGE(
        TAG,
        "Servo, arm and movement outputs remain disabled");

    while (true)
    {
        ESP_LOGW(
            TAG,
            "SAFE MODE heartbeat");

        vTaskDelay(
            pdMS_TO_TICKS(
                KIVO_HEARTBEAT_INTERVAL_MS));
    }
}

static void run_stable_main_loop(void)
{
    TickType_t last_wake_time =
        xTaskGetTickCount();

    uint32_t heartbeat_count = 0U;

    ESP_LOGI(
        TAG,
        "Entering stable main loop");

    while (true)
    {
        heartbeat_count++;

        ESP_LOGI(
            TAG,
            "Heartbeat=%" PRIu32,
            heartbeat_count);

        if (
            heartbeat_count %
                KIVO_FULL_HEALTH_INTERVAL_COUNT ==
            0U)
        {
            ESP_LOGI(
                TAG,
                "Periodic health checkpoint reached");
        }

        /*
         * vTaskDelayUntil giữ nhịp ổn định hơn vTaskDelay.
         * Vòng lặp không chạy chiếm 100% CPU.
         */
        vTaskDelayUntil(
            &last_wake_time,
            pdMS_TO_TICKS(
                KIVO_HEARTBEAT_INTERVAL_MS));
    }
}

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting KIVO ONE V1");

    /*
     * Bước 1: Khởi tạo phần board cơ bản.
     */
    const esp_err_t board_result =
        kivo_board_init();

    if (board_result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Board initialization failed: %s",
            esp_err_to_name(board_result));

        run_safe_mode();
        return;
    }

    /*
     * Bước 2: Chạy diagnostics.
     */
    kivo_boot_report_t boot_report = {0};

    const esp_err_t diagnostics_result =
        kivo_diagnostics_run_boot_checks(
            &boot_report);

    if (diagnostics_result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Diagnostics failed: %s",
            esp_err_to_name(diagnostics_result));

        run_safe_mode();
        return;
    }

    /*
     * Bước 3: In báo cáo boot.
     */
    kivo_diagnostics_print_boot_report(
        &boot_report);

    /*
     * Bước 4: Chặn hệ thống nếu có lỗi nghiêm trọng.
     */
    if (
        kivo_diagnostics_has_fatal_error(
            &boot_report))
    {
        run_safe_mode();
        return;
    }

    /*
     * Bước 5: Đi vào vòng lặp hoạt động ổn định.
     */
    run_stable_main_loop();
}