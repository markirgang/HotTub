#include "spa_controller.h"
#include <ArduinoJson.h>

SpaController::SpaController()
    : _pump1(PumpSpeed::SPEED_OFF),
      _pump2(PumpSpeed::SPEED_OFF),
      _blower(BlowerSpeed::SPEED_OFF),
      _blowerPwm(0),
      _heaterState(HeaterState::HEATER_OFF),
      _ozone(false),
      _light(false),
      _lightBrightness(100),
      _mode(SpaMode::MODE_STANDARD),
      _flowVerifiedStartTime(0),
      _heaterOffTimestamp(0),
      _heaterCooldownStartTime(0),
      _jet1HighStartTime(0),
      _blowerStartTime(0),
      _cleanCycleStartTime(0),
      _cleanCycleRunning(false),
      _freezeProtectActive(false),
      _lastError("OK"),
      _lastLoopTime(0) {
}

bool SpaController::begin() {
    log_i("Initializing Spa Controller Engine...");
    
    _mode = Config.getOperatingMode();
    _lastError = "OK";
    _heaterOffTimestamp = millis() - (HEATER_MIN_OFF_TIME_SEC * 1000); // Allow immediate heating if needed
    
    // Ensure all relays start OFF
    updateHardwareOutputs();
    return true;
}

void SpaController::setTargetTemperatureF(float tempF) {
    Config.setTargetTempF(tempF);
    log_i("Target temperature set to: %.1f F", tempF);
}

void SpaController::setPump1Speed(PumpSpeed speed) {
    _pump1 = speed;
    if (speed == PumpSpeed::SPEED_HIGH) {
        _jet1HighStartTime = millis();
        log_i("Jet Pump 1 set to HIGH (15-min auto-off timer active)");
    } else if (speed == PumpSpeed::SPEED_LOW) {
        _jet1HighStartTime = 0;
        log_i("Jet Pump 1 set to LOW (Circulation)");
    } else {
        _jet1HighStartTime = 0;
        log_i("Jet Pump 1 turned OFF");
    }
}

void SpaController::setPump2Speed(PumpSpeed speed) {
    _pump2 = speed;
    log_i("Jet Pump 2 set to: %d", (int)speed);
}

void SpaController::setBlowerSpeed(BlowerSpeed speed) {
    _blower = speed;
    if (speed != BlowerSpeed::SPEED_OFF) {
        _blowerStartTime = millis();
        switch (speed) {
            case BlowerSpeed::SPEED_LOW: _blowerPwm = 40; break;
            case BlowerSpeed::SPEED_MED: _blowerPwm = 70; break;
            case BlowerSpeed::SPEED_HIGH: _blowerPwm = 100; break;
            default: break;
        }
        PWMControl.setBlowerSpeedPercent(_blowerPwm);
        log_i("Air Blower set to speed %d (%d%% PWM)", (int)speed, _blowerPwm);
    } else {
        _blowerStartTime = 0;
        _blowerPwm = 0;
        PWMControl.setBlowerSpeedPercent(0);
        log_i("Air Blower turned OFF");
    }
}

void SpaController::setBlowerPercent(uint8_t percent) {
    _blowerPwm = (percent > 100) ? 100 : percent;
    if (_blowerPwm > 0) {
        _blowerStartTime = millis();
        if (_blowerPwm < 50) _blower = BlowerSpeed::SPEED_LOW;
        else if (_blowerPwm < 85) _blower = BlowerSpeed::SPEED_MED;
        else _blower = BlowerSpeed::SPEED_HIGH;
    } else {
        _blowerStartTime = 0;
        _blower = BlowerSpeed::SPEED_OFF;
    }
    PWMControl.setBlowerSpeedPercent(_blowerPwm);
}

void SpaController::setLightState(bool on) {
    _light = on;
    if (on) {
        PWMControl.setLightBrightnessPercent(_lightBrightness);
    } else {
        PWMControl.setLightBrightnessPercent(0);
    }
    log_i("Spa Light set to: %s", on ? "ON" : "OFF");
}

