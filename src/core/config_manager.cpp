#include "config_manager.h"

ConfigManager::ConfigManager()
    : _targetTempF(TEMP_DEFAULT_SETPOINT_F),
      _tempUnit(TemperatureUnit::FAHRENHEIT),
      _tempCalOffset(0.0f),
      _operatingMode(SpaMode::MODE_STANDARD),
      _jetTimeoutSec(JET_HIGH_TIMEOUT_SEC),
      _blowerTimeoutSec(BLOWER_TIMEOUT_SEC),
      _screenBrightness(100),
      _screenSleepSec(300),
      _wifiSSID(""),
      _wifiPass("") {
    // Default Filter Cycle 1: 08:00 for 2 hours (120 mins)
    _filter1 = { .enabled = true, .startHour = 8, .startMinute = 0, .durationMins = 120 };
    // Default Filter Cycle 2: 20:00 for 2 hours (120 mins)
    _filter2 = { .enabled = true, .startHour = 20, .startMinute = 0, .durationMins = 120 };
    // Default Eco schedule: 23:00 to 07:00
    _ecoSchedule = { .enabled = false, .startHour = 23, .stopHour = 7 };
}

bool ConfigManager::begin() {
    _prefs.begin("spa_cfg", false);
    load();
    log_i("Configuration loaded. Target Temp: %.1f F, Mode: %d", _targetTempF, (int)_operatingMode);
    return true;
}

void ConfigManager::load() {
    _targetTempF = _prefs.getFloat("tgt_temp", TEMP_DEFAULT_SETPOINT_F);
    if (_targetTempF < TEMP_MIN_SETPOINT_F || _targetTempF > TEMP_MAX_SETPOINT_F) {
        _targetTempF = TEMP_DEFAULT_SETPOINT_F;
    }
    
    _tempUnit = (TemperatureUnit)_prefs.getUChar("unit", (uint8_t)TemperatureUnit::FAHRENHEIT);
    _tempCalOffset = _prefs.getFloat("cal_off", 0.0f);
    _operatingMode = (SpaMode)_prefs.getUChar("mode", (uint8_t)SpaMode::MODE_STANDARD);

    _filter1.enabled = _prefs.getBool("f1_en", true);
    _filter1.startHour = _prefs.getUChar("f1_hr", 8);
    _filter1.startMinute = _prefs.getUChar("f1_min", 0);
    _filter1.durationMins = _prefs.getUShort("f1_dur", 120);

    _filter2.enabled = _prefs.getBool("f2_en", true);
    _filter2.startHour = _prefs.getUChar("f2_hr", 20);
    _filter2.startMinute = _prefs.getUChar("f2_min", 0);
    _filter2.durationMins = _prefs.getUShort("f2_dur", 120);

    _ecoSchedule.enabled = _prefs.getBool("eco_en", false);
    _ecoSchedule.startHour = _prefs.getUChar("eco_start", 23);
    _ecoSchedule.stopHour = _prefs.getUChar("eco_stop", 7);

    _jetTimeoutSec = _prefs.getUShort("jet_to", JET_HIGH_TIMEOUT_SEC);
    _blowerTimeoutSec = _prefs.getUShort("blw_to", BLOWER_TIMEOUT_SEC);

    _screenBrightness = _prefs.getUChar("scr_br", 100);
    _screenSleepSec = _prefs.getUShort("scr_slp", 300);

    _wifiSSID = _prefs.getString("wifi_ssid", "");
    _wifiPass = _prefs.getString("wifi_pass", "");
}

void ConfigManager::save() {
    _prefs.putFloat("tgt_temp", _targetTempF);
    _prefs.putUChar("unit", (uint8_t)_tempUnit);
    _prefs.putFloat("cal_off", _tempCalOffset);
    _prefs.putUChar("mode", (uint8_t)_operatingMode);

    _prefs.putBool("f1_en", _filter1.enabled);
    _prefs.putUChar("f1_hr", _filter1.startHour);
    _prefs.putUChar("f1_min", _filter1.startMinute);
    _prefs.putUShort("f1_dur", _filter1.durationMins);

    _prefs.putBool("f2_en", _filter2.enabled);
    _prefs.putUChar("f2_hr", _filter2.startHour);
    _prefs.putUChar("f2_min", _filter2.startMinute);
    _prefs.putUShort("f2_dur", _filter2.durationMins);

    _prefs.putBool("eco_en", _ecoSchedule.enabled);
    _prefs.putUChar("eco_start", _ecoSchedule.startHour);
    _prefs.putUChar("eco_stop", _ecoSchedule.stopHour);

    _prefs.putUShort("jet_to", _jetTimeoutSec);
    _prefs.putUShort("blw_to", _blowerTimeoutSec);

    _prefs.putUChar("scr_br", _screenBrightness);
    _prefs.putUShort("scr_slp", _screenSleepSec);

    _prefs.putString("wifi_ssid", _wifiSSID);
    _prefs.putString("wifi_pass", _wifiPass);
    log_i("Configuration successfully saved to NVS.");
}

