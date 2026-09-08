#ifndef SPA_SCHEDULER_H
#define SPA_SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>
#include "config_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the filtration scheduler task.
 */
void spa_scheduler_init(void);

/**
 * @brief Check whether a filtration cycle is currently active based on current time/uptime.
 */
bool spa_scheduler_is_filter_active(const hottub_config_t *config);

/**
 * @brief Get total system uptime in seconds.
 */
uint32_t spa_scheduler_get_uptime_sec(void);

#ifdef __cplusplus
}
#endif

#endif // SPA_SCHEDULER_H