void SpaController::setLightBrightness(uint8_t percent) {
    _lightBrightness = (percent > 100) ? 100 : percent;
    if (_light) {
        PWMControl.setLightBrightnessPercent(_lightBrightness);
    }
}

void SpaController::setOperatingMode(SpaMode mode) {
    _mode = mode;
    Config.setOperatingMode(mode);
    
    if (mode == SpaMode::MODE_PARTY) {
        setPump1Speed(PumpSpeed::SPEED_HIGH);
        setPump2Speed(PumpSpeed::SPEED_HIGH);
        setBlowerSpeed(BlowerSpeed::SPEED_HIGH);
        setLightState(true);
    } else if (mode == SpaMode::MODE_CLEAN) {
        startCleanCycle();
    }
    
    log_i("Operating mode changed to: %d", (int)mode);
}

void SpaController::startCleanCycle() {
    _cleanCycleRunning = true;
    _cleanCycleStartTime = millis();
    setPump1Speed(PumpSpeed::SPEED_HIGH);
    setBlowerSpeed(BlowerSpeed::SPEED_HIGH);
    _ozone = true;
    log_i("10-Minute Spa Clean / Filtration Purge Cycle Started.");
}

void SpaController::cancelAllTimers() {
    _jet1HighStartTime = 0;
    _blowerStartTime = 0;
    _cleanCycleRunning = false;
    setPump1Speed(PumpSpeed::SPEED_OFF);
    setPump2Speed(PumpSpeed::SPEED_OFF);
    setBlowerSpeed(BlowerSpeed::SPEED_OFF);
}

void SpaController::emergencyShutdown() {
    log_e("CRITICAL: Emergency Spa Shutdown Triggered!");
    _pump1 = PumpSpeed::SPEED_OFF;
    _pump2 = PumpSpeed::SPEED_OFF;
    _blower = BlowerSpeed::SPEED_OFF;
    _heaterState = HeaterState::FAULT_OVERHEAT;
    _ozone = false;
    _light = false;
    _cleanCycleRunning = false;
    _lastError = "EMERGENCY SHUTDOWN";
    MCP23017.emergencyAllOff();
    PWMControl.setBlowerSpeedPercent(0);
    PWMControl.setLightBrightnessPercent(0);
}

void SpaController::updatePhysicalButtons() {
    // Jet Button: Cycle Off -> Low -> High -> Off
    if (MCP23017.wasJetButtonPressed()) {
        if (_pump1 == PumpSpeed::SPEED_OFF) {
            setPump1Speed(PumpSpeed::SPEED_LOW);
        } else if (_pump1 == PumpSpeed::SPEED_LOW) {
            setPump1Speed(PumpSpeed::SPEED_HIGH);
        } else {
            setPump1Speed(PumpSpeed::SPEED_OFF);
        }
    }

    // Blower Button: Cycle Off -> Low -> Med -> High -> Off
    if (MCP23017.wasBlowerButtonPressed()) {
        if (_blower == BlowerSpeed::SPEED_OFF) {
            setBlowerSpeed(BlowerSpeed::SPEED_LOW);
        } else if (_blower == BlowerSpeed::SPEED_LOW) {
            setBlowerSpeed(BlowerSpeed::SPEED_MED);
        } else if (_blower == BlowerSpeed::SPEED_MED) {
            setBlowerSpeed(BlowerSpeed::SPEED_HIGH);
        } else {
            setBlowerSpeed(BlowerSpeed::SPEED_OFF);
        }
    }

    // Light Button: Toggle On / Off
    if (MCP23017.wasLightButtonPressed()) {
        setLightState(!_light);
    }
}

