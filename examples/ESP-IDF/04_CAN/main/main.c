/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   TWAI (CAN bus) Transmission and Reception Test
 ******************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "twai.h"

static const char *TAG = "04_CAN";

#define TX_GPIO_NUM 20
#define RX_GPIO_NUM 19

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing CAN (TWAI) Driver...");
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (can_init(t_config, f_config, g_config) != ESP_OK) {
        ESP_LOGE(TAG, "CAN init failed!");
        return;
    }

    ESP_LOGI(TAG, "Transmitting test CAN frame...");
    twai_message_t tx_msg = {
        .identifier = 0x123,
        .data_length_code = 8,
        .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
    };
    can_write_Byte(tx_msg);

    while (1) {
        can_read_alerts();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
