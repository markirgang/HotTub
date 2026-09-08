/*****************************************************************************
 * Smart Hot Tub Controller Firmware for Waveshare ESP32-S3 Touch LCD 7" / 7B
 ******************************************************************************/
#include "rgb_lcd_port.h" // Header for Waveshare RGB LCD driver
#include "gt911.h"        // Header for touch screen operations (GT911)
#include "lvgl_port.h"    // Header for LVGL port initialization and locking
#include "hal_hottub.h"
#include "config_manager.h"
#include "spa_controller.h"
#include "ui_hottub.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN_HOTTUB";

static void ui_periodic_update_task(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (lvgl_port_lock(-1)) {
            ui_hottub_update();
            lvgl_port_unlock();
        }
    }
}

void app_main()
{
    ESP_LOGI(TAG, "Starting Smart Hot Tub Controller Firmware...");

    // 1. Initialize NVS Flash
    ESP_ERROR_CHECK(config_manager_init());

    // 2. Initialize Spa Controller & Safety Interlocks Core
    ESP_ERROR_CHECK(spa_controller_init());

    // 3. Initialize Display & GT911 Touch Screen Drivers
    static esp_lcd_panel_handle_t panel_handle = NULL;
    static esp_lcd_touch_handle_t tp_handle = NULL;

    tp_handle = touch_gt911_init();
    panel_handle = waveshare_esp32_s3_rgb_lcd_init();

    // Turn on LCD backlight
    wavesahre_rgb_lcd_bl_on();

    // 4. Initialize LVGL Graphics Engine Port
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    // 5. Build 800x480 Hot Tub Touchscreen UI
    if (lvgl_port_lock(-1)) {
        ui_hottub_init();
        lvgl_port_unlock();
    }

    // 6. Launch periodic UI refresh task
    xTaskCreate(ui_periodic_update_task, "ui_update_task", 4 * 1024, NULL, 4, NULL);

    ESP_LOGI(TAG, "Smart Hot Tub Controller fully running.");
}