void SpaController::updateTimers() {
    uint32_t now = millis();

    // Jet 1 High 15-min auto-off timer
    if (_pump1 == PumpSpeed::SPEED_HIGH && _jet1HighStartTime > 0) {
        if (now - _jet1HighStartTime >= (Config.getJetTimeoutSeconds() * 1000)) {
            log_i("Jet 1 High timer expired. Reverting to Low/Circulation.");
            setPump1Speed(PumpSpeed::SPEED_LOW);
        }
    }

    // Air Blower 15-min auto-off timer
    if (_blower != BlowerSpeed::SPEED_OFF && _blowerStartTime > 0) {
        if (now - _blowerStartTime >= (Config.getBlowerTimeoutSeconds() * 1000)) {
            log_i("Air Blower timer expired. Turning off.");
            setBlowerSpeed(BlowerSpeed::SPEED_OFF);
        }
    }

    // Clean cycle (10 minutes)
    if (_cleanCycleRunning) {
        if (now - _cleanCycleStartTime >= (10 * 60 * 1000)) {
            log_i("Clean cycle finished. Reverting to normal operation.");
            _cleanCycleRunning = false;
            setPump1Speed(PumpSpeed::SPEED_OFF);
            setBlowerSpeed(BlowerSpeed::SPEED_OFF);
            _mode = Config.getOperatingMode();
        }
    }
}

void SpaController::updateSafetyInterlocks() {
    float waterTempF = TempSensor.getWaterTempF();
    bool sensorValid = TempSensor.isSensorValid();
    bool flowOk = MCP23017.isFlowSwitchClosed();
    bool highLimitOk = MCP23017.isHighLimitOk();
    bool waterLevelOk = MCP23017.isWaterLevelOk();

    // 1. Critical High-Limit / Over-Temperature Hard Trip
    if (waterTempF >= TEMP_EMERGENCY_TRIP_F || !highLimitOk) {
        _lastError = !highLimitOk ? "ERR: HIGH LIMIT TRIPPED" : "ERR: WATER OVERHEAT TRIP (108F)";
        emergencyShutdown();
        return;
    }

    // 2. Sensor Failure Protection
    if (!sensorValid) {
        _lastError = "ERR: TEMP SENSOR FAULT";
        if (_heaterState == HeaterState::HEATER_ON || _heaterState == HeaterState::HEATER_PRE_FLOW) {
            _heaterState = HeaterState::FAULT_SENSOR;
        }
        return;
    }

    // 3. Low Water Level
    if (!waterLevelOk) {
        _lastError = "WARN: LOW WATER LEVEL";
        if (_heaterState == HeaterState::HEATER_ON) {
            _heaterState = HeaterState::HEATER_OFF;
        }
        return;
    }

    // 4. Freeze Protection Logic
    if (waterTempF <= TEMP_FREEZE_PROTECT_F) {
        if (!_freezeProtectActive) {
            _freezeProtectActive = true;
            _lastError = "INFO: FREEZE PROTECTION ACTIVE";
            log_w("Freeze protection triggered (Water Temp: %.1f F)!", waterTempF);
        }
    } else if (waterTempF >= 55.0f && _freezeProtectActive) {
        _freezeProtectActive = false;
        _lastError = "OK";
        log_i("Freeze protection restored to normal.");
    }
}

