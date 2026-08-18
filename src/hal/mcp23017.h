#pragma once
#include <Arduino.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

// MCP23017 Register Addresses (IOCON.BANK = 0)
#define MCP_REG_IODIRA      0x00
#define MCP_REG_IODIRB      0x01
#define MCP_REG_IPOLA       0x02
#define MCP_REG_IPOLB       0x03
#define MCP_REG_GPINTENA    0x04
#define MCP_REG_GPINTENB    0x05
#define MCP_REG_DEFVALA     0x06
#define MCP_REG_DEFVALB     0x07
#define MCP_REG_INTCONA     0x08
#define MCP_REG_INTCONB     0x09
#define MCP_REG_IOCON       0x0A
#define MCP_REG_GPPUA       0x0C
#define MCP_REG_GPPUB       0x0D
#define MCP_REG_INTFA       0x0E
#define MCP_REG_INTFB       0x0F
#define MCP_REG_INTCAPA     0x10
#define MCP_REG_INTCAPB     0x11
#define MCP_REG_GPIOA       0x12
#define MCP_REG_GPIOB       0x13
#define MCP_REG_OLATA       0x14
#define MCP_REG_OLATB       0x15

class MCP23017Controller {
public:
    static MCP23017Controller& getInstance() {
        static MCP23017Controller instance;
        return instance;
    }

    bool begin(uint8_t i2cAddr = MCP23017_I2C_ADDR);
    
    // Relay / Output control (GPA0 - GPA7)
    bool setRelay(uint8_t pin, bool turnOn);
    bool getRelayState(uint8_t pin) const;
    uint8_t getRelayByte() const { return _relayState; }
    void emergencyAllOff();

    // Input Reading & Debouncing (GPB0 - GPB7)
    void updateInputs();
    bool isPinLow(uint8_t pin) const;     // Input pin grounded
    bool isPinHigh(uint8_t pin) const;
    
    // High-Level Sensor Status Methods
    bool isFlowSwitchClosed() const;     // Water flowing through heater tube
    bool isHighLimitOk() const;          // High limit switch closed (NC)
    bool isWaterLevelOk() const;         // Water level above minimum float
    bool isCoverClosed() const;          // Spa cover on
    
    // Pushbutton triggers (falling-edge detect with debounce)
    bool wasJetButtonPressed();
    bool wasBlowerButtonPressed();
    bool wasLightButtonPressed();

    bool isConnected() const { return _connected; }

private:
    MCP23017Controller();
    uint8_t _i2cAddr;
    bool _connected;
    
    uint8_t _relayState;       // Logical state of relays (1 = Active, 0 = Inactive)
    uint8_t _rawInputs;        // Instantaneous port B reading
    uint8_t _debouncedInputs;  // Filtered port B reading
    uint8_t _lastInputs;       // Previous reading for edge detection
    uint8_t _buttonPressFlags; // Latched button press events
    
    uint32_t _lastDebounceTime;
    
    bool writePortA(uint8_t val);
    uint8_t readPortB();
};

#define MCP23017 MCP23017Controller::getInstance()
