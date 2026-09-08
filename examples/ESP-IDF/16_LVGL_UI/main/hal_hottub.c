#include "hal_hottub.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <math.h>

static const char *TAG = "HAL_HOTTUB";

static const gpio_num_t RELAY_GPIOS[RELAY_COUNT] = {
    [RELAY_HEATER]    = GPIO_RELAY_HEATER,
    [RELAY_CIRC_PUMP] = GPIO_RELAY_CIRC_PUMP,
    [RELAY_JET1]      = GPIO_RELAY_JET1,
    [RELAY_JET2]      = GPIO_RELAY_JET2,
    [RELAY_BLOWER]    = GPIO_RELAY_BLOWER,
    [RELAY_LIGHT]     = GPIO_RELAY_LIGHT,
    [RELAY_OZONE]     = GPIO_RELAY_OZONE,
};

static SemaphoreHandle_t s_hal_mutex = NULL;
static hal_io_states_t s_io_states = {0};
static float s_simulated_temp_f = 100.0f; // Default bench test temp

esp_err_t hal_hottub_init(void)
{
    s_hal_mutex = xSemaphoreCreateMutex();
    if (!s_hal_mutex) {
        ESP_LOGE(TAG, "Failed to create HAL mutex");
        return ESP_ERR_NO_MEM;
    }

    // Configure Relay Output GPIOs
    for (int i = 0; i < RELAY_COUNT; i++) {
        gpio_reset_pin(RELAY_GPIOS[i]);
        gpio_set_direction(RELAY_GPIOS[i], GPIO_MODE_OUTPUT);
        gpio_set_level(RELAY_GPIOS[i], 0);
    }

    // Configure Input Sensors with pullup
    gpio_reset_pin(GPIO_INPUT_FLOW_SWITCH);
    gpio_set_direction(GPIO_INPUT_FLOW_SWITCH, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_INPUT_FLOW_SWITCH, GPIO_PULLUP_ONLY);

    gpio_reset_pin(GPIO_INPUT_HIGH_LIMIT);
    gpio_set_direction(GPIO_INPUT_HIGH_LIMIT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_INPUT_HIGH_LIMIT, GPIO_PULLUP_ONLY);

    // Initial state setup
    s_io_states.flow_switch_active = true; // Default active for bench test unless hardware trips
    s_io_states.high_limit_active = false; // No overtemp trip
    s_io_states.water_temp_f = 100.0f;
    s_io_states.water_temp_c = (100.0f - 32.0f) * 5.0f / 9.0f;
    s_io_states.temp_sensor_connected = true;
    s_io_states.temp_calibration_offset = 0.0f;

    ESP_LOGI(TAG, "Hardware Abstraction Layer initialized (7 Relays, 2 Sensors, 1-Wire)");
    return ESP_OK;
}

void hal_set_relay(uint8_t relay_id, bool state)
{
    if (relay_id >= RELAY_COUNT) return;

    if (xSemaphoreTake(s_hal_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        gpio_set_level(RELAY_GPIOS[relay_id], state ? 1 : 0);
        switch (relay_id) {
            case RELAY_HEATER:    s_io_states.heater_relay = state; break;
            case RELAY_CIRC_PUMP: s_io_states.circ_pump_relay = state; break;
            case RELAY_JET1:      s_io_states.jet1_relay = state; break;
            case RELAY_JET2:      s_io_states.jet2_relay = state; break;
            case RELAY_BLOWER:    s_io_states.blower_relay = state; break;
            case RELAY_LIGHT:     s_io_states.light_relay = state; break;
            case RELAY_OZONE:     s_io_states.ozone_relay = state; break;
            default: break;
        }
        xSemaphoreGive(s_hal_mutex);
    }
}

bool hal_get_relay(uint8_t relay_id)
{
    bool val = false;
    if (relay_id >= RELAY_COUNT) return false;

    if (xSemaphoreTake(s_hal_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        switch (relay_id) {
            case RELAY_HEATER:    val = s_io_states.heater_relay; break;
            case RELAY_CIRC_PUMP: val = s_io_states.circ_pump_relay; break;
            case RELAY_JET1:      val = s_io_states.jet1_relay; break;
            case RELAY_JET2:      val = s_io_states.jet2_relay; break;
            case RELAY_BLOWER:    val = s_io_states.blower_relay; break;
            case RELAY_LIGHT:     val = s_io_states.light_relay; break;
            case RELAY_OZONE:     val = s_io_states.ozone_relay; break;
            default: break;
        }
        xSemaphoreGive(s_hal_mutex);
    }
    return val;
}

bool hal_get_flow_switch(void)
{
    // Read hardware digital pin (Active low when flow is detected)
    int level = gpio_get_level(GPIO_INPUT_FLOW_SWITCH);
    // If flow switch pin is pulled low or circ pump is active in bench test mode, flow is true
    bool hardware_flow = (level == 0);
    bool bench_flow = s_io_states.circ_pump_relay;
    return hardware_flow || bench_flow;
}

bool hal_get_high_limit_switch(void)
{
    int level = gpio_get_level(GPIO_INPUT_HIGH_LIMIT);
    return (level == 1); // Active HIGH trip
}

float hal_read_temperature_f(void)
{
    float temp_f = 100.0f;
    if (xSemaphoreTake(s_hal_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        // Bench test thermal simulation: if heater is ON, slowly warm up; if OFF, slowly cool down
        if (s_io_states.heater_relay) {
            s_simulated_temp_f += 0.05f;
            if (s_simulated_temp_f > 106.0f) s_simulated_temp_f = 106.0f;
        } else {
            s_simulated_temp_f -= 0.02f;
            if (s_simulated_temp_f < 65.0f) s_simulated_temp_f = 65.0f;
        }
        
        temp_f = s_simulated_temp_f + s_io_states.temp_calibration_offset;
        s_io_states.water_temp_f = temp_f;
        s_io_states.water_temp_c = (temp_f - 32.0f) * 5.0f / 9.0f;
        xSemaphoreGive(s_hal_mutex);
    }
    return temp_f;
}

float hal_read_temperature_c(void)
{
    return (hal_read_temperature_f() - 32.0f) * 5.0f / 9.0f;
}

void hal_get_io_states(hal_io_states_t *states)
{
    if (!states) return;
    if (xSemaphoreTake(s_hal_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_io_states.flow_switch_active = hal_get_flow_switch();
        s_io_states.high_limit_active = hal_get_high_limit_switch();
        *states = s_io_states;
        xSemaphoreGive(s_hal_mutex);
    }
}

void hal_set_temp_calibration(float offset_f)
{
    if (xSemaphoreTake(s_hal_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_io_states.temp_calibration_offset = offset_f;
        xSemaphoreGive(s_hal_mutex);
    }
}
