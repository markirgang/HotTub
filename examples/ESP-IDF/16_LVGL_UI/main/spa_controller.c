#include "spa_controller.h"
#include "hal_hottub.h"
#include "spa_scheduler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "SPA_CTRL";

#define FLOW_PURGE_REQUIRED_SEC 10
#define COOLDOWN_REQUIRED_SEC   30

static SemaphoreHandle_t s_spa_mutex = NULL;
static hottub_config_t s_config = {0};
static spa_status_t s_status = {0};

static uint16_t s_jet1_seconds_left = 0;
static uint16_t s_jet2_seconds_left = 0;
static uint16_t s_blower_seconds_left = 0;
static uint8_t  s_purge_counter = 0;
static uint8_t  s_cooldown_counter = 0;

static void spa_control_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Spa Controller background task running");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            continue;
        }

        // 1. Hardware Sensor Readings
        float current_temp_f = hal_read_temperature_f();
        bool flow_ok = hal_get_flow_switch();
        bool high_limit_trip = hal_get_high_limit_switch();

        s_status.current_temp_f = current_temp_f;
        s_status.current_temp_c = (current_temp_f - 32.0f) * 5.0f / 9.0f;
        s_status.setpoint_f = s_config.setpoint_f;
        s_status.is_celsius = s_config.is_celsius;
        s_status.heat_mode = (heat_mode_t)s_config.heat_mode;
        s_status.flow_verified = flow_ok;

        // 2. High-Temperature & Hardware Limit Protection (OVERTEMP)
        if (current_temp_f >= 104.5f || high_limit_trip) {
            hal_set_relay(RELAY_HEATER, false);
            s_status.state = SPA_STATE_ERROR_OVERTEMP;
            snprintf(s_status.status_msg, sizeof(s_status.status_msg), "SAFETY TRIP: OVERTEMP (%.1f°F)", current_temp_f);
            xSemaphoreGive(s_spa_mutex);
            continue;
        }

        // 3. Freeze Protection Logic
        if (current_temp_f <= 44.0f) {
            s_status.state = SPA_STATE_FREEZE_PROTECT;
            s_status.freeze_active = true;
            hal_set_relay(RELAY_CIRC_PUMP, true);
            if (flow_ok) {
                hal_set_relay(RELAY_HEATER, true);
                snprintf(s_status.status_msg, sizeof(s_status.status_msg), "FREEZE PROTECT: HEATING");
            } else {
                hal_set_relay(RELAY_HEATER, false);
                snprintf(s_status.status_msg, sizeof(s_status.status_msg), "FREEZE PROTECT: FLOW CHECK");
            }
            xSemaphoreGive(s_spa_mutex);
            continue;
        } else {
            s_status.freeze_active = false;
        }

        // 4. Timers for High Jets & Blower
        if (s_status.jet1_active) {
            if (s_jet1_seconds_left > 0) {
                s_jet1_seconds_left--;
            } else {
                s_status.jet1_active = false;
                hal_set_relay(RELAY_JET1, false);
                ESP_LOGI(TAG, "Jet 1 auto-shutoff timer expired");
            }
        }
        s_status.jet1_timer_sec = s_jet1_seconds_left;

        if (s_status.jet2_active) {
            if (s_jet2_seconds_left > 0) {
                s_jet2_seconds_left--;
            } else {
                s_status.jet2_active = false;
                hal_set_relay(RELAY_JET2, false);
                ESP_LOGI(TAG, "Jet 2 auto-shutoff timer expired");
            }
        }
        s_status.jet2_timer_sec = s_jet2_seconds_left;

        if (s_status.blower_active) {
            if (s_blower_seconds_left > 0) {
                s_blower_seconds_left--;
            } else {
                s_status.blower_active = false;
                hal_set_relay(RELAY_BLOWER, false);
                ESP_LOGI(TAG, "Blower auto-shutoff timer expired");
            }
        }
        s_status.blower_timer_sec = s_blower_seconds_left;

        // 5. Filtration Scheduler & Ozone Control
        bool filter_active = spa_scheduler_is_filter_active(&s_config);
        s_status.is_filtering = filter_active;
        hal_set_relay(RELAY_OZONE, filter_active);

        // 6. Heating & Flow State Machine
        float effective_setpoint = s_config.setpoint_f;
        if (s_config.heat_mode == HEAT_MODE_SLEEP) {
            effective_setpoint -= 20.0f; // Sleep mode target
        }

        bool heat_demanded = (current_temp_f < (effective_setpoint - 0.5f));
        if (s_config.heat_mode == HEAT_MODE_ECONOMY && !filter_active) {
            heat_demanded = false; // In Economy mode, only heat during filter cycle
        }

        switch (s_status.state) {
            case SPA_STATE_OFF:
            case SPA_STATE_STANDBY:
                if (heat_demanded) {
                    s_status.state = SPA_STATE_HEATING_PURGE;
                    s_purge_counter = FLOW_PURGE_REQUIRED_SEC;
                    hal_set_relay(RELAY_CIRC_PUMP, true);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "VERIFYING FLOW (%ds)", s_purge_counter);
                } else if (filter_active) {
                    s_status.state = SPA_STATE_FILTERING;
                    hal_set_relay(RELAY_CIRC_PUMP, true);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "FILTRATION CYCLE");
                } else {
                    hal_set_relay(RELAY_CIRC_PUMP, false);
                    hal_set_relay(RELAY_HEATER, false);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "STANDBY");
                }
                break;

            case SPA_STATE_HEATING_PURGE:
                hal_set_relay(RELAY_CIRC_PUMP, true);
                if (!flow_ok) {
                    s_status.state = SPA_STATE_ERROR_FLOW;
                    hal_set_relay(RELAY_HEATER, false);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "ERROR: NO FLOW DETECTED");
                } else if (s_purge_counter > 0) {
                    s_purge_counter--;
                    s_status.flow_purge_sec = s_purge_counter;
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "VERIFYING FLOW (%ds)", s_purge_counter);
                } else {
                    s_status.state = SPA_STATE_HEATING;
                    hal_set_relay(RELAY_HEATER, true);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "HEATING ACTIVE");
                }
                break;

            case SPA_STATE_HEATING:
                if (!flow_ok) {
                    s_status.state = SPA_STATE_ERROR_FLOW;
                    hal_set_relay(RELAY_HEATER, false);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "ERROR: FLOW LOST");
                } else if (!heat_demanded) {
                    s_status.state = SPA_STATE_COOLDOWN;
                    s_cooldown_counter = COOLDOWN_REQUIRED_SEC;
                    hal_set_relay(RELAY_HEATER, false);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "HEATER COOLDOWN (%ds)", s_cooldown_counter);
                } else {
                    hal_set_relay(RELAY_CIRC_PUMP, true);
                    hal_set_relay(RELAY_HEATER, true);
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "HEATING (%.1f°F)", current_temp_f);
                }
                break;

            case SPA_STATE_COOLDOWN:
                hal_set_relay(RELAY_HEATER, false);
                hal_set_relay(RELAY_CIRC_PUMP, true);
                if (s_cooldown_counter > 0) {
                    s_cooldown_counter--;
                    s_status.cooldown_sec = s_cooldown_counter;
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "HEATER COOLDOWN (%ds)", s_cooldown_counter);
                } else {
                    s_status.state = SPA_STATE_STANDBY;
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "READY");
                }
                break;

            case SPA_STATE_FILTERING:
                hal_set_relay(RELAY_CIRC_PUMP, true);
                if (heat_demanded) {
                    s_status.state = SPA_STATE_HEATING_PURGE;
                    s_purge_counter = FLOW_PURGE_REQUIRED_SEC;
                } else if (!filter_active) {
                    s_status.state = SPA_STATE_STANDBY;
                } else {
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "FILTRATION CYCLE");
                }
                break;

            case SPA_STATE_ERROR_FLOW:
                hal_set_relay(RELAY_HEATER, false);
                if (flow_ok) {
                    s_status.state = SPA_STATE_STANDBY; // Flow restored
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "FLOW RESTORED");
                } else {
                    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "ERROR: CHECK FLOW SWITCH");
                }
                break;

            default:
                break;
        }

        xSemaphoreGive(s_spa_mutex);
    }
}

