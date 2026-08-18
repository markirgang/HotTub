#include "i2c_bus.h"

I2CBusManager::I2CBusManager() : _mutex(nullptr), _initialized(false) {
    _mutex = xSemaphoreCreateMutex();
}

I2CBusManager::~I2CBusManager() {
    if (_mutex) {
        vSemaphoreDelete(_mutex);
    }
}

bool I2CBusManager::begin(int sda, int scl, uint32_t freq) {
    if (_initialized) return true;
    
    log_i("Initializing shared I2C bus on SDA=%d, SCL=%d, Freq=%u Hz", sda, scl, freq);
    bool ok = Wire.begin(sda, scl, freq);
    if (ok) {
        Wire.setTimeOut(50);
        _initialized = true;
        scanBus();
    } else {
        log_e("Failed to initialize I2C bus");
    }
    return ok;
}

bool I2CBusManager::lock(TickType_t timeout) {
    if (!_mutex) return false;
    return (xSemaphoreTake(_mutex, timeout) == pdTRUE);
}

void I2CBusManager::unlock() {
    if (_mutex) {
        xSemaphoreGive(_mutex);
    }
}

bool I2CBusManager::writeRegister8(uint8_t devAddr, uint8_t regAddr, uint8_t value) {
    if (!lock()) return false;
    Wire.beginTransmission(devAddr);
    Wire.write(regAddr);
    Wire.write(value);
    uint8_t err = Wire.endTransmission();
    unlock();
    return (err == 0);
}

uint8_t I2CBusManager::readRegister8(uint8_t devAddr, uint8_t regAddr, bool* success) {
    if (success) *success = false;
    if (!lock()) return 0;
    
    Wire.beginTransmission(devAddr);
    Wire.write(regAddr);
    if (Wire.endTransmission(false) != 0) {
        unlock();
        return 0;
    }
    
    if (Wire.requestFrom(devAddr, (uint8_t)1) == 1) {
        uint8_t val = Wire.read();
        unlock();
        if (success) *success = true;
        return val;
    }
    
    unlock();
    return 0;
}

bool I2CBusManager::readRegisters(uint8_t devAddr, uint8_t startReg, uint8_t* buffer, size_t length) {
    if (!buffer || length == 0) return false;
    if (!lock()) return false;
    
    Wire.beginTransmission(devAddr);
    Wire.write(startReg);
    if (Wire.endTransmission(false) != 0) {
        unlock();
        return false;
    }
    
    size_t received = Wire.requestFrom(devAddr, (uint8_t)length);
    if (received == length) {
        for (size_t i = 0; i < length; i++) {
            buffer[i] = Wire.read();
        }
        unlock();
        return true;
    }
    
    unlock();
    return false;
}

bool I2CBusManager::writeBytes(uint8_t devAddr, const uint8_t* data, size_t length) {
    if (!data || length == 0) return false;
    if (!lock()) return false;
    
    Wire.beginTransmission(devAddr);
    for (size_t i = 0; i < length; i++) {
        Wire.write(data[i]);
    }
    uint8_t err = Wire.endTransmission();
    unlock();
    return (err == 0);
}

bool I2CBusManager::readBytes(uint8_t devAddr, uint8_t* buffer, size_t length) {
    if (!buffer || length == 0) return false;
    if (!lock()) return false;
    
    size_t received = Wire.requestFrom(devAddr, (uint8_t)length);
    if (received == length) {
        for (size_t i = 0; i < length; i++) {
            buffer[i] = Wire.read();
        }
        unlock();
        return true;
    }
    
    unlock();
    return false;
}

bool I2CBusManager::ping(uint8_t devAddr) {
    if (!lock()) return false;
    Wire.beginTransmission(devAddr);
    uint8_t err = Wire.endTransmission();
    unlock();
    return (err == 0);
}

void I2CBusManager::scanBus() {
    log_i("Scanning I2C bus...");
    int count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (ping(addr)) {
            log_i(" - Found I2C device at 0x%02X", addr);
            count++;
        }
    }
    log_i("I2C scan complete. Total %d devices found.", count);
}
