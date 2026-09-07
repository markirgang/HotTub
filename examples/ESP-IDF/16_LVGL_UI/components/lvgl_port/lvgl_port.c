#include "lvgl_port.h"
#include "rgb_lcd_port.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "lvgl_port";

static SemaphoreHandle_t lvgl_mutex = NULL;
static TaskHandle_t lvgl_task_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void lvgl_touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t touch_x[1];
    uint16_t touch_y[1];
    uint16_t touch_strength[1];
    uint8_t touch_cnt = 0;

    data->state = LV_INDEV_STATE_REL;

    if (touch_handle) {
        esp_lcd_touch_read_data(touch_handle);
        bool pressed = esp_lcd_touch_get_coordinates(touch_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);
        if (pressed && touch_cnt > 0) {
            data->point.x = touch_x[0];
            data->point.y = touch_y[0];
            data->state = LV_INDEV_STATE_PR;
        }
    }
}

static void lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = 10;
    while (1) {
        if (lvgl_port_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }
        if (task_delay_ms > 500) {
            task_delay_ms = 500;
        } else if (task_delay_ms < 1) {
            task_delay_ms = 1;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

static void lvgl_tick_cb(void *arg)
{
    lv_tick_inc(2);
}

esp_err_t lvgl_port_init(esp_lcd_panel_handle_t panel_handle, esp_lcd_touch_handle_t tp_handle)
{
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    if (!lvgl_mutex) {
        ESP_LOGE(TAG, "Create mutex failed");
        return ESP_FAIL;
    }

    touch_handle = tp_handle;
    lv_init();

    // Allocate frame buffers in PSRAM
    void *buf1 = NULL;
    void *buf2 = NULL;
    waveshare_get_frame_buffer(&buf1, &buf2);

    static lv_disp_draw_buf_t disp_buf;
    if (buf1 && buf2) {
        lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES);
    } else {
        lv_color_t *buf = heap_caps_malloc(EXAMPLE_LCD_H_RES * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
        lv_disp_draw_buf_init(&disp_buf, buf, NULL, EXAMPLE_LCD_H_RES * 40);
    }

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);

    if (touch_handle) {
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = lvgl_touch_read_cb;
        lv_indev_drv_register(&indev_drv);
    }

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lvgl_tick_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 2000));

    xTaskCreatePinnedToCore(lvgl_port_task, "LVGL", 8 * 1024, NULL, 4, &lvgl_task_handle, 1);

    return ESP_OK;
}

bool lvgl_port_lock(int timeout_ms)
{
    if (!lvgl_mutex) {
        return false;
    }
    TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xSemaphoreTakeRecursive(lvgl_mutex, timeout_ticks) == pdTRUE);
}

void lvgl_port_unlock(void)
{
    if (lvgl_mutex) {
        xSemaphoreGiveRecursive(lvgl_mutex);
    }
}

bool lvgl_port_notify_rgb_vsync(void)
{
    return false;
}