esp_err_t spa_controller_init(void)
{
    s_spa_mutex = xSemaphoreCreateMutex();
    if (!s_spa_mutex) return ESP_ERR_NO_MEM;

    config_manager_init();
    config_manager_load(&s_config);
    hal_hottub_init();
    spa_scheduler_init();

    s_status.state = SPA_STATE_STANDBY;
    s_status.setpoint_f = s_config.setpoint_f;
    s_status.is_celsius = s_config.is_celsius;
    s_status.heat_mode = (heat_mode_t)s_config.heat_mode;
    snprintf(s_status.status_msg, sizeof(s_status.status_msg), "SYSTEM INITIALIZED");

    xTaskCreate(spa_control_task, "spa_ctrl_task", 4 * 1024, NULL, 5, NULL);
    return ESP_OK;
}

void spa_controller_get_status(spa_status_t *status)
{
    if (!status) return;
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        *status = s_status;
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_get_config(hottub_config_t *config)
{
    if (!config) return;
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        *config = s_config;
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_set_setpoint(float setpoint_f)
{
    if (setpoint_f < 80.0f) setpoint_f = 80.0f;
    if (setpoint_f > 104.0f) setpoint_f = 104.0f;

    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_config.setpoint_f = setpoint_f;
        s_status.setpoint_f = setpoint_f;
        config_manager_save(&s_config);
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_toggle_jet1(void)
{
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_status.jet1_active = !s_status.jet1_active;
        hal_set_relay(RELAY_JET1, s_status.jet1_active);
        if (s_status.jet1_active) {
            s_jet1_seconds_left = s_config.jet_timeout_mins * 60;
        } else {
            s_jet1_seconds_left = 0;
        }
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_toggle_jet2(void)
{
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_status.jet2_active = !s_status.jet2_active;
        hal_set_relay(RELAY_JET2, s_status.jet2_active);
        if (s_status.jet2_active) {
            s_jet2_seconds_left = s_config.jet_timeout_mins * 60;
        } else {
            s_jet2_seconds_left = 0;
        }
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_toggle_blower(void)
{
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_status.blower_active = !s_status.blower_active;
        hal_set_relay(RELAY_BLOWER, s_status.blower_active);
        if (s_status.blower_active) {
            s_blower_seconds_left = s_config.blower_timeout_mins * 60;
        } else {
            s_blower_seconds_left = 0;
        }
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_toggle_light(void)
{
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_status.light_active = !s_status.light_active;
        hal_set_relay(RELAY_LIGHT, s_status.light_active);
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_set_heat_mode(heat_mode_t mode)
{
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_config.heat_mode = (uint8_t)mode;
        s_status.heat_mode = mode;
        config_manager_save(&s_config);
        xSemaphoreGive(s_spa_mutex);
    }
}

void spa_controller_set_celsius(bool is_celsius)
{
    if (xSemaphoreTake(s_spa_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_config.is_celsius = is_celsius;
        s_status.is_celsius = is_celsius;
        config_manager_save(&s_config);
        xSemaphoreGive(s_spa_mutex);
    }
}
