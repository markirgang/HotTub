/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   BMP Image Display Test
 ******************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rgb_lcd_port.h"
#include "gui_paint.h"
#include "gui_bmp.h"
#include "sd.h"
#include "i2c.h"
#include "io_extension.h"

static const char *TAG = "09_Display_bmp";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing hardware...");
    DEV_I2C_Init();
    IO_EXTENSION_Init();

    waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();

    ESP_LOGI(TAG, "Initializing SD card for BMP reading...");
    if (sd_mmc_init() == ESP_OK) {
        ESP_LOGI(TAG, "Loading /sdcard/pic.bmp...");
        GUI_ReadBmp(0, 0, "/sdcard/pic.bmp");
    } else {
        ESP_LOGW(TAG, "SD card mount failed - ensure SD card with BMP file is inserted.");
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
