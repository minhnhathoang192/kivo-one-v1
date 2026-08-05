#ifndef KIVO_DIAGNOSTICS_H
#define KIVO_DIAGNOSTICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        KIVO_DIAG_STATUS_OK = 0,
        KIVO_DIAG_STATUS_WARNING,
        KIVO_DIAG_STATUS_FATAL
    } kivo_diag_status_t;

    typedef struct
    {
        const char *product_name;
        const char *firmware_version;
        const char *target;

        const char *reset_reason;

        uint32_t cpu_cores;
        uint32_t cpu_frequency_mhz;

        size_t flash_size_bytes;

        size_t internal_heap_free_bytes;
        size_t internal_heap_minimum_bytes;

        size_t psram_total_bytes;
        size_t psram_free_bytes;
        bool psram_available;

        kivo_diag_status_t board_status;
        kivo_diag_status_t display_status;
        kivo_diag_status_t overall_status;
    } kivo_boot_report_t;

    /**
     * Chạy toàn bộ kiểm tra cơ bản khi KIVO khởi động.
     *
     * @param report Vùng nhớ nhận kết quả.
     * @return ESP_OK nếu đã chạy diagnostics thành công.
     */
    esp_err_t kivo_diagnostics_run_boot_checks(kivo_boot_report_t *report);

    /**
     * In báo cáo diagnostics ra Serial Monitor.
     */
    void kivo_diagnostics_print_boot_report(
        const kivo_boot_report_t *report);

    /**
     * Kiểm tra báo cáo có lỗi nghiêm trọng hay không.
     */
    bool kivo_diagnostics_has_fatal_error(
        const kivo_boot_report_t *report);

    /**
     * Chuyển trạng thái diagnostics thành chuỗi.
     */
    const char *kivo_diagnostics_status_to_string(
        kivo_diag_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* KIVO_DIAGNOSTICS_H */