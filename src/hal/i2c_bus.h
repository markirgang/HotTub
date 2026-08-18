#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"

class I2CBusManager {
public:
    static I2CBusManager& getInstance() {
        static I2CBusManager instance;
        return instance;
    }

    bool begin(int sda = I2C_SDA_PIN, int scl = I2C_SCL_PIN, uint32_t freq = I2C_FREQ_HZ);
    
    bool lock(TickType_t timeout = pdMS_TO_TICKS(100));
    void unlock();
    
    TwoWire& getWire() { return Wire; }

    bool writeRegister8(uint8_t devAddr, uint8_t regAddr, uint8_t value);
    uint8_t readRegister8(uint8_t devAddr, uint8_t regAddr, bool* success = nullptr);
    bool readRegisters(uint8_t devAddr, uint8_t startReg, uint8_t* buffer, size_t length);
    bool writeBytes(uint8_t devAddr, const uint8_t* data, size_t length);
    bool readBytes(uint8_t devAddr, uint8_t* buffer, size_t length);
    
    void scanBus();
    bool ping(uint8_t devAddr);

private:
    I2CBusManager();
    ~I2CBusManager();
    
    SemaphoreHandle_t _mutex;
    bool _initialized;
};

#define I2CBus I2CBusManager::getInstance()
