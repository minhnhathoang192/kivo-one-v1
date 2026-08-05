#pragma once

/*
 * KIVO ONE V1 pin map
 *
 * Giá trị -1 nghĩa là chân chưa được cấu hình.
 * Khi chốt sơ đồ dây, thay -1 bằng GPIO thật.
 */

/* Display SPI */
#define KIVO_PIN_DISPLAY_MOSI (-1)
#define KIVO_PIN_DISPLAY_SCLK (-1)
#define KIVO_PIN_DISPLAY_CS (-1)
#define KIVO_PIN_DISPLAY_DC (-1)
#define KIVO_PIN_DISPLAY_RST (-1)
#define KIVO_PIN_DISPLAY_BACKLIGHT (-1)

/* Touch controller */
#define KIVO_PIN_TOUCH_SDA (-1)
#define KIVO_PIN_TOUCH_SCL (-1)
#define KIVO_PIN_TOUCH_INT (-1)
#define KIVO_PIN_TOUCH_RST (-1)

/* INMP441 microphone */
#define KIVO_PIN_MIC_BCLK (-1)
#define KIVO_PIN_MIC_WS (-1)
#define KIVO_PIN_MIC_DATA (-1)

/* MAX98357A speaker amplifier */
#define KIVO_PIN_SPK_BCLK (-1)
#define KIVO_PIN_SPK_WS (-1)
#define KIVO_PIN_SPK_DATA (-1)
#define KIVO_PIN_SPK_ENABLE (-1)

/* ESP32-WROOM Bluetooth coprocessor UART */
#define KIVO_PIN_BT_UART_TX (-1)
#define KIVO_PIN_BT_UART_RX (-1)
#define KIVO_PIN_BT_STATUS (-1)

/* Servos */
#define KIVO_PIN_SERVO_NECK (-1)
#define KIVO_PIN_SERVO_LEFT_ARM (-1)
#define KIVO_PIN_SERVO_RIGHT_ARM (-1)
#define KIVO_PIN_SERVO_POWER_ENABLE (-1)

/* Physical controls */
#define KIVO_PIN_BUTTON_ACTION (-1)
#define KIVO_PIN_BUTTON_MODE (-1)
#define KIVO_PIN_BUTTON_BACK (-1)
#define KIVO_PIN_BUTTON_SETUP (-1)

/* Power monitoring */
#define KIVO_PIN_BATTERY_ADC (-1)