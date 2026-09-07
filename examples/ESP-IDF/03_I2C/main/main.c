/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   I2C Bus Scanner Test
 ******************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c.h"
#include "io_extension.h"

static const char *TAG = "03_I2C";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing I2C driver...");
    DEV_I2C_Init();

    ESP_LOGI(TAG, "Scanning I2C bus for active devices...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        uint8_t dummy = 0;
        uint8_t res = DEV_I2C_Read_Nbyte(addr, 0, &dummy, 1);
        if (res == 0) {
            ESP_LOGI(TAG, "Found I2C device at address: 0x%02X", addr);
        }
    }

    ESP_LOGI(TAG, "Initializing IO_EXTENSION chip...");
    IO_EXTENSION_Init();
    ESP_LOGI(TAG, "I2C Test finished.");
}
