#ifndef _LVGL_PORT_H_
#define _LVGL_PORT_H_

#include <stdbool.h>
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "touch.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the LVGL porting layer.
 * @param panel_handle Handle to the LCD panel
 * @param tp_handle Handle to the touch panel
 * @return ESP_OK on success
 */
esp_err_t lvgl_port_init(esp_lcd_panel_handle_t panel_handle, esp_lcd_touch_handle_t tp_handle);

/**
 * @brief Lock the LVGL mutex for thread safety.
 * @param timeout_ms Timeout in milliseconds (-1 for indefinite wait)
 * @return true if mutex was locked successfully, false otherwise
 */
bool lvgl_port_lock(int timeout_ms);

/**
 * @brief Unlock the LVGL mutex.
 */
void lvgl_port_unlock(void);

/**
 * @brief Notify LVGL port of RGB VSYNC event.
 * @return true if higher priority task was woken, false otherwise
 */
bool lvgl_port_notify_rgb_vsync(void);

#ifdef __cplusplus
}
#endif

#endif // _LVGL_PORT_H_
