#include "mcp23017.h"
#include "i2c_bus.h"

MCP23017Controller::MCP23017Controller()
    : _i2cAddr(MCP23017_I2C_ADDR),
      _connected(false),
      _relayState(0),
      _rawInputs(0xFF),
      _debouncedInputs(0xFF),
      _lastInputs(0xFF),
      _buttonPressFlags(0),
      _lastDebounceTime(0) {
}

bool MCP23017Controller::begin(uint8_t i2cAddr) {
    _i2cAddr = i2cAddr;
    
    if (!I2CBus.ping(_i2cAddr)) {
        log_e("MCP23017 not detected at I2C 0x%02X!", _i2cAddr);
        _connected = false;
        return false;
    }

    log_i("Initializing MCP23017 at I2C address 0x%02X...", _i2cAddr);

    // 1. Configure Port Direction:
    // Port A (GPA0-GPA7): All OUTPUTS (0x00)
    // Port B (GPB0-GPB7): All INPUTS  (0xFF)
    if (!I2CBus.writeRegister8(_i2cAddr, MCP_REG_IODIRA, 0x00)) return false;
    if (!I2CBus.writeRegister8(_i2cAddr, MCP_REG_IODIRB, 0xFF)) return false;

    // 2. Enable Pull-Up Resistors on Port B (Inputs)
    if (!I2CBus.writeRegister8(_i2cAddr, MCP_REG_GPPUB, 0xFF)) return false;

    // 3. Normal input polarity (0 = Normal, not inverted)
    if (!I2CBus.writeRegister8(_i2cAddr, MCP_REG_IPOLB, 0x00)) return false;

    // 4. Initial Relay Output State: All Relays OFF
    _relayState = 0x00;
    uint8_t initialHardwareLevel = (RELAY_ACTIVE_LEVEL == LOW) ? 0xFF : 0x00;
    if (!I2CBus.writeRegister8(_i2cAddr, MCP_REG_GPIOA, initialHardwareLevel)) return false;

    _connected = true;
    log_i("MCP23017 Initialized successfully. Relays set to safe OFF state.");
    
    // Read initial inputs
    updateInputs();
    return true;
}

bool MCP23017Controller::writePortA(uint8_t logicalState) {
    if (!_connected) return false;
    
    // Convert logical state (1 = ON, 0 = OFF) to electrical output (ACTIVE_LEVEL)
    uint8_t physicalByte = 0;
    for (int i = 0; i < 8; i++) {
        bool bitOn = (logicalState & (1 << i)) != 0;
        if (RELAY_ACTIVE_LEVEL == LOW) {
            if (!bitOn) physicalByte |= (1 << i); // Inverted for Active LOW
        } else {
            if (bitOn) physicalByte |= (1 << i);
        }
    }
    
    return I2CBus.writeRegister8(_i2cAddr, MCP_REG_GPIOA, physicalByte);
}

uint8_t MCP23017Controller::readPortB() {
    if (!_connected) return 0xFF;
    bool ok = false;
    uint8_t val = I2CBus.readRegister8(_i2cAddr, MCP_REG_GPIOB, &ok);
    return ok ? val : 0xFF;
}

bool MCP23017Controller::setRelay(uint8_t pin, bool turnOn) {
    if (pin >= 8) return false;
    
    if (turnOn) {
        _relayState |= (1 << pin);
    } else {
        _relayState &= ~(1 << pin);
    }
    
    return writePortA(_relayState);
}

bool MCP23017Controller::getRelayState(uint8_t pin) const {
    if (pin >= 8) return false;
    return (_relayState & (1 << pin)) != 0;
}

void MCP23017Controller::emergencyAllOff() {
    _relayState = 0x00;
    writePortA(0x00);
    log_w("EMERGENCY: All MCP23017 Relays de-energized!");
}

void MCP23017Controller::updateInputs() {
    if (!_connected) return;
    
    uint8_t currentRaw = readPortB();
    _rawInputs = currentRaw;
    
    uint32_t now = millis();
    if (now - _lastDebounceTime >= 25) { // 25ms debounce window
        uint8_t previousDebounced = _debouncedInputs;
        _debouncedInputs = currentRaw;
        _lastDebounceTime = now;
        
        // Detect falling edges on pushbuttons (Active-LOW button press: transition from 1 -> 0)
        uint8_t fallingEdges = previousDebounced & (~_debouncedInputs);
        _buttonPressFlags |= fallingEdges;
    }
}

bool MCP23017Controller::isPinLow(uint8_t pin) const {
    if (pin >= 8 && pin <= 15) {
        uint8_t portBPin = pin - 8;
        return ((_debouncedInputs & (1 << portBPin)) == 0);
    }
    return false;
}

bool MCP23017Controller::isPinHigh(uint8_t pin) const {
    return !isPinLow(pin);
}

bool MCP23017Controller::isFlowSwitchClosed() const {
    // Flow switch closes circuit to GND when water flows
    return isPinLow(MCP_PIN_FLOW_SWITCH);
}

bool MCP23017Controller::isHighLimitOk() const {
    // High-limit mechanical switch is Normally Closed (NC) to GND. If tripped, circuit opens (High).
    return isPinLow(MCP_PIN_HILIMIT_SWITCH);
}

bool MCP23017Controller::isWaterLevelOk() const {
    // Float switch closes to GND when water level is normal
    return isPinLow(MCP_PIN_WATER_LEVEL);
}

bool MCP23017Controller::isCoverClosed() const {
    // Reed switch on cover closes to GND when cover is placed on spa
    return isPinLow(MCP_PIN_COVER_SWITCH);
}

bool MCP23017Controller::wasJetButtonPressed() {
    uint8_t pinBit = 1 << (MCP_PIN_BTN_JETS - 8);
    if (_buttonPressFlags & pinBit) {
        _buttonPressFlags &= ~pinBit; // Clear flag on read
        return true;
    }
    return false;
}

bool MCP23017Controller::wasBlowerButtonPressed() {
    uint8_t pinBit = 1 << (MCP_PIN_BTN_BLOWER - 8);
    if (_buttonPressFlags & pinBit) {
        _buttonPressFlags &= ~pinBit;
        return true;
    }
    return false;
}

bool MCP23017Controller::wasLightButtonPressed() {
    uint8_t pinBit = 1 << (MCP_PIN_BTN_LIGHT - 8);
    if (_buttonPressFlags & pinBit) {
        _buttonPressFlags &= ~pinBit;
        return true;
    }
    return false;
}
