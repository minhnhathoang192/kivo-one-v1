#include "esp_check.h"

#include "board_init.h"

#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "KIVO_BOARD";

static kivo_board_status_t s_board_status = {
    .core_initialized = false,
    .safe_outputs_applied = false,
    .nvs_initialized = false,
};

static esp_err_t initialize_nvs(void)
{
    esp_err_t result = nvs_flash_init();

    if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
        result == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {

        ESP_LOGW(TAG, "NVS requires erase before initialization");

        ESP_RETURN_ON_ERROR(
            nvs_flash_erase(),
            TAG,
            "Failed to erase NVS");

        result = nvs_flash_init();
    }

    return result;
}

static void apply_safe_output_defaults(void)
{
    /*
     * Khi chốt GPIO thật:
     *
     * - tắt nguồn servo;
     * - tắt amplifier nếu có chân enable;
     * - tắt backlight trước khi display init;
     * - giữ motor lock;
     * - không phát xung PWM servo khi boot.
     *
     * Hiện chưa chốt pin nên chỉ ghi nhận trạng thái logic.
     */

    s_board_status.safe_outputs_applied = true;
    ESP_LOGI(TAG, "Safe output defaults applied");
}

esp_err_t kivo_board_init(void)
{
    ESP_LOGI(TAG, "Initializing KIVO board layer");

    apply_safe_output_defaults();

    esp_err_t result = initialize_nvs();

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "NVS initialization failed: %s",
            esp_err_to_name(result));

        return result;
    }

    s_board_status.nvs_initialized = true;
    s_board_status.core_initialized = true;

    ESP_LOGI(TAG, "Board layer initialized");

    return ESP_OK;
}

const kivo_board_status_t *kivo_board_get_status(void)
{
    return &s_board_status;
}