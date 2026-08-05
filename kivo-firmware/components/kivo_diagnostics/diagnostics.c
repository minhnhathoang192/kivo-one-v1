#include "diagnostics.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "app_version.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "KIVO_DIAGNOSTICS";

/*
 * Chuyển nguyên nhân reset thành chuỗi dễ đọc.
 */
static const char *reset_reason_to_string(
    esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_UNKNOWN:
        return "UNKNOWN";

    case ESP_RST_POWERON:
        return "POWER_ON";

    case ESP_RST_EXT:
        return "EXTERNAL_RESET";

    case ESP_RST_SW:
        return "SOFTWARE_RESET";

    case ESP_RST_PANIC:
        return "PANIC";

    case ESP_RST_INT_WDT:
        return "INTERRUPT_WATCHDOG";

    case ESP_RST_TASK_WDT:
        return "TASK_WATCHDOG";

    case ESP_RST_WDT:
        return "OTHER_WATCHDOG";

    case ESP_RST_DEEPSLEEP:
        return "DEEP_SLEEP_WAKEUP";

    case ESP_RST_BROWNOUT:
        return "BROWNOUT";

    case ESP_RST_SDIO:
        return "SDIO_RESET";

    case ESP_RST_USB:
        return "USB_RESET";

    case ESP_RST_JTAG:
        return "JTAG_RESET";

    case ESP_RST_EFUSE:
        return "EFUSE_ERROR";

    case ESP_RST_PWR_GLITCH:
        return "POWER_GLITCH";

    case ESP_RST_CPU_LOCKUP:
        return "CPU_LOCKUP";

    default:
        return "UNRECOGNIZED";
    }
}

/*
 * Reset nào được xem là cảnh báo.
 */
static bool reset_reason_is_warning(
    esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
    case ESP_RST_BROWNOUT:
    case ESP_RST_PWR_GLITCH:
        return true;

    default:
        return false;
    }
}

/*
 * Reset nào được xem là nghiêm trọng.
 */
static bool reset_reason_is_fatal(
    esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_CPU_LOCKUP:
    case ESP_RST_EFUSE:
        return true;

    default:
        return false;
    }
}

const char *kivo_diagnostics_status_to_string(
    kivo_diag_status_t status)
{
    switch (status)
    {
    case KIVO_DIAG_STATUS_OK:
        return "OK";

    case KIVO_DIAG_STATUS_WARNING:
        return "WARNING";

    case KIVO_DIAG_STATUS_FATAL:
        return "FATAL";

    default:
        return "UNKNOWN";
    }
}

