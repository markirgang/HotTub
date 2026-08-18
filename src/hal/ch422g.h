#pragma once
#include <Arduino.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

class CH422GDriver {
public:
    static CH422GDriver& getInstance() {
        static CH422GDriver instance;
        return instance;
    }

    bool begin(uint8_t i2cAddr = CH422G_I2C_ADDR);
    
    void setBacklight(bool enable);
    void setTouchReset(bool level);
    void setLcdReset(bool level);
    void setOutputPin(uint8_t pinIndex, bool level);
    
    void resetDisplayAndTouch();

private:
    CH422GDriver();
    uint8_t _i2cAddr;
    uint8_t _outputState;
    bool _initialized;
    
    void writeOutputState();
};

#define CH422G CH422GDriver::getInstance()
