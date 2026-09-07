/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   Micro SD Card Mount & Info Test
 ******************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c.h"
#include "io_extension.h"
#include "sd.h"

static const char *TAG = "07_SD";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing I2C bus and IO_EXTENSION...");
    DEV_I2C_Init();
    IO_EXTENSION_Init();

    ESP_LOGI(TAG, "Initializing SD Card MMC...");
    if (sd_mmc_init() == ESP_OK) {
        sd_card_print_info();
        size_t total_kb = 0, free_kb = 0;
        if (read_sd_capacity(&total_kb, &free_kb) == ESP_OK) {
            ESP_LOGI(TAG, "SD Card Total: %d MB, Free: %d MB", (int)(total_kb / 1024), (int)(free_kb / 1024));
        }
    } else {
        ESP_LOGE(TAG, "SD Card initialization failed!");
    }
}
