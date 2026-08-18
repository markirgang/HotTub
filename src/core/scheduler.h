#pragma once
#include <Arduino.h>
#include <time.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

#if __has_include("src/core/config_manager.h")
#include "src/core/config_manager.h"
#elif __has_include("config_manager.h")
#include "config_manager.h"
#endif

class SchedulerManager {
public:
    static SchedulerManager& getInstance() {
        static SchedulerManager instance;
        return instance;
    }

    bool begin();
    void update(); // Evaluates active time windows
    
    void syncNTP(long gmtOffsetSec = DEFAULT_TIMEZONE_OFFSET, int daylightOffsetSec = 3600);
    bool isTimeSynchronized() const { return _timeSynced; }

    bool isFilterCycleActive() const { return _filterActive; }
    bool isEcoHeatingAllowed() const { return _ecoHeatingAllowed; }
    bool isPurgeCycleActive() const { return _purgeActive; }

    String getFormattedTime(bool includeSeconds = false) const;
    String getFormattedDate() const;
    
    uint8_t getCurrentHour() const { return _currentHour; }
    uint8_t getCurrentMinute() const { return _currentMinute; }

private:
    SchedulerManager();

    bool _timeSynced;
    uint32_t _lastTimeCheck;
    uint32_t _lastNtpSync;
    
    uint8_t _currentHour;
    uint8_t _currentMinute;
    uint8_t _currentSecond;
    
    bool _filterActive;
    bool _ecoHeatingAllowed;
    bool _purgeActive;
    uint32_t _purgeStartTime;
    uint8_t _lastPurgeDay;

    bool isWithinWindow(uint8_t startHr, uint8_t startMin, uint16_t durationMins, uint8_t curHr, uint8_t curMin) const;
};

#define SpaScheduler SchedulerManager::getInstance()
