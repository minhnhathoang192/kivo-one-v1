#pragma once

#include <stdbool.h>
#include "esp_err.h"

typedef struct
{
    bool core_initialized;
    bool safe_outputs_applied;
    bool nvs_initialized;
} kivo_board_status_t;

/**
 * Khởi tạo lớp phần cứng cơ bản của KIVO.
 *
 * Chỉ đặt GPIO và hệ thống vào trạng thái an toàn.
 * Không khởi tạo display, audio, Wi-Fi hoặc servo driver ở đây.
 */
esp_err_t kivo_board_init(void);

/**
 * Lấy trạng thái board hiện tại.
 */
const kivo_board_status_t *kivo_board_get_status(void);