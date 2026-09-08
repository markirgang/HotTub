#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HEAT_MODE_STANDARD = 0, // Heats whenever temperature drops below setpoint
    HEAT_MODE_ECONOMY  = 1, // Heats only during active filtration cycles
    HEAT_MODE_SLEEP    = 2  // Maintains temp 20°F below setpoint to save energy
} heat_mode_t;

typedef struct {
    float setpoint_f;                 // Target temperature (Fahrenheit, 80°F to 104°F)
    bool is_celsius;                  // Temperature display scale toggle
    uint8_t heat_mode;                // Standard, Economy, or Sleep mode
    uint8_t filter_start_hour_1;      // Filter Cycle 1 start hour (0-23)
    uint8_t filter_duration_hours_1;  // Filter Cycle 1 duration (1-12 hours)
    uint8_t filter_start_hour_2;      // Filter Cycle 2 start hour (0-23)
    uint8_t filter_duration_hours_2;  // Filter Cycle 2 duration (1-12 hours)
    uint16_t jet_timeout_mins;        // Auto-off timer for High Jets (1-60 mins)
    uint16_t blower_timeout_mins;     // Auto-off timer for Air Blower (1-60 mins)
    float temp_offset_f;              // Sensor calibration offset (°F)
} hottub_config_t;

/**
 * @brief Initialize NVS flash storage for persistent configuration.
 */
esp_err_t config_manager_init(void);

/**
 * @brief Load hot tub settings from NVS.
 */
esp_err_t config_manager_load(hottub_config_t *config);

/**
 * @brief Save hot tub settings to NVS.
 */
esp_err_t config_manager_save(const hottub_config_t *config);

/**
 * @brief Populate config structure with factory defaults.
 */
void config_manager_get_defaults(hottub_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_MANAGER_H