void ConfigManager::resetToDefaults() {
    _prefs.clear();
    _targetTempF = TEMP_DEFAULT_SETPOINT_F;
    _tempUnit = TemperatureUnit::FAHRENHEIT;
    _tempCalOffset = 0.0f;
    _operatingMode = SpaMode::MODE_STANDARD;
    _filter1 = { .enabled = true, .startHour = 8, .startMinute = 0, .durationMins = 120 };
    _filter2 = { .enabled = true, .startHour = 20, .startMinute = 0, .durationMins = 120 };
    _ecoSchedule = { .enabled = false, .startHour = 23, .stopHour = 7 };
    _jetTimeoutSec = JET_HIGH_TIMEOUT_SEC;
    _blowerTimeoutSec = BLOWER_TIMEOUT_SEC;
    _screenBrightness = 100;
    _screenSleepSec = 300;
    _wifiSSID = "";
    _wifiPass = "";
    save();
}

void ConfigManager::setTargetTempF(float tempF) {
    if (tempF < TEMP_MIN_SETPOINT_F) tempF = TEMP_MIN_SETPOINT_F;
    if (tempF > TEMP_MAX_SETPOINT_F) tempF = TEMP_MAX_SETPOINT_F;
    _targetTempF = tempF;
    _prefs.putFloat("tgt_temp", _targetTempF);
}

void ConfigManager::setTempUnit(TemperatureUnit unit) {
    _tempUnit = unit;
    _prefs.putUChar("unit", (uint8_t)_tempUnit);
}

void ConfigManager::setTempCalibrationOffset(float offset) {
    _tempCalOffset = offset;
    _prefs.putFloat("cal_off", _tempCalOffset);
}

void ConfigManager::setOperatingMode(SpaMode mode) {
    _operatingMode = mode;
    _prefs.putUChar("mode", (uint8_t)_operatingMode);
}

void ConfigManager::setFilterCycle1(const FilterCycleConfig& cfg) {
    _filter1 = cfg;
    _prefs.putBool("f1_en", _filter1.enabled);
    _prefs.putUChar("f1_hr", _filter1.startHour);
    _prefs.putUChar("f1_min", _filter1.startMinute);
    _prefs.putUShort("f1_dur", _filter1.durationMins);
}

void ConfigManager::setFilterCycle2(const FilterCycleConfig& cfg) {
    _filter2 = cfg;
    _prefs.putBool("f2_en", _filter2.enabled);
    _prefs.putUChar("f2_hr", _filter2.startHour);
    _prefs.putUChar("f2_min", _filter2.startMinute);
    _prefs.putUShort("f2_dur", _filter2.durationMins);
}

void ConfigManager::setEcoSchedule(const EcoScheduleConfig& cfg) {
    _ecoSchedule = cfg;
    _prefs.putBool("eco_en", _ecoSchedule.enabled);
    _prefs.putUChar("eco_start", _ecoSchedule.startHour);
    _prefs.putUChar("eco_stop", _ecoSchedule.stopHour);
}

void ConfigManager::setJetTimeoutSeconds(uint16_t sec) {
    _jetTimeoutSec = sec;
    _prefs.putUShort("jet_to", _jetTimeoutSec);
}

void ConfigManager::setBlowerTimeoutSeconds(uint16_t sec) {
    _blowerTimeoutSec = sec;
    _prefs.putUShort("blw_to", _blowerTimeoutSec);
}

void ConfigManager::setScreenBrightness(uint8_t percent) {
    _screenBrightness = (percent > 100) ? 100 : percent;
    _prefs.putUChar("scr_br", _screenBrightness);
}

void ConfigManager::setScreenSleepTimeoutSec(uint16_t sec) {
    _screenSleepSec = sec;
    _prefs.putUShort("scr_slp", _screenSleepSec);
}

void ConfigManager::setWifiCredentials(const String& ssid, const String& pass) {
    _wifiSSID = ssid;
    _wifiPass = pass;
    _prefs.putString("wifi_ssid", _wifiSSID);
    _prefs.putString("wifi_pass", _wifiPass);
}
