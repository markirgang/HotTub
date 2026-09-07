/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   LVGL Button Interaction Demo
 ******************************************************************************/
#include "rgb_lcd_port.h"
#include "gt911.h"
#include "lvgl_port.h"
#include "lvgl.h"

static const char *TAG = "14_LVGL_BTN";

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Clicked: %d", cnt);
        ESP_LOGI(TAG, "Button clicked: %d times", cnt);
    }
}

void app_main(void)
{
    static esp_lcd_panel_handle_t panel_handle = NULL;
    static esp_lcd_touch_handle_t tp_handle = NULL;

    tp_handle = touch_gt911_init();
    panel_handle = waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();

    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    if (lvgl_port_lock(-1)) {
        lv_obj_t * btn = lv_btn_create(lv_scr_act());
        lv_obj_set_size(btn, 200, 80);
        lv_obj_center(btn);
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text(label, "Click Me!");
        lv_obj_center(label);

        lvgl_port_unlock();
    }
}
