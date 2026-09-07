/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   RS485 Communication Test
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "usart.h"

static const char *TAG = "05_RS485";

#define RS485_TXD_PIN 43
#define RS485_RXD_PIN 44

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing RS485 Interface...");
    DEV_UART_Init(RS485_TXD_PIN, RS485_RXD_PIN, 115200);

    const char *msg = "Waveshare ESP32-S3 RS485 Test\r\n";
    UART_Write_Byte((uint8_t *)msg);

    uint8_t rx_buf[128];
    while (1) {
        int len = UART_Read_Byte(rx_buf, sizeof(rx_buf) - 1);
        if (len > 0) {
            rx_buf[len] = '\0';
            ESP_LOGI(TAG, "RS485 RX [%d bytes]: %s", len, (char *)rx_buf);
            UART_Write_Byte(rx_buf);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