void SpaController::updateTemperatureRegulation() {
    uint32_t now = millis();
    float curTemp = TempSensor.getWaterTempF();
    float targetTemp = Config.getTargetTempF();
    bool flowSwitch = MCP23017.isFlowSwitchClosed();
    bool heaterAllowedByMode = true;

    // Check operating mode restrictions
    if (_mode == SpaMode::MODE_ECO && !SpaScheduler.isEcoHeatingAllowed()) {
        heaterAllowedByMode = false;
    }

    // Freeze protection overrides schedule
    if (_freezeProtectActive) {
        heaterAllowedByMode = true;
        targetTemp = 60.0f; // Target 60F to prevent freezing
    }

    bool tempNeedsHeat = (curTemp < (targetTemp - (TEMP_HYSTERESIS_F / 2.0f)));
    bool tempReachedSetpoint = (curTemp >= targetTemp);

    // Anti-short-cycle delay check (2 minutes)
    bool antiShortCycleOk = (now - _heaterOffTimestamp >= (HEATER_MIN_OFF_TIME_SEC * 1000));

    // Automated Filtration / Circulation demands pump running
    bool filterRunning = SpaScheduler.isFilterCycleActive() || SpaScheduler.isPurgeCycleActive() || _cleanCycleRunning;

    // State machine for Heater & Circulation
    switch (_heaterState) {
        case HeaterState::HEATER_OFF:
        case HeaterState::FAULT_NO_FLOW:
        case HeaterState::FAULT_OVERHEAT:
        case HeaterState::FAULT_SENSOR:
            if (tempNeedsHeat && heaterAllowedByMode && antiShortCycleOk && TempSensor.isSensorValid() && MCP23017.isWaterLevelOk()) {
                // To heat, we must first start the pump on Low to establish flow
                if (_pump1 == PumpSpeed::SPEED_OFF) {
                    setPump1Speed(PumpSpeed::SPEED_LOW);
                }
                
                // Check if flow switch is closed
                if (flowSwitch) {
                    if (_flowVerifiedStartTime == 0) {
                        _flowVerifiedStartTime = now;
                    }
                    _heaterState = HeaterState::HEATER_PRE_FLOW;
                } else {
                    _flowVerifiedStartTime = 0;
                }
            } else {
                _flowVerifiedStartTime = 0;
                // If not heating, ensure pump speed matches user/filter requests
                if (filterRunning && _pump1 == PumpSpeed::SPEED_OFF) {
                    setPump1Speed(PumpSpeed::SPEED_LOW);
                }
            }
            break;

        case HeaterState::HEATER_PRE_FLOW:
            // Ensure pump is circulating
            if (_pump1 == PumpSpeed::SPEED_OFF) {
                setPump1Speed(PumpSpeed::SPEED_LOW);
            }

            if (!flowSwitch) {
                _heaterState = HeaterState::FAULT_NO_FLOW;
                _lastError = "ERR: NO FLOW DETECTED";
                _flowVerifiedStartTime = 0;
            } else if (now - _flowVerifiedStartTime >= (HEATER_FLOW_VERIFY_SEC * 1000)) {
                // Flow switch has been verified continuously for 15s -> Safe to energize heater!
                _heaterState = HeaterState::HEATER_ON;
                _lastError = "OK";
                log_i("Heater energized (Current: %.1f F, Target: %.1f F)", curTemp, targetTemp);
            }
            break;

        case HeaterState::HEATER_ON:
            // Instant Flow Loss Fault
            if (!flowSwitch) {
                _heaterState = HeaterState::FAULT_NO_FLOW;
                _heaterOffTimestamp = now;
                _lastError = "ERR: FLOW LOST DURING HEAT";
                log_w("Heater immediately tripped: Flow switch opened!");
                break;
            }

            // High Temp Trip (> 104.5°F safety buffer)
            if (curTemp >= TEMP_OVERHEAT_LIMIT_F) {
                _heaterState = HeaterState::FAULT_OVERHEAT;
                _heaterOffTimestamp = now;
                _lastError = "ERR: TEMP OVERHEAT";
                break;
            }

            // Reached setpoint or mode heating disallowed -> Initiate Cooldown cycle
            if (tempReachedSetpoint || !heaterAllowedByMode) {
                log_i("Setpoint reached (%.1f F). Starting 30s heater cooldown flush...", curTemp);
                _heaterState = HeaterState::HEATER_COOLDOWN;
                _heaterCooldownStartTime = now;
                _heaterOffTimestamp = now;
            }
            break;

        case HeaterState::HEATER_COOLDOWN:
            // Maintain pump circulation for 30s to flush residual heat from heater tube
            if (_pump1 == PumpSpeed::SPEED_OFF) {
                setPump1Speed(PumpSpeed::SPEED_LOW);
            }

            if (now - _heaterCooldownStartTime >= (HEATER_COOLDOWN_SEC * 1000)) {
                _heaterState = HeaterState::HEATER_OFF;
                log_i("Heater cooldown cycle complete.");
                
                // If no user high jet and no filter cycle, turn pump off
                if (!filterRunning && _jet1HighStartTime == 0 && !_cleanCycleRunning) {
                    setPump1Speed(PumpSpeed::SPEED_OFF);
                }
            }
            break;
    }

    // Sync Ozone with pump running during filter cycles or clean mode
    _ozone = (_pump1 != PumpSpeed::SPEED_OFF) && (filterRunning || _cleanCycleRunning);
}

