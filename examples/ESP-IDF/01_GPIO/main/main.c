/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   GPIO / IO Extension Test
 ******************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "gpio.h"
#include "i2c.h"
#include "io_extension.h"

static const char *TAG = "01_GPIO";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing I2C bus and IO_EXTENSION...");
    DEV_I2C_Init();
    IO_EXTENSION_Init();

    ESP_LOGI(TAG, "Starting GPIO and IO Extension Toggling...");

    uint8_t state = 0;
    while (1) {
        state = !state;
        IO_EXTENSION_Output(0, state);
        ESP_LOGI(TAG, "IO_EXTENSION Pin 0 state: %d", state);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
