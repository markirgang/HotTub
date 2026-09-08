#ifndef SPA_CONTROLLER_H
#define SPA_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "config_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SPA_STATE_OFF = 0,
    SPA_STATE_STANDBY,
    SPA_STATE_HEATING_PURGE, // Circulating water for flow verification (10s)
    SPA_STATE_HEATING,       // Heater element energized
    SPA_STATE_COOLDOWN,      // Circulating pump flush after heating (30s)
    SPA_STATE_FILTERING,     // Filtration cycle running
    SPA_STATE_FREEZE_PROTECT,// Low temperature anti-freeze emergency heating
    SPA_STATE_ERROR_FLOW,    // Flow switch open error trip
    SPA_STATE_ERROR_OVERTEMP // Over-temperature 105°F safety lockout
} spa_state_t;

typedef struct {
    spa_state_t state;
    float current_temp_f;
    float current_temp_c;
    float setpoint_f;
    bool is_celsius;
    heat_mode_t heat_mode;
    
    bool flow_verified;
    uint8_t flow_purge_sec;
    uint8_t cooldown_sec;
    
    bool jet1_active;
    uint16_t jet1_timer_sec;
    bool jet2_active;
    uint16_t jet2_timer_sec;
    bool blower_active;
    uint16_t blower_timer_sec;
    bool light_active;
    bool ozone_active;
    
    bool is_filtering;
    bool freeze_active;
    char status_msg[64];
} spa_status_t;

/**
 * @brief Initialize safety controller state machine and background monitoring task.
 */
esp_err_t spa_controller_init(void);

/**
 * @brief Retrieve copy of current spa status.
 */
void spa_controller_get_status(spa_status_t *status);

/**
 * @brief Get active configuration reference.
 */
void spa_controller_get_config(hottub_config_t *config);

/**
 * @brief Update target setpoint temperature (°F).
 */
void spa_controller_set_setpoint(float setpoint_f);

/**
 * @brief Toggle High Jet Pump 1 state.
 */
void spa_controller_toggle_jet1(void);

/**
 * @brief Toggle High Jet Pump 2 state.
 */
void spa_controller_toggle_jet2(void);

/**
 * @brief Toggle Air Blower state.
 */
void spa_controller_toggle_blower(void);

/**
 * @brief Toggle Spa Light state.
 */
void spa_controller_toggle_light(void);

/**
 * @brief Change heating mode (Standard, Economy, Sleep).
 */
void spa_controller_set_heat_mode(heat_mode_t mode);

/**
 * @brief Change temperature scale toggle (°F / °C).
 */
void spa_controller_set_celsius(bool is_celsius);

#ifdef __cplusplus
}
#endif

#endif // SPA_CONTROLLER_H
