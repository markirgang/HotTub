#ifndef UI_HOTTUB_H
#define UI_HOTTUB_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LVGL 8.3 800x480 Hot Tub UI widgets and layout.
 */
void ui_hottub_init(void);

/**
 * @brief Periodic UI refresh tick (called from LVGL task or main loop).
 */
void ui_hottub_update(void);

#ifdef __cplusplus
}
#endif

#endif // UI_HOTTUB_H
