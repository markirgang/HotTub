#include "temp_sensor.h"

TempSensorManager::TempSensorManager()
    : _oneWire(ONE_WIRE_BUS_PIN),
      _sensors(&_oneWire),
      _deviceCount(0),
      _sensorValid(false),
      _conversionPending(false),
      _lastConversionRequestTime(0),
      _rawTempF(100.0f),
      _filteredTempF(100.0f),
      _secondaryTempF(100.0f),
      _calOffsetF(0.0f),
      _historyIndex(0),
      _historyCount(0) {
    for (int i = 0; i < FILTER_SIZE; i++) {
        _history[i] = 100.0f;
    }
}

bool TempSensorManager::begin(uint8_t pin) {
    _oneWire.begin(pin);
    _sensors.begin();
    
    // Set 10-bit resolution (0.25°C step, 187.5ms conversion time) for fast reliable responsive reads
    _sensors.setResolution(10);
    _sensors.setWaitForConversion(false); // Non-blocking!
    
    _deviceCount = _sensors.getDeviceCount();
    log_i("DS18B20 1-Wire bus initialized on GPIO %d. Found %d sensor(s).", pin, _deviceCount);
    
    if (_deviceCount > 0) {
        _sensors.requestTemperatures();
        _conversionPending = true;
        _lastConversionRequestTime = millis();
        return true;
    } else {
        log_w("No DS18B20 sensors found on 1-Wire pin %d! (Check 4.7k pull-up resistor)", pin);
        return false;
    }
}

void TempSensorManager::pushSample(float raw) {
    _history[_historyIndex] = raw;
    _historyIndex = (_historyIndex + 1) % FILTER_SIZE;
    if (_historyCount < FILTER_SIZE) _historyCount++;
    
    float sum = 0.0f;
    for (int i = 0; i < _historyCount; i++) {
        sum += _history[i];
    }
    _filteredTempF = (sum / _historyCount) + _calOffsetF;
}

void TempSensorManager::update() {
    uint32_t now = millis();
    
    if (_conversionPending) {
        // Wait at least 200ms for 10-bit conversion to finish
        if (now - _lastConversionRequestTime >= 250) {
            float tempC = _sensors.getTempCByIndex(0);
            
            // DS18B20 error values: -127.0 (disconnected), 85.0 (power-on reset unread)
            if (tempC > -50.0f && tempC < 85.0f && tempC != DEVICE_DISCONNECTED_C) {
                _rawTempF = (tempC * 9.0f / 5.0f) + 32.0f;
                pushSample(_rawTempF);
                _sensorValid = true;
                
                // Read secondary sensor if present
                if (_deviceCount > 1) {
                    float temp2C = _sensors.getTempCByIndex(1);
                    if (temp2C > -50.0f && temp2C < 85.0f && temp2C != DEVICE_DISCONNECTED_C) {
                        _secondaryTempF = (temp2C * 9.0f / 5.0f) + 32.0f;
                    }
                }
            } else {
                // Sensor disconnected or read error
                _sensorValid = false;
                log_w("DS18B20 read error! Raw temp: %.2f C", tempC);
            }
            
            _conversionPending = false;
        }
    } else {
        // Request next temperature sample every 1000ms
        if (now - _lastConversionRequestTime >= 1000) {
            _sensors.requestTemperatures();
            _conversionPending = true;
            _lastConversionRequestTime = now;
        }
    }
}
