/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   RGB LCD Panel Driver Test
 ******************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rgb_lcd_port.h"

static const char *TAG = "06_LCD";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing Waveshare RGB LCD Display...");
    esp_lcd_panel_handle_t panel_handle = waveshare_esp32_s3_rgb_lcd_init();

    ESP_LOGI(TAG, "Turning on Backlight...");
    wavesahre_rgb_lcd_bl_on();

    ESP_LOGI(TAG, "RGB LCD display initialized successfully.");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
