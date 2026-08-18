#pragma once
#include <Arduino.h>
#include "config.h"

class PWMController {
public:
    static PWMController& getInstance() {
        static PWMController instance;
        return instance;
    }

    bool begin();

    void setBlowerSpeedPercent(uint8_t percent); // 0 - 100%
    uint8_t getBlowerSpeedPercent() const { return _blowerPercent; }

    void setLightBrightnessPercent(uint8_t percent); // 0 - 100%
    uint8_t getLightBrightnessPercent() const { return _lightPercent; }

private:
    PWMController();
    
    uint8_t _blowerPercent;
    uint8_t _lightPercent;
    bool _initialized;
};

#define PWMControl PWMController::getInstance()
