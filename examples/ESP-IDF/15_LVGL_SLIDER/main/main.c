/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   LVGL Slider Demo
 ******************************************************************************/
#include "rgb_lcd_port.h"
#include "gt911.h"
#include "lvgl_port.h"
#include "lvgl.h"

static const char *TAG = "15_LVGL_SLIDER";

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    ESP_LOGI(TAG, "Slider Value: %ld", (long)val);
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
        lv_obj_t * slider = lv_slider_create(lv_scr_act());
        lv_obj_set_width(slider, 400);
        lv_obj_center(slider);
        lv_slider_set_range(slider, 0, 100);
        lv_slider_set_value(slider, 50, LV_ANIM_OFF);
        lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

        lvgl_port_unlock();
    }
}
