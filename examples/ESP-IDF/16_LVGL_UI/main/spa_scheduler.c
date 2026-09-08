#include "spa_scheduler.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <time.h>

static const char *TAG = "SPA_SCHED";

void spa_scheduler_init(void)
{
    ESP_LOGI(TAG, "Spa Scheduler initialized");
}

uint32_t spa_scheduler_get_uptime_sec(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000000ULL);
}

bool spa_scheduler_is_filter_active(const hottub_config_t *config)
{
    if (!config) return false;

    // Use current system time if synced, else fallback to uptime hour offset
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    uint8_t current_hour = timeinfo.tm_hour;
    if (now < 100000) { // System time not synced via NTP yet, simulate 12-hour cycle from uptime
        uint32_t uptime = spa_scheduler_get_uptime_sec();
        current_hour = (uint8_t)((uptime / 3600) % 24);
    }

    // Check Filter Cycle 1
    uint8_t end1 = (config->filter_start_hour_1 + config->filter_duration_hours_1) % 24;
    bool in_cycle_1 = false;
    if (config->filter_start_hour_1 <= end1) {
        in_cycle_1 = (current_hour >= config->filter_start_hour_1 && current_hour < end1);
    } else {
        in_cycle_1 = (current_hour >= config->filter_start_hour_1 || current_hour < end1);
    }

    // Check Filter Cycle 2
    uint8_t end2 = (config->filter_start_hour_2 + config->filter_duration_hours_2) % 24;
    bool in_cycle_2 = false;
    if (config->filter_start_hour_2 <= end2) {
        in_cycle_2 = (current_hour >= config->filter_start_hour_2 && current_hour < end2);
    } else {
        in_cycle_2 = (current_hour >= config->filter_start_hour_2 || current_hour < end2);
    }

    return in_cycle_1 || in_cycle_2;
}
