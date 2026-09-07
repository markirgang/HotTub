/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   LVGL v8 Demo Test
 ******************************************************************************/
#include "rgb_lcd_port.h"
#include "gt911.h"
#include "lvgl_port.h"
#include "lvgl.h"
#include "demos/lv_demos.h"

static const char *TAG = "13_lvgl_v8_demo";

void app_main(void)
{
    static esp_lcd_panel_handle_t panel_handle = NULL;
    static esp_lcd_touch_handle_t tp_handle = NULL;

    tp_handle = touch_gt911_init();
    panel_handle = waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();

    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    ESP_LOGI(TAG, "Displaying LVGL v8 Widgets Demo");
    if (lvgl_port_lock(-1)) {
        lv_demo_widgets();
        lvgl_port_unlock();
    }
}
