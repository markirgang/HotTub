#pragma once
#include <Arduino.h>
#include <Preferences.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

enum class TemperatureUnit : uint8_t {
    FAHRENHEIT = 0,
    CELSIUS = 1
};

enum class SpaMode : uint8_t {
    MODE_STANDARD = 0,      // Ready: Heats 24/7 to maintain target temperature
    MODE_ECO = 1,           // Economy: Heats only during scheduled filter/off-peak hours
    MODE_PARTY = 2,         // Party: Jets + Blower high, temperature boost
    MODE_CLEAN = 3,         // Clean: 10-minute high-flow filtration purge
    MODE_FREEZE_PROTECT = 4 // Automatic freeze safeguard
};

enum class PumpSpeed : uint8_t {
    SPEED_OFF = 0,
    SPEED_LOW = 1,
    SPEED_HIGH = 2
};

enum class BlowerSpeed : uint8_t {
    SPEED_OFF = 0,
    SPEED_LOW = 1,
    SPEED_MED = 2,
    SPEED_HIGH = 3
};

struct FilterCycleConfig {
    bool enabled;
    uint8_t startHour;      // 0 - 23
    uint8_t startMinute;    // 0 - 59
    uint16_t durationMins;  // 15 - 360 mins
};

struct EcoScheduleConfig {
    bool enabled;
    uint8_t startHour;      // e.g. 23 (11 PM)
    uint8_t stopHour;       // e.g. 7 (7 AM)
};

class ConfigManager {
public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    bool begin();
    void load();
    void save();
    void resetToDefaults();

    // Temperature & Units
    float getTargetTempF() const { return _targetTempF; }
    void setTargetTempF(float tempF);
    
    TemperatureUnit getTempUnit() const { return _tempUnit; }
    void setTempUnit(TemperatureUnit unit);

    float getTempCalibrationOffset() const { return _tempCalOffset; }
    void setTempCalibrationOffset(float offset);

    // Operating Mode
    SpaMode getOperatingMode() const { return _operatingMode; }
    void setOperatingMode(SpaMode mode);

    // Timers & Schedules
    FilterCycleConfig getFilterCycle1() const { return _filter1; }
    void setFilterCycle1(const FilterCycleConfig& cfg);

    FilterCycleConfig getFilterCycle2() const { return _filter2; }
    void setFilterCycle2(const FilterCycleConfig& cfg);

    EcoScheduleConfig getEcoSchedule() const { return _ecoSchedule; }
    void setEcoSchedule(const EcoScheduleConfig& cfg);

    uint16_t getJetTimeoutSeconds() const { return _jetTimeoutSec; }
    void setJetTimeoutSeconds(uint16_t sec);

    uint16_t getBlowerTimeoutSeconds() const { return _blowerTimeoutSec; }
    void setBlowerTimeoutSeconds(uint16_t sec);

    // UI & Hardware Settings
    uint8_t getScreenBrightness() const { return _screenBrightness; }
    void setScreenBrightness(uint8_t percent);

    uint16_t getScreenSleepTimeoutSec() const { return _screenSleepSec; }
    void setScreenSleepTimeoutSec(uint16_t sec);

    // WiFi
    String getWifiSSID() const { return _wifiSSID; }
    String getWifiPass() const { return _wifiPass; }
    void setWifiCredentials(const String& ssid, const String& pass);

private:
    ConfigManager();
    Preferences _prefs;

    float _targetTempF;
    TemperatureUnit _tempUnit;
    float _tempCalOffset;
    SpaMode _operatingMode;
    
    FilterCycleConfig _filter1;
    FilterCycleConfig _filter2;
    EcoScheduleConfig _ecoSchedule;
    
    uint16_t _jetTimeoutSec;
    uint16_t _blowerTimeoutSec;
    
    uint8_t _screenBrightness;
    uint16_t _screenSleepSec;
    
    String _wifiSSID;
    String _wifiPass;
};

#define Config ConfigManager::getInstance()