void SpaController::updateHardwareOutputs() {
    // 1. Jet Pump 1 Relays (Interlocked GPA0 & GPA1)
    if (_pump1 == PumpSpeed::SPEED_LOW) {
        MCP23017.setRelay(MCP_PIN_PUMP1_HIGH, false);
        MCP23017.setRelay(MCP_PIN_PUMP1_LOW, true);
    } else if (_pump1 == PumpSpeed::SPEED_HIGH) {
        MCP23017.setRelay(MCP_PIN_PUMP1_LOW, false);
        MCP23017.setRelay(MCP_PIN_PUMP1_HIGH, true);
    } else {
        MCP23017.setRelay(MCP_PIN_PUMP1_LOW, false);
        MCP23017.setRelay(MCP_PIN_PUMP1_HIGH, false);
    }

    // 2. Jet Pump 2 Relay (GPA2)
    MCP23017.setRelay(MCP_PIN_PUMP2_HIGH, _pump2 == PumpSpeed::SPEED_HIGH);

    // 3. Air Blower Relays (GPA3 & GPA4)
    if (_blower == BlowerSpeed::SPEED_LOW) {
        MCP23017.setRelay(MCP_PIN_BLOWER_HIGH, false);
        MCP23017.setRelay(MCP_PIN_BLOWER_LOW, true);
    } else if (_blower == BlowerSpeed::SPEED_MED || _blower == BlowerSpeed::SPEED_HIGH) {
        MCP23017.setRelay(MCP_PIN_BLOWER_LOW, false);
        MCP23017.setRelay(MCP_PIN_BLOWER_HIGH, true);
    } else {
        MCP23017.setRelay(MCP_PIN_BLOWER_LOW, false);
        MCP23017.setRelay(MCP_PIN_BLOWER_HIGH, false);
    }

    // 4. Heater Contactor Relay (GPA5)
    MCP23017.setRelay(MCP_PIN_HEATER, (_heaterState == HeaterState::HEATER_ON));

    // 5. Ozone Sanitizer Relay (GPA6)
    MCP23017.setRelay(MCP_PIN_OZONE, _ozone);

    // 6. Spa Light Relay (GPA7)
    MCP23017.setRelay(MCP_PIN_LIGHT_RELAY, _light);
}

void SpaController::update() {
    uint32_t now = millis();
    if (now - _lastLoopTime < 100) return; // 100ms state machine tick
    _lastLoopTime = now;

    // 1. Poll MCP23017 inputs and buttons
    MCP23017.updateInputs();
    updatePhysicalButtons();

    // 2. Update temperature probe readings
    TempSensor.update();

    // 3. Update scheduler windows (NTP/Filtration)
    SpaScheduler.update();

    // 4. Update countdown timers
    updateTimers();

    // 5. Evaluate safety interlocks
    updateSafetyInterlocks();

    // 6. Evaluate temperature regulation state machine
    updateTemperatureRegulation();

    // 7. Write final relay outputs
    updateHardwareOutputs();
}

