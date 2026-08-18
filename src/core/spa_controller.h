#pragma once
#include <Arduino.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

#if __has_include("src/core/config_manager.h")
#include "src/core/config_manager.h"
#elif __has_include("config_manager.h")
#include "config_manager.h"
#endif

#if __has_include("src/hal/mcp23017.h")
#include "src/hal/mcp23017.h"
#elif __has_include("../hal/mcp23017.h")
#include "../hal/mcp23017.h"
#elif __has_include("mcp23017.h")
#include "mcp23017.h"
#endif

#if __has_include("src/hal/temp_sensor.h")
#include "src/hal/temp_sensor.h"
#elif __has_include("../hal/temp_sensor.h")
#include "../hal/temp_sensor.h"
#elif __has_include("temp_sensor.h")
#include "temp_sensor.h"
#endif

#if __has_include("src/hal/pwm_controller.h")
#include "src/hal/pwm_controller.h"
#elif __has_include("../hal/pwm_controller.h")
#include "../hal/pwm_controller.h"
#elif __has_include("pwm_controller.h")
#include "pwm_controller.h"
#endif

#if __has_include("src/core/scheduler.h")
#include "src/core/scheduler.h"
#elif __has_include("scheduler.h")
#include "scheduler.h"
#endif

enum class HeaterState : uint8_t {
    HEATER_OFF = 0,
    HEATER_PRE_FLOW = 1,    // Verifying flow switch for 15s before turning on element
    HEATER_ON = 2,          // Element energized
    HEATER_COOLDOWN = 3,    // Pump circulating 30s after heater off to dissipate heat
    FAULT_NO_FLOW = 4,      // Flow switch open error
    FAULT_OVERHEAT = 5,     // Over-temperature limit tripped (>105°F)
    FAULT_SENSOR = 6,       // Temperature sensor disconnected or fault
    FAULT_HIGH_LIMIT = 7    // Mechanical high-limit safety tripped
};

struct SpaTelemetry {
    float waterTempF;
    float waterTempC;
    float targetTempF;
    float targetTempC;
    PumpSpeed pump1Speed;
    PumpSpeed pump2Speed;
    BlowerSpeed blowerSpeed;
    uint8_t blowerPercent;
    HeaterState heaterState;
    bool ozoneActive;
    bool lightActive;
    uint8_t lightBrightness;
    SpaMode mode;
    bool flowOk;
    bool highLimitOk;
    bool waterLevelOk;
    bool coverClosed;
    uint32_t jet1RemainingSec;
    uint32_t blowerRemainingSec;
    uint32_t cleanRemainingSec;
    String errorString;
    String statusString;
};

class SpaController {
public:
    static SpaController& getInstance() {
        static SpaController instance;
        return instance;
    }

    bool begin();
    void update(); // Main 100ms state machine loop

    // User Commands
    void setTargetTemperatureF(float tempF);
    void setPump1Speed(PumpSpeed speed);
    void setPump2Speed(PumpSpeed speed);
    void setBlowerSpeed(BlowerSpeed speed);
    void setBlowerPercent(uint8_t percent);
    void setLightState(bool on);
    void setLightBrightness(uint8_t percent);
    void setOperatingMode(SpaMode mode);
    void startCleanCycle();
    void cancelAllTimers();
    void emergencyShutdown();

    // Telemetry & State
    SpaTelemetry getTelemetry() const;
    String getStatusJson() const;

private:
    SpaController();

    void updatePhysicalButtons();
    void updateSafetyInterlocks();
    void updateTemperatureRegulation();
    void updateHardwareOutputs();
    void updateTimers();

    PumpSpeed _pump1;
    PumpSpeed _pump2;
    BlowerSpeed _blower;
    uint8_t _blowerPwm;
    HeaterState _heaterState;
    bool _ozone;
    bool _light;
    uint8_t _lightBrightness;
    SpaMode _mode;

    // Safety & Interlock Timers
    uint32_t _flowVerifiedStartTime;
    uint32_t _heaterOffTimestamp;
    uint32_t _heaterCooldownStartTime;
    
    // Auto-shutoff Timers
    uint32_t _jet1HighStartTime;
    uint32_t _blowerStartTime;
    uint32_t _cleanCycleStartTime;
    
    // State Flags
    bool _cleanCycleRunning;
    bool _freezeProtectActive;
    String _lastError;
    uint32_t _lastLoopTime;
};

#define Spa SpaController::getInstance()
