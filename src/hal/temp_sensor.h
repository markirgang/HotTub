#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

class TempSensorManager {
public:
    static TempSensorManager& getInstance() {
        static TempSensorManager instance;
        return instance;
    }

    bool begin(uint8_t pin = ONE_WIRE_BUS_PIN);
    void update(); // Non-blocking temperature conversion cycle

    float getWaterTempF() const { return _filteredTempF; }
    float getWaterTempC() const { return (_filteredTempF - 32.0f) * 5.0f / 9.0f; }
    
    float getRawTempF() const { return _rawTempF; }
    float getSecondaryTempF() const { return _secondaryTempF; }

    bool isSensorValid() const { return _sensorValid; }
    bool isOverheated() const { return (_filteredTempF >= TEMP_OVERHEAT_LIMIT_F); }
    bool isEmergencyTrip() const { return (_filteredTempF >= TEMP_EMERGENCY_TRIP_F); }
    bool isFreezeRisk() const { return (_filteredTempF <= TEMP_FREEZE_PROTECT_F); }

    void setCalibrationOffsetF(float offset) { _calOffsetF = offset; }
    float getCalibrationOffsetF() const { return _calOffsetF; }

    uint8_t getDeviceCount() const { return _deviceCount; }

private:
    TempSensorManager();
    
    OneWire _oneWire;
    DallasTemperature _sensors;
    
    uint8_t _deviceCount;
    bool _sensorValid;
    bool _conversionPending;
    uint32_t _lastConversionRequestTime;
    
    float _rawTempF;
    float _filteredTempF;
    float _secondaryTempF;
    float _calOffsetF;

    // Moving average filter buffer
    static const uint8_t FILTER_SIZE = 8;
    float _history[FILTER_SIZE];
    uint8_t _historyIndex;
    uint8_t _historyCount;
    
    void pushSample(float raw);
};

#define TempSensor TempSensorManager::getInstance()