SpaTelemetry SpaController::getTelemetry() const {
    SpaTelemetry t;
    t.waterTempF = TempSensor.getWaterTempF();
    t.waterTempC = TempSensor.getWaterTempC();
    t.targetTempF = Config.getTargetTempF();
    t.targetTempC = (t.targetTempF - 32.0f) * 5.0f / 9.0f;
    t.pump1Speed = _pump1;
    t.pump2Speed = _pump2;
    t.blowerSpeed = _blower;
    t.blowerPercent = _blowerPwm;
    t.heaterState = _heaterState;
    t.ozoneActive = _ozone;
    t.lightActive = _light;
    t.lightBrightness = _lightBrightness;
    t.mode = _mode;
    t.flowOk = MCP23017.isFlowSwitchClosed();
    t.highLimitOk = MCP23017.isHighLimitOk();
    t.waterLevelOk = MCP23017.isWaterLevelOk();
    t.coverClosed = MCP23017.isCoverClosed();

    uint32_t now = millis();
    
    // Remaining high jet timer
    if (_pump1 == PumpSpeed::SPEED_HIGH && _jet1HighStartTime > 0) {
        uint32_t elapsed = (now - _jet1HighStartTime) / 1000;
        uint32_t total = Config.getJetTimeoutSeconds();
        t.jet1RemainingSec = (elapsed < total) ? (total - elapsed) : 0;
    } else {
        t.jet1RemainingSec = 0;
    }

    // Remaining blower timer
    if (_blower != BlowerSpeed::SPEED_OFF && _blowerStartTime > 0) {
        uint32_t elapsed = (now - _blowerStartTime) / 1000;
        uint32_t total = Config.getBlowerTimeoutSeconds();
        t.blowerRemainingSec = (elapsed < total) ? (total - elapsed) : 0;
    } else {
        t.blowerRemainingSec = 0;
    }

    // Remaining clean cycle timer
    if (_cleanCycleRunning) {
        uint32_t elapsed = (now - _cleanCycleStartTime) / 1000;
        t.cleanRemainingSec = (elapsed < 600) ? (600 - elapsed) : 0;
    } else {
        t.cleanRemainingSec = 0;
    }

    t.errorString = _lastError;
    
    if (_heaterState == HeaterState::HEATER_ON) {
        t.statusString = "Heating";
    } else if (_heaterState == HeaterState::HEATER_PRE_FLOW) {
        t.statusString = "Verifying Flow";
    } else if (_heaterState == HeaterState::HEATER_COOLDOWN) {
        t.statusString = "Cooling Down";
    } else if (SpaScheduler.isFilterCycleActive()) {
        t.statusString = "Filter Cycle Active";
    } else {
        t.statusString = "Ready";
    }

    return t;
}

String SpaController::getStatusJson() const {
    SpaTelemetry t = getTelemetry();
    JsonDocument doc;

    doc["water_temp_f"] = round(t.waterTempF * 10) / 10.0;
    doc["water_temp_c"] = round(t.waterTempC * 10) / 10.0;
    doc["target_temp_f"] = round(t.targetTempF * 10) / 10.0;
    doc["target_temp_c"] = round(t.targetTempC * 10) / 10.0;
    doc["unit"] = (Config.getTempUnit() == TemperatureUnit::CELSIUS) ? "C" : "F";
    doc["pump1"] = (int)t.pump1Speed;
    doc["pump2"] = (int)t.pump2Speed;
    doc["blower"] = (int)t.blowerSpeed;
    doc["blower_pct"] = t.blowerPercent;
    doc["heater"] = (int)t.heaterState;
    doc["ozone"] = t.ozoneActive;
    doc["light"] = t.lightActive;
    doc["light_brightness"] = t.lightBrightness;
    doc["mode"] = (int)t.mode;
    doc["flow_ok"] = t.flowOk;
    doc["high_limit_ok"] = t.highLimitOk;
    doc["water_level_ok"] = t.waterLevelOk;
    doc["cover_closed"] = t.coverClosed;
    doc["jet_remaining"] = t.jet1RemainingSec;
    doc["blower_remaining"] = t.blowerRemainingSec;
    doc["clean_remaining"] = t.cleanRemainingSec;
    doc["error"] = t.errorString;
    doc["status"] = t.statusString;
    doc["time"] = SpaScheduler.getFormattedTime();
    doc["date"] = SpaScheduler.getFormattedDate();
    doc["relays"] = MCP23017.getRelayByte();

    String out;
    serializeJson(doc, out);
    return out;
}
