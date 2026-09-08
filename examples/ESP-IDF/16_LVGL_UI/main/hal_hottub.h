#ifndef HAL_HOTTUB_H
#define HAL_HOTTUB_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Relay ID Definitions
#define RELAY_HEATER        0
#define RELAY_CIRC_PUMP     1
#define RELAY_JET1          2
#define RELAY_JET2          3
#define RELAY_BLOWER        4
#define RELAY_LIGHT         5
#define RELAY_OZONE         6
#define RELAY_COUNT         7

// Hardware GPIO Mapping (Default for ESP32-S3 expansion header)
#define GPIO_RELAY_HEATER       8
#define GPIO_RELAY_CIRC_PUMP    9
#define GPIO_RELAY_JET1         10
#define GPIO_RELAY_JET2         11
#define GPIO_RELAY_BLOWER       12
#define GPIO_RELAY_LIGHT        13
#define GPIO_RELAY_OZONE        14

#define GPIO_INPUT_FLOW_SWITCH  15
#define GPIO_INPUT_HIGH_LIMIT   16
#define GPIO_TEMP_SENSOR_1WIRE  4

typedef struct {
    bool heater_relay;
    bool circ_pump_relay;
    bool jet1_relay;
    bool jet2_relay;
    bool blower_relay;
    bool light_relay;
    bool ozone_relay;
    bool flow_switch_active;
    bool high_limit_active;
    float water_temp_f;
    float water_temp_c;
    bool temp_sensor_connected;
    float temp_calibration_offset;
} hal_io_states_t;

/**
 * @brief Initialize all relay outputs, input sensors, and DS18B20 temperature probe interface.
 */
esp_err_t hal_hottub_init(void);

/**
 * @brief Set relay output state (on/off).
 */
void hal_set_relay(uint8_t relay_id, bool state);

/**
 * @brief Get current state of a relay output.
 */
bool hal_get_relay(uint8_t relay_id);

/**
 * @brief Read water flow switch digital input.
 */
bool hal_get_flow_switch(void);

/**
 * @brief Read emergency high-limit overtemp switch digital input.
 */
bool hal_get_high_limit_switch(void);

/**
 * @brief Read current water temperature in Fahrenheit.
 */
float hal_read_temperature_f(void);

/**
 * @brief Read current water temperature in Celsius.
 */
float hal_read_temperature_c(void);

/**
 * @brief Copy full current IO state snapshot.
 */
void hal_get_io_states(hal_io_states_t *states);

/**
 * @brief Set temperature sensor calibration offset (°F).
 */
void hal_set_temp_calibration(float offset_f);

#ifdef __cplusplus
}
#endif

#endif // HAL_HOTTUB_H
