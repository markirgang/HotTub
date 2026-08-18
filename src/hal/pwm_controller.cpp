#include "pwm_controller.h"

#define BLOWER_PWM_CHANNEL  0
#define LIGHT_PWM_CHANNEL   1

PWMController::PWMController()
    : _blowerPercent(0), _lightPercent(0), _initialized(false) {
}

bool PWMController::begin() {
    #if defined(PIN_BLOWER_PWM) && PIN_BLOWER_PWM >= 0
    ledcSetup(BLOWER_PWM_CHANNEL, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
    ledcAttachPin(PIN_BLOWER_PWM, BLOWER_PWM_CHANNEL);
    ledcWrite(BLOWER_PWM_CHANNEL, 0);
    #endif

    #if defined(PIN_LIGHT_PWM) && PIN_LIGHT_PWM >= 0
    ledcSetup(LIGHT_PWM_CHANNEL, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
    ledcAttachPin(PIN_LIGHT_PWM, LIGHT_PWM_CHANNEL);
    ledcWrite(LIGHT_PWM_CHANNEL, 0);
    #endif

    _initialized = true;
    log_i("PWM Controller initialized (LEDC Blower Ch0, Light Ch1)");
    return true;
}

void PWMController::setBlowerSpeedPercent(uint8_t percent) {
    if (percent > 100) percent = 100;
    _blowerPercent = percent;
    
    #if defined(PIN_BLOWER_PWM) && PIN_BLOWER_PWM >= 0
    uint32_t duty = (uint32_t)(percent * 255 / 100);
    ledcWrite(BLOWER_PWM_CHANNEL, duty);
    #endif
}

void PWMController::setLightBrightnessPercent(uint8_t percent) {
    if (percent > 100) percent = 100;
    _lightPercent = percent;
    
    #if defined(PIN_LIGHT_PWM) && PIN_LIGHT_PWM >= 0
    uint32_t duty = (uint32_t)(percent * 255 / 100);
    ledcWrite(LIGHT_PWM_CHANNEL, duty);
    #endif
}
