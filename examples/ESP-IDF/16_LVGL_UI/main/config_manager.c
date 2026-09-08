#include "config_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "CONFIG_MGR";
static const char *NVS_NAMESPACE = "hottub_cfg";
static const char *NVS_KEY_BLOB  = "spa_settings";

void config_manager_get_defaults(hottub_config_t *config)
{
    if (!config) return;
    config->setpoint_f = 102.0f;
    config->is_celsius = false;
    config->heat_mode = HEAT_MODE_STANDARD;
    config->filter_start_hour_1 = 8;      // 8:00 AM
    config->filter_duration_hours_1 = 2;  // 2 hours
    config->filter_start_hour_2 = 20;     // 8:00 PM
    config->filter_duration_hours_2 = 2;  // 2 hours
    config->jet_timeout_mins = 15;        // 15 minutes
    config->blower_timeout_mins = 15;     // 15 minutes
    config->temp_offset_f = 0.0f;
}

esp_err_t config_manager_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS flash partition truncated/outdated; erasing and reinitializing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t config_manager_load(hottub_config_t *config)
{
    if (!config) return ESP_ERR_INVALID_ARG;

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "NVS namespace not found, applying default configuration");
        config_manager_get_defaults(config);
        return ESP_OK;
    }

    size_t required_size = sizeof(hottub_config_t);
    err = nvs_get_blob(nvs_handle, NVS_KEY_BLOB, config, &required_size);
    nvs_close(nvs_handle);

    if (err != ESP_OK || required_size != sizeof(hottub_config_t)) {
        ESP_LOGW(TAG, "Failed to read configuration blob from NVS (err=%d), using defaults", err);
        config_manager_get_defaults(config);
        return ESP_OK;
    }

    // Safety Sanity Validation
    if (config->setpoint_f < 80.0f || config->setpoint_f > 104.0f) {
        config->setpoint_f = 102.0f;
    }
    ESP_LOGI(TAG, "Configuration loaded successfully: Setpoint=%.1f°F, HeatMode=%d", 
             config->setpoint_f, config->heat_mode);
    return ESP_OK;
}

esp_err_t config_manager_save(const hottub_config_t *config)
{
    if (!config) return ESP_ERR_INVALID_ARG;

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle for write: %d", err);
        return err;
    }

    err = nvs_set_blob(nvs_handle, NVS_KEY_BLOB, config, sizeof(hottub_config_t));
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
        ESP_LOGI(TAG, "Configuration saved to NVS (Setpoint=%.1f°F)", config->setpoint_f);
    } else {
        ESP_LOGE(TAG, "Error setting NVS blob: %d", err);
    }

    nvs_close(nvs_handle);
    return err;
}
