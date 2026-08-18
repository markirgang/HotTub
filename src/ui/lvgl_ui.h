#pragma once
#include <Arduino.h>

#if __has_include("include/lv_conf.h")
#include "include/lv_conf.h"
#elif __has_include("../lv_conf.h")
#include "../lv_conf.h"
#elif __has_include("lv_conf.h")
#include "lv_conf.h"
#endif

#include <lvgl.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

class LVGLUIManager {
public:
    static LVGLUIManager& getInstance() {
        static LVGLUIManager instance;
        return instance;
    }

    bool begin();
    void update(); // Periodic UI telemetry refresh (called every 200ms)

private:
    LVGLUIManager();

    void createStyles();
    void createHeader(lv_obj_t* parent);
    void createTabs(lv_obj_t* parent);
    void buildDashboardTab(lv_obj_t* parent);
    void buildPumpsTab(lv_obj_t* parent);
    void buildScheduleTab(lv_obj_t* parent);
    void buildDiagnosticsTab(lv_obj_t* parent);
    void buildSettingsTab(lv_obj_t* parent);

    // Dynamic UI Widgets
    lv_obj_t* _labelTime;
    lv_obj_t* _labelWaterTemp;
    lv_obj_t* _labelTargetTemp;
    lv_obj_t* _labelHeaterStatus;
    lv_obj_t* _labelFlowBadge;
    lv_obj_t* _labelJetTimer;
    lv_obj_t* _labelBlowerTimer;
    lv_obj_t* _labelStatusMessage;
    lv_obj_t* _arcTemp;

    // Quick control buttons
    lv_obj_t* _btnJet1;
    lv_obj_t* _labelBtnJet1;
    lv_obj_t* _btnJet2;
    lv_obj_t* _labelBtnJet2;
    lv_obj_t* _btnBlower;
    lv_obj_t* _labelBtnBlower;
    lv_obj_t* _btnLight;
    lv_obj_t* _labelBtnLight;

    // Diagnostics labels
    lv_obj_t* _labelDiagRelays;
    lv_obj_t* _labelDiagInputs;
    lv_obj_t* _labelDiagSensors;
    lv_obj_t* _labelDiagSystem;

    // Settings widgets
    lv_obj_t* _sliderBrightness;
    lv_obj_t* _switchUnits;
    lv_obj_t* _sliderCal;
    lv_obj_t* _labelCalVal;

    // Filtration widgets
    lv_obj_t* _sliderF1Start;
    lv_obj_t* _labelF1Start;
    lv_obj_t* _sliderF1Dur;
    lv_obj_t* _labelF1Dur;
    lv_obj_t* _sliderF2Start;
    lv_obj_t* _labelF2Start;
    lv_obj_t* _sliderF2Dur;
    lv_obj_t* _labelF2Dur;

    uint32_t _lastUiUpdate;
};

#define UIManager LVGLUIManager::getInstance()