esp_err_t kivo_diagnostics_run_boot_checks(
    kivo_boot_report_t *report)
{
    if (report == NULL)
    {
        ESP_LOGE(TAG, "Boot report pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    memset(report, 0, sizeof(*report));

    report->product_name = KIVO_PRODUCT_NAME;
    report->firmware_version = KIVO_FIRMWARE_VERSION;
    report->target = KIVO_BUILD_TARGET;

    /*
     * Kiểm tra nguyên nhân reset.
     */
    const esp_reset_reason_t reset_reason =
        esp_reset_reason();

    report->reset_reason =
        reset_reason_to_string(reset_reason);

    /*
     * Đọc thông tin CPU/chip.
     */
    esp_chip_info_t chip_info = {0};
    esp_chip_info(&chip_info);

    report->cpu_cores =
        (uint32_t)chip_info.cores;

    /*
     * ESP32-S3 mặc định thường chạy tối đa 240 MHz.
     * Iteration sau có thể đọc trực tiếp cấu hình PM/clock.
     */
    report->cpu_frequency_mhz = 240U;

    /*
     * Đọc kích thước flash vật lý.
     */
    uint32_t flash_size = 0U;

    const esp_err_t flash_result =
        esp_flash_get_size(NULL, &flash_size);

    if (flash_result == ESP_OK)
    {
        report->flash_size_bytes =
            (size_t)flash_size;
    }
    else
    {
        report->flash_size_bytes = 0U;

        ESP_LOGE(
            TAG,
            "Cannot read flash size: %s",
            esp_err_to_name(flash_result));
    }

    /*
     * Heap nội RAM.
     */
    report->internal_heap_free_bytes =
        heap_caps_get_free_size(
            MALLOC_CAP_INTERNAL |
            MALLOC_CAP_8BIT);

    report->internal_heap_minimum_bytes =
        heap_caps_get_minimum_free_size(
            MALLOC_CAP_INTERNAL |
            MALLOC_CAP_8BIT);

    /*
     * Kiểm tra PSRAM qua heap capabilities.
     *
     * Không gọi esp_psram_is_initialized() để tránh phụ thuộc
     * API/linker thay đổi giữa các phiên bản ESP-IDF.
     */
    report->psram_total_bytes =
        heap_caps_get_total_size(
            MALLOC_CAP_SPIRAM);

    report->psram_free_bytes =
        heap_caps_get_free_size(
            MALLOC_CAP_SPIRAM);

    report->psram_available =
        report->psram_total_bytes > 0U;

    /*
     * Iteration hiện tại:
     * - Board cơ bản được xem là OK nếu đọc được flash.
     * - Display chưa khởi tạo nên để WARNING.
     *
     * Khi viết display_service, trạng thái display sẽ được cập nhật
     * bằng kết quả thực tế của display_service_init().
     */
    report->board_status =
        flash_result == ESP_OK
            ? KIVO_DIAG_STATUS_OK
            : KIVO_DIAG_STATUS_FATAL;

    report->display_status =
        KIVO_DIAG_STATUS_WARNING;

    report->overall_status =
        KIVO_DIAG_STATUS_OK;

    if (reset_reason_is_fatal(reset_reason))
    {
        report->overall_status =
            KIVO_DIAG_STATUS_FATAL;
    }
    else if (
        reset_reason_is_warning(reset_reason) ||
        !report->psram_available ||
        report->display_status == KIVO_DIAG_STATUS_WARNING)
    {
        report->overall_status =
            KIVO_DIAG_STATUS_WARNING;
    }

    if (report->board_status == KIVO_DIAG_STATUS_FATAL)
    {
        report->overall_status =
            KIVO_DIAG_STATUS_FATAL;
    }

    ESP_LOGI(TAG, "Boot diagnostics completed");

    return ESP_OK;
}

void kivo_diagnostics_print_boot_report(
    const kivo_boot_report_t *report)
{
    if (report == NULL)
    {
        ESP_LOGE(TAG, "Cannot print NULL boot report");
        return;
    }

    ESP_LOGI(
        TAG,
        "========================================");

    ESP_LOGI(
        TAG,
        "%s - BOOT DIAGNOSTICS",
        report->product_name);

    ESP_LOGI(
        TAG,
        "Firmware: %s",
        report->firmware_version);

    ESP_LOGI(
        TAG,
        "Target: %s",
        report->target);

    ESP_LOGI(
        TAG,
        "Reset reason: %s",
        report->reset_reason);

    ESP_LOGI(
        TAG,
        "CPU cores: %" PRIu32,
        report->cpu_cores);

    ESP_LOGI(
        TAG,
        "CPU frequency: %" PRIu32 " MHz",
        report->cpu_frequency_mhz);

    ESP_LOGI(
        TAG,
        "Flash size: %.2f MB",
        (double)report->flash_size_bytes /
            (1024.0 * 1024.0));

    ESP_LOGI(
        TAG,
        "Internal heap free: %u bytes",
        (unsigned int)
            report->internal_heap_free_bytes);

    ESP_LOGI(
        TAG,
        "Internal heap minimum: %u bytes",
        (unsigned int)
            report->internal_heap_minimum_bytes);

    if (report->psram_available)
    {
        ESP_LOGI(
            TAG,
            "PSRAM total: %.2f MB",
            (double)report->psram_total_bytes /
                (1024.0 * 1024.0));

        ESP_LOGI(
            TAG,
            "PSRAM free: %u bytes",
            (unsigned int)
                report->psram_free_bytes);
    }
    else
    {
        ESP_LOGW(
            TAG,
            "PSRAM: NOT AVAILABLE");
    }

    ESP_LOGI(
        TAG,
        "Board init: %s",
        kivo_diagnostics_status_to_string(
            report->board_status));

    ESP_LOGI(
        TAG,
        "Display service: %s",
        kivo_diagnostics_status_to_string(
            report->display_status));

    ESP_LOGI(
        TAG,
        "Overall status: %s",
        kivo_diagnostics_status_to_string(
            report->overall_status));

    ESP_LOGI(
        TAG,
        "========================================");
}

bool kivo_diagnostics_has_fatal_error(
    const kivo_boot_report_t *report)
{
    if (report == NULL)
    {
        return true;
    }

    return report->overall_status ==
           KIVO_DIAG_STATUS_FATAL;
}