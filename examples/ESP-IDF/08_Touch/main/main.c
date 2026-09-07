/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   GT911 Touch Controller Coordinate Reading Test
 ******************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "gt911.h"

static const char *TAG = "08_Touch";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing GT911 Touch Controller...");
    esp_lcd_touch_handle_t tp_handle = touch_gt911_init();
    if (!tp_handle) {
        ESP_LOGE(TAG, "Failed to initialize touch controller!");
        return;
    }

    ESP_LOGI(TAG, "Touch controller ready. Polling for touch coordinates...");
    while (1) {
        touch_gt911_point_t pt = touch_gt911_read_point(5);
        if (pt.cnt > 0) {
            for (int i = 0; i < pt.cnt; i++) {
                ESP_LOGI(TAG, "Touch [%d/%d]: X=%d, Y=%d", i + 1, pt.cnt, pt.x[i], pt.y[i]);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
