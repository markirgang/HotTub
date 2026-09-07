/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   UART Serial Echo Test
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "usart.h"

static const char *TAG = "02_UART";

#define TXD_PIN 43
#define RXD_PIN 44

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing UART...");
    DEV_UART_Init(TXD_PIN, RXD_PIN, 115200);

    const char *test_str = "ESP32-S3 UART Echo Ready!\r\n";
    UART_Write_Byte((uint8_t *)test_str);

    uint8_t rx_buf[128];
    while (1) {
        int len = UART_Read_Byte(rx_buf, sizeof(rx_buf) - 1);
        if (len > 0) {
            rx_buf[len] = '\0';
            ESP_LOGI(TAG, "Received %d bytes: %s", len, (char *)rx_buf);
            UART_Write_Byte(rx_buf);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
