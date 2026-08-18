#include "ch422g.h"
#include "i2c_bus.h"

#define CH422G_CMD_SET_IO_OUT   0x38

CH422GDriver::CH422GDriver() : _i2cAddr(CH422G_I2C_ADDR), _outputState(0xFF), _initialized(false) {
}

bool CH422GDriver::begin(uint8_t i2cAddr) {
    _i2cAddr = i2cAddr;
    
    // Default initial state: Backlight OFF, Resets HIGH
    _outputState = 0xFF;
    
    // Send configuration to CH422G
    uint8_t initData[2] = {CH422G_CMD_SET_IO_OUT, _outputState};
    if (I2CBus.writeBytes(_i2cAddr, initData, 2)) {
        _initialized = true;
        log_i("CH422G IO Expander initialized at 0x%02X", _i2cAddr);
    } else {
        // Fallback for some revisions responding directly to write
        if (I2CBus.writeRegister8(_i2cAddr, 0x00, _outputState)) {
            _initialized = true;
            log_i("CH422G IO Expander initialized at 0x%02X (direct mode)", _i2cAddr);
        } else {
            log_w("CH422G not responding at 0x%02X (check board revision / I2C bus)", _i2cAddr);
        }
    }
    
    return _initialized;
}

void CH422GDriver::writeOutputState() {
    uint8_t data[2] = {CH422G_CMD_SET_IO_OUT, _outputState};
    I2CBus.writeBytes(_i2cAddr, data, 2);
}

void CH422GDriver::setOutputPin(uint8_t pinIndex, bool level) {
    if (level) {
        _outputState |= (1 << pinIndex);
    } else {
        _outputState &= ~(1 << pinIndex);
    }
    writeOutputState();
}

void CH422GDriver::setBacklight(bool enable) {
    // EXIO2 controls Backlight on Waveshare 7-inch board
    setOutputPin(CH422G_PIN_LCD_BL, enable);
}

void CH422GDriver::setTouchReset(bool level) {
    // EXIO1 controls Touch Reset
    setOutputPin(CH422G_PIN_TP_RST, level);
}

void CH422GDriver::setLcdReset(bool level) {
    // EXIO3 controls LCD Reset
    setOutputPin(CH422G_PIN_LCD_RST, level);
}

void CH422GDriver::resetDisplayAndTouch() {
    log_i("Resetting Display & Touch Controller via CH422G...");
    setLcdReset(false);
    setTouchReset(false);
    delay(20);
    setLcdReset(true);
    setTouchReset(true);
    delay(50);
}
