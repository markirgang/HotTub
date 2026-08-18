#include "scheduler.h"
#include <WiFi.h>

SchedulerManager::SchedulerManager()
    : _timeSynced(false),
      _lastTimeCheck(0),
      _lastNtpSync(0),
      _currentHour(12),
      _currentMinute(0),
      _currentSecond(0),
      _filterActive(false),
      _ecoHeatingAllowed(true),
      _purgeActive(false),
      _purgeStartTime(0),
      _lastPurgeDay(255) {
}

bool SchedulerManager::begin() {
    log_i("Initializing Scheduler...");
    // Initialize standard config time
    configTime(DEFAULT_TIMEZONE_OFFSET, 3600, NTP_SERVER_1, NTP_SERVER_2);
    return true;
}

void SchedulerManager::syncNTP(long gmtOffsetSec, int daylightOffsetSec) {
    if (WiFi.status() == WL_CONNECTED) {
        log_i("Synchronizing time with NTP servers...");
        configTime(gmtOffsetSec, daylightOffsetSec, NTP_SERVER_1, NTP_SERVER_2);
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 2000)) {
            _timeSynced = true;
            _lastNtpSync = millis();
            log_i("NTP Time synchronized: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            log_w("NTP Time sync failed or timed out.");
        }
    }
}

bool SchedulerManager::isWithinWindow(uint8_t startHr, uint8_t startMin, uint16_t durationMins, uint8_t curHr, uint8_t curMin) const {
    uint32_t startTotalMins = startHr * 60 + startMin;
    uint32_t currentTotalMins = curHr * 60 + curMin;
    uint32_t endTotalMins = (startTotalMins + durationMins) % 1440;

    if (startTotalMins + durationMins <= 1440) {
        // Simple same-day window
        return (currentTotalMins >= startTotalMins && currentTotalMins < (startTotalMins + durationMins));
    } else {
        // Midnight wrap-around window
        return (currentTotalMins >= startTotalMins || currentTotalMins < endTotalMins);
    }
}

void SchedulerManager::update() {
    uint32_t now = millis();
    
    // Update local time readings once per second
    if (now - _lastTimeCheck >= 1000) {
        _lastTimeCheck = now;
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 50)) {
            _timeSynced = true;
            _currentHour = timeinfo.tm_hour;
            _currentMinute = timeinfo.tm_min;
            _currentSecond = timeinfo.tm_sec;
            
            // Check daily purge cycle (runs at 12:00 PM once daily for 30s)
            if (_currentHour == 12 && _currentMinute == 0 && timeinfo.tm_mday != _lastPurgeDay && !_purgeActive) {
                _purgeActive = true;
                _purgeStartTime = now;
                _lastPurgeDay = timeinfo.tm_mday;
                log_i("Starting automatic daily pipe purge cycle (30s)...");
            }
        }
        
        // Handle active purge timer
        if (_purgeActive) {
            if (now - _purgeStartTime >= (PURGE_CYCLE_DURATION_S * 1000)) {
                _purgeActive = false;
                log_i("Daily pipe purge cycle completed.");
            }
        }

        // Evaluate Filter Cycle 1 & Filter Cycle 2
        FilterCycleConfig f1 = Config.getFilterCycle1();
        FilterCycleConfig f2 = Config.getFilterCycle2();
        
        bool f1Active = f1.enabled && isWithinWindow(f1.startHour, f1.startMinute, f1.durationMins, _currentHour, _currentMinute);
        bool f2Active = f2.enabled && isWithinWindow(f2.startHour, f2.startMinute, f2.durationMins, _currentHour, _currentMinute);
        
        _filterActive = f1Active || f2Active;

        // Evaluate Economy Heating Window
        EcoScheduleConfig eco = Config.getEcoSchedule();
        if (eco.enabled) {
            // Eco mode only heats during off-peak hours OR during active filter cycles
            uint16_t ecoDuration = (eco.stopHour >= eco.startHour) ? 
                (eco.stopHour - eco.startHour) * 60 : 
                (24 - eco.startHour + eco.stopHour) * 60;
                
            bool inEcoWindow = isWithinWindow(eco.startHour, 0, ecoDuration, _currentHour, _currentMinute);
            _ecoHeatingAllowed = inEcoWindow || _filterActive;
        } else {
            _ecoHeatingAllowed = true;
        }
    }
}

String SchedulerManager::getFormattedTime(bool includeSeconds) const {
    char buf[16];
    if (includeSeconds) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", _currentHour, _currentMinute, _currentSecond);
    } else {
        snprintf(buf, sizeof(buf), "%02d:%02d", _currentHour, _currentMinute);
    }
    return String(buf);
}

String SchedulerManager::getFormattedDate() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 50)) {
        char buf[32];
        strftime(buf, sizeof(buf), "%a, %b %d %Y", &timeinfo);
        return String(buf);
    }
    return "Spa Controller";
}
