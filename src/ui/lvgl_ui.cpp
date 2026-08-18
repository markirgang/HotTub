#include "lvgl_ui.h"
#include "hal/display_driver.h"
#include "hal/mcp23017.h"
#include "core/spa_controller.h"
#include "core/config_manager.h"
#include "core/scheduler.h"
#include <WiFi.h>

// UI Event Callbacks
static void onTempUpClicked(lv_event_t* e) {
    float cur = Config.getTargetTempF();
    if (cur < TEMP_MAX_SETPOINT_F) {
        Spa.setTargetTemperatureF(cur + 1.0f);
    }
}

static void onTempDownClicked(lv_event_t* e) {
    float cur = Config.getTargetTempF();
    if (cur > TEMP_MIN_SETPOINT_F) {
        Spa.setTargetTemperatureF(cur - 1.0f);
    }
}

static void onJet1Clicked(lv_event_t* e) {
    SpaTelemetry t = Spa.getTelemetry();
    if (t.pump1Speed == PumpSpeed::SPEED_OFF) {
        Spa.setPump1Speed(PumpSpeed::SPEED_LOW);
    } else if (t.pump1Speed == PumpSpeed::SPEED_LOW) {
        Spa.setPump1Speed(PumpSpeed::SPEED_HIGH);
    } else {
        Spa.setPump1Speed(PumpSpeed::SPEED_OFF);
    }
}

static void onJet2Clicked(lv_event_t* e) {
    SpaTelemetry t = Spa.getTelemetry();
    Spa.setPump2Speed((t.pump2Speed == PumpSpeed::SPEED_OFF) ? PumpSpeed::SPEED_HIGH : PumpSpeed::SPEED_OFF);
}

static void onBlowerClicked(lv_event_t* e) {
    SpaTelemetry t = Spa.getTelemetry();
    if (t.blowerSpeed == BlowerSpeed::SPEED_OFF) {
        Spa.setBlowerSpeed(BlowerSpeed::SPEED_LOW);
    } else if (t.blowerSpeed == BlowerSpeed::SPEED_LOW) {
        Spa.setBlowerSpeed(BlowerSpeed::SPEED_MED);
    } else if (t.blowerSpeed == BlowerSpeed::SPEED_MED) {
        Spa.setBlowerSpeed(BlowerSpeed::SPEED_HIGH);
    } else {
        Spa.setBlowerSpeed(BlowerSpeed::SPEED_OFF);
    }
}

static void onLightClicked(lv_event_t* e) {
    SpaTelemetry t = Spa.getTelemetry();
    Spa.setLightState(!t.lightActive);
}

static void onModeSelected(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    uint32_t modeId = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    Spa.setOperatingMode((SpaMode)modeId);
}

static void onBlowerSliderChanged(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    Spa.setBlowerPercent(val);
}

static void onCleanCycleClicked(lv_event_t* e) {
    Spa.startCleanCycle();
}

static void onBrightnessChanged(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    Display.setBrightness(val);
    Config.setScreenBrightness(val);
}

static void onUnitsToggled(lv_event_t* e) {
    lv_obj_t* sw = lv_event_get_target(e);
    bool isCelsius = lv_obj_has_state(sw, LV_STATE_CHECKED);
    Config.setTempUnit(isCelsius ? TemperatureUnit::CELSIUS : TemperatureUnit::FAHRENHEIT);
}

LVGLUIManager::LVGLUIManager()
    : _labelTime(nullptr),
      _labelWaterTemp(nullptr),
      _labelTargetTemp(nullptr),
      _labelHeaterStatus(nullptr),
      _labelFlowBadge(nullptr),
      _labelJetTimer(nullptr),
      _labelBlowerTimer(nullptr),
      _labelStatusMessage(nullptr),
      _arcTemp(nullptr),
      _btnJet1(nullptr),
      _labelBtnJet1(nullptr),
      _btnJet2(nullptr),
      _labelBtnJet2(nullptr),
      _btnBlower(nullptr),
      _labelBtnBlower(nullptr),
      _btnLight(nullptr),
      _labelBtnLight(nullptr),
      _labelDiagRelays(nullptr),
      _labelDiagInputs(nullptr),
      _labelDiagSensors(nullptr),
      _labelDiagSystem(nullptr),
      _sliderBrightness(nullptr),
      _switchUnits(nullptr),
      _sliderCal(nullptr),
      _labelCalVal(nullptr),
      _sliderF1Start(nullptr),
      _labelF1Start(nullptr),
      _sliderF1Dur(nullptr),
      _labelF1Dur(nullptr),
      _sliderF2Start(nullptr),
      _labelF2Start(nullptr),
      _sliderF2Dur(nullptr),
      _labelF2Dur(nullptr),
      _lastUiUpdate(0) {
}

bool LVGLUIManager::begin() {
    if (!Display.lockUI()) return false;

    log_i("Creating LVGL User Interface for 800x480 screen...");

    // Set dark theme background
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0F141C), 0);

    // 1. Build Header Bar (Height: 45px)
    createHeader(scr);

    // 2. Build TabView
    createTabs(scr);

    Display.unlockUI();
    log_i("LVGL UI initialized successfully.");
    return true;
}

void LVGLUIManager::createHeader(lv_obj_t* parent) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, 800, 48);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x161F30), 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(header, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_border_width(header, 2, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Title / Brand
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "HOT TUB CONTROLLER");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 10, 0);

    // Status Message / Alert Banner
    _labelStatusMessage = lv_label_create(header);
    lv_label_set_text(_labelStatusMessage, "System Ready");
    lv_obj_set_style_text_color(_labelStatusMessage, lv_color_hex(0x81C784), 0);
    lv_obj_set_style_text_font(_labelStatusMessage, &lv_font_montserrat_16, 0);
    lv_obj_align(_labelStatusMessage, LV_ALIGN_CENTER, 0, 0);

    // Flow OK Badge
    _labelFlowBadge = lv_label_create(header);
    lv_label_set_text(_labelFlowBadge, "FLOW OK");
    lv_obj_set_style_text_color(_labelFlowBadge, lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_text_font(_labelFlowBadge, &lv_font_montserrat_14, 0);
    lv_obj_align(_labelFlowBadge, LV_ALIGN_RIGHT_MID, -130, 0);

    // Clock
    _labelTime = lv_label_create(header);
    lv_label_set_text(_labelTime, "12:00 PM");
    lv_obj_set_style_text_color(_labelTime, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(_labelTime, &lv_font_montserrat_18, 0);
    lv_obj_align(_labelTime, LV_ALIGN_RIGHT_MID, -10, 0);
}

void LVGLUIManager::createTabs(lv_obj_t* parent) {
    lv_obj_t* tabview = lv_tabview_create(parent, LV_DIR_BOTTOM, 55);
    lv_obj_set_size(tabview, 800, 432);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(tabview, lv_color_hex(0x0F141C), 0);

    // Tab buttons styling
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0x161F30), 0);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(0x9E9E9E), 0);
    lv_obj_set_style_text_font(tab_btns, &lv_font_montserrat_16, 0);

    // Add Tabs
    lv_obj_t* tabHome = lv_tabview_add_tab(tabview, "Dashboard");
    lv_obj_t* tabPumps = lv_tabview_add_tab(tabview, "Jets & Blower");
    lv_obj_t* tabSchedule = lv_tabview_add_tab(tabview, "Filtration");
    lv_obj_t* tabDiagnostics = lv_tabview_add_tab(tabview, "Hardware IO");
    lv_obj_t* tabSettings = lv_tabview_add_tab(tabview, "Settings");

    buildDashboardTab(tabHome);
    buildPumpsTab(tabPumps);
    buildScheduleTab(tabSchedule);
    buildDiagnosticsTab(tabDiagnostics);
    buildSettingsTab(tabSettings);
}

void LVGLUIManager::buildDashboardTab(lv_obj_t* parent) {
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // ================= LEFT: Temperature Gauge & Setpoint (400px wide) =================
    lv_obj_t* leftBox = lv_obj_create(parent);
    lv_obj_set_size(leftBox, 370, 360);
    lv_obj_align(leftBox, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_bg_color(leftBox, lv_color_hex(0x131B2A), 0);
    lv_obj_set_style_border_color(leftBox, lv_color_hex(0x23334E), 0);
    lv_obj_set_style_radius(leftBox, 16, 0);
    lv_obj_clear_flag(leftBox, LV_OBJ_FLAG_SCROLLABLE);

    // Circular Temperature Arc Gauge
    _arcTemp = lv_arc_create(leftBox);
    lv_obj_set_size(_arcTemp, 230, 230);
    lv_arc_set_rotation(_arcTemp, 135);
    lv_arc_set_bg_angles(_arcTemp, 0, 270);
    lv_arc_set_range(_arcTemp, 80, 106);
    lv_arc_set_value(_arcTemp, 100);
    lv_obj_set_style_arc_color(_arcTemp, lv_color_hex(0x2196F3), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(_arcTemp, 16, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(_arcTemp, lv_color_hex(0x1F2A3E), LV_PART_MAIN);
    lv_obj_set_style_arc_width(_arcTemp, 16, LV_PART_MAIN);
    lv_obj_remove_style(_arcTemp, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(_arcTemp, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(_arcTemp, LV_ALIGN_TOP_MID, 0, 10);

    // Current Water Temp Display inside Arc
    _labelWaterTemp = lv_label_create(_arcTemp);
    lv_label_set_text(_labelWaterTemp, "100°F");
    lv_obj_set_style_text_font(_labelWaterTemp, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(_labelWaterTemp, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(_labelWaterTemp, LV_ALIGN_CENTER, 0, -15);

    // Heater Status Badge inside Arc
    _labelHeaterStatus = lv_label_create(_arcTemp);
    lv_label_set_text(_labelHeaterStatus, "HEATER OFF");
    lv_obj_set_style_text_font(_labelHeaterStatus, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(_labelHeaterStatus, lv_color_hex(0x90CAF9), 0);
    lv_obj_align(_labelHeaterStatus, LV_ALIGN_CENTER, 0, 30);

    // Setpoint Adjustment Controls (+ / -)
    lv_obj_t* btnDown = lv_btn_create(leftBox);
    lv_obj_set_size(btnDown, 80, 60);
    lv_obj_align(btnDown, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    lv_obj_set_style_bg_color(btnDown, lv_color_hex(0x1E88E5), 0);
    lv_obj_set_style_radius(btnDown, 12, 0);
    lv_obj_t* lblDown = lv_label_create(btnDown);
    lv_label_set_text(lblDown, "-");
    lv_obj_set_style_text_font(lblDown, &lv_font_montserrat_36, 0);
    lv_obj_center(lblDown);
    lv_obj_add_event_cb(btnDown, onTempDownClicked, LV_EVENT_CLICKED, NULL);

    _labelTargetTemp = lv_label_create(leftBox);
    lv_label_set_text(_labelTargetTemp, "Set: 102°F");
    lv_obj_set_style_text_font(_labelTargetTemp, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(_labelTargetTemp, lv_color_hex(0x00E5FF), 0);
    lv_obj_align(_labelTargetTemp, LV_ALIGN_BOTTOM_MID, 0, -25);

    lv_obj_t* btnUp = lv_btn_create(leftBox);
    lv_obj_set_size(btnUp, 80, 60);
    lv_obj_align(btnUp, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    lv_obj_set_style_bg_color(btnUp, lv_color_hex(0xE53935), 0);
    lv_obj_set_style_radius(btnUp, 12, 0);
    lv_obj_t* lblUp = lv_label_create(btnUp);
    lv_label_set_text(lblUp, "+");
    lv_obj_set_style_text_font(lblUp, &lv_font_montserrat_36, 0);
    lv_obj_center(lblUp);
    lv_obj_add_event_cb(btnUp, onTempUpClicked, LV_EVENT_CLICKED, NULL);

    // ================= RIGHT: Quick Action Control Tiles (380px wide) =================
    lv_obj_t* rightBox = lv_obj_create(parent);
    lv_obj_set_size(rightBox, 380, 360);
    lv_obj_align(rightBox, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(rightBox, lv_color_hex(0x131B2A), 0);
    lv_obj_set_style_border_color(rightBox, lv_color_hex(0x23334E), 0);
    lv_obj_set_style_radius(rightBox, 16, 0);
    lv_obj_clear_flag(rightBox, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Jet 1 Button
    _btnJet1 = lv_btn_create(rightBox);
    lv_obj_set_size(_btnJet1, 165, 80);
    lv_obj_align(_btnJet1, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(_btnJet1, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_radius(_btnJet1, 12, 0);
    _labelBtnJet1 = lv_label_create(_btnJet1);
    lv_label_set_text(_labelBtnJet1, "JETS 1\nOFF");
    lv_obj_set_style_text_align(_labelBtnJet1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(_labelBtnJet1, &lv_font_montserrat_16, 0);
    lv_obj_center(_labelBtnJet1);
    lv_obj_add_event_cb(_btnJet1, onJet1Clicked, LV_EVENT_CLICKED, NULL);

    // 2. Jet 2 Button
    _btnJet2 = lv_btn_create(rightBox);
    lv_obj_set_size(_btnJet2, 165, 80);
    lv_obj_align(_btnJet2, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_bg_color(_btnJet2, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_radius(_btnJet2, 12, 0);
    _labelBtnBtn2:
    _labelBtnJet2 = lv_label_create(_btnJet2);
    lv_label_set_text(_labelBtnJet2, "JETS 2\nOFF");
    lv_obj_set_style_text_align(_labelBtnJet2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(_labelBtnJet2, &lv_font_montserrat_16, 0);
    lv_obj_center(_labelBtnJet2);
    lv_obj_add_event_cb(_btnJet2, onJet2Clicked, LV_EVENT_CLICKED, NULL);

    // 3. Air Blower Button
    _btnBlower = lv_btn_create(rightBox);
    lv_obj_set_size(_btnBlower, 165, 80);
    lv_obj_align(_btnBlower, LV_ALIGN_TOP_LEFT, 5, 95);
    lv_obj_set_style_bg_color(_btnBlower, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_radius(_btnBlower, 12, 0);
    _labelBtnBlower = lv_label_create(_btnBlower);
    lv_label_set_text(_labelBtnBlower, "BLOWER\nOFF");
    lv_obj_set_style_text_align(_labelBtnBlower, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(_labelBtnBlower, &lv_font_montserrat_16, 0);
    lv_obj_center(_labelBtnBlower);
    lv_obj_add_event_cb(_btnBlower, onBlowerClicked, LV_EVENT_CLICKED, NULL);

    // 4. Spa Light Button
    _btnLight = lv_btn_create(rightBox);
    lv_obj_set_size(_btnLight, 165, 80);
    lv_obj_align(_btnLight, LV_ALIGN_TOP_RIGHT, -5, 95);
    lv_obj_set_style_bg_color(_btnLight, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_radius(_btnLight, 12, 0);
    _labelBtnLight = lv_label_create(_btnLight);
    lv_label_set_text(_labelBtnLight, "LIGHT\nOFF");
    lv_obj_set_style_text_align(_labelBtnLight, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(_labelBtnLight, &lv_font_montserrat_16, 0);
    lv_obj_center(_labelBtnLight);
    lv_obj_add_event_cb(_btnLight, onLightClicked, LV_EVENT_CLICKED, NULL);

    // 5. Operating Mode Selector Segment
    lv_obj_t* modeLabel = lv_label_create(rightBox);
    lv_label_set_text(modeLabel, "OPERATING MODE");
    lv_obj_set_style_text_color(modeLabel, lv_color_hex(0x90CAF9), 0);
    lv_obj_set_style_text_font(modeLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(modeLabel, LV_ALIGN_TOP_LEFT, 10, 190);

    const char* modeNames[4] = {"Standard", "Eco", "Party", "Clean"};
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btnMode = lv_btn_create(rightBox);
        lv_obj_set_size(btnMode, 80, 45);
        lv_obj_align(btnMode, LV_ALIGN_TOP_LEFT, 5 + (i * 88), 215);
        lv_obj_set_style_bg_color(btnMode, (i == 0) ? lv_color_hex(0x0288D1) : lv_color_hex(0x1F2A3E), 0);
        lv_obj_set_style_radius(btnMode, 8, 0);
        
        lv_obj_t* lbl = lv_label_create(btnMode);
        lv_label_set_text(lbl, modeNames[i]);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
        lv_obj_center(lbl);
        
        lv_obj_add_event_cb(btnMode, onModeSelected, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }

    // Timers info
    _labelJetTimer = lv_label_create(rightBox);
    lv_label_set_text(_labelJetTimer, "Jet Timer: --");
    lv_obj_set_style_text_font(_labelJetTimer, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(_labelJetTimer, lv_color_hex(0x81C784), 0);
    lv_obj_align(_labelJetTimer, LV_ALIGN_BOTTOM_LEFT, 10, -10);

    _labelBlowerTimer = lv_label_create(rightBox);
    lv_label_set_text(_labelBlowerTimer, "Blower Timer: --");
    lv_obj_set_style_text_font(_labelBlowerTimer, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(_labelBlowerTimer, lv_color_hex(0x81C784), 0);
    lv_obj_align(_labelBlowerTimer, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
}

void LVGLUIManager::buildPumpsTab(lv_obj_t* parent) {
    // Air Blower Variable Speed Slider
    lv_obj_t* titleBlower = lv_label_create(parent);
    lv_label_set_text(titleBlower, "AIR BLOWER VARIABLE SPEED (PWM / TRIAC)");
    lv_obj_set_style_text_color(titleBlower, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(titleBlower, &lv_font_montserrat_16, 0);
    lv_obj_align(titleBlower, LV_ALIGN_TOP_LEFT, 20, 20);

    lv_obj_t* sliderBlower = lv_slider_create(parent);
    lv_obj_set_size(sliderBlower, 700, 30);
    lv_slider_set_range(sliderBlower, 0, 100);
    lv_slider_set_value(sliderBlower, 0, LV_ANIM_OFF);
    lv_obj_align(sliderBlower, LV_ALIGN_TOP_LEFT, 20, 55);
    lv_obj_add_event_cb(sliderBlower, onBlowerSliderChanged, LV_EVENT_VALUE_CHANGED, NULL);

    // Clean Cycle Button
    lv_obj_t* btnClean = lv_btn_create(parent);
    lv_obj_set_size(btnClean, 340, 60);
    lv_obj_align(btnClean, LV_ALIGN_TOP_LEFT, 20, 120);
    lv_obj_set_style_bg_color(btnClean, lv_color_hex(0x00897B), 0);
    lv_obj_set_style_radius(btnClean, 12, 0);
    lv_obj_t* lblClean = lv_label_create(btnClean);
    lv_label_set_text(lblClean, "START 10-MIN CLEAN PURGE");
    lv_obj_set_style_text_font(lblClean, &lv_font_montserrat_16, 0);
    lv_obj_center(lblClean);
    lv_obj_add_event_cb(btnClean, onCleanCycleClicked, LV_EVENT_CLICKED, NULL);

    // Cancel All Timers
    lv_obj_t* btnCancel = lv_btn_create(parent);
    lv_obj_set_size(btnCancel, 340, 60);
    lv_obj_align(btnCancel, LV_ALIGN_TOP_RIGHT, -20, 120);
    lv_obj_set_style_bg_color(btnCancel, lv_color_hex(0x546E7A), 0);
    lv_obj_set_style_radius(btnCancel, 12, 0);
    lv_obj_t* lblCancel = lv_label_create(btnCancel);
    lv_label_set_text(lblCancel, "ALL OFF / RESET TIMERS");
    lv_obj_set_style_text_font(lblCancel, &lv_font_montserrat_16, 0);
    lv_obj_center(lblCancel);
    lv_obj_add_event_cb(btnCancel, [](lv_event_t* e) { Spa.cancelAllTimers(); }, LV_EVENT_CLICKED, NULL);
}

void LVGLUIManager::buildScheduleTab(lv_obj_t* parent) {
    lv_obj_t* title = lv_label_create(parent);
    lv_label_set_text(title, "AUTOMATIC CIRCULATION & FILTRATION CYCLES");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 15);

    // Filter Cycle 1
    _labelF1Start = lv_label_create(parent);
    lv_label_set_text(_labelF1Start, "Cycle 1: 08:00 AM (Duration: 2.0 Hours)");
    lv_obj_set_style_text_font(_labelF1Start, &lv_font_montserrat_16, 0);
    lv_obj_align(_labelF1Start, LV_ALIGN_TOP_LEFT, 20, 50);

    _sliderF1Start = lv_slider_create(parent);
    lv_obj_set_size(_sliderF1Start, 700, 20);
    lv_slider_set_range(_sliderF1Start, 0, 23);
    lv_slider_set_value(_sliderF1Start, 8, LV_ANIM_OFF);
    lv_obj_align(_sliderF1Start, LV_ALIGN_TOP_LEFT, 20, 80);

    // Filter Cycle 2
    _labelF2Start = lv_label_create(parent);
    lv_label_set_text(_labelF2Start, "Cycle 2: 08:00 PM (Duration: 2.0 Hours)");
    lv_obj_set_style_text_font(_labelF2Start, &lv_font_montserrat_16, 0);
    lv_obj_align(_labelF2Start, LV_ALIGN_TOP_LEFT, 20, 125);

    _sliderF2Start = lv_slider_create(parent);
    lv_obj_set_size(_sliderF2Start, 700, 20);
    lv_slider_set_range(_sliderF2Start, 0, 23);
    lv_slider_set_value(_sliderF2Start, 20, LV_ANIM_OFF);
    lv_obj_align(_sliderF2Start, LV_ALIGN_TOP_LEFT, 20, 155);

    lv_obj_t* note = lv_label_create(parent);
    lv_label_set_text(note, "Note: Ozone/UV sanitizer activates automatically during filtration cycles.");
    lv_obj_set_style_text_color(note, lv_color_hex(0x9E9E9E), 0);
    lv_obj_set_style_text_font(note, &lv_font_montserrat_14, 0);
    lv_obj_align(note, LV_ALIGN_BOTTOM_LEFT, 20, -20);
}

void LVGLUIManager::buildDiagnosticsTab(lv_obj_t* parent) {
    lv_obj_t* title = lv_label_create(parent);
    lv_label_set_text(title, "HARDWARE I/O & RELAY DIAGNOSTICS");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 15);

    _labelDiagRelays = lv_label_create(parent);
    lv_label_set_text(_labelDiagRelays, "MCP23017 Relays: [1:OFF] [2:OFF] [3:OFF] [4:OFF] [5:OFF] [6:OFF] [7:OFF] [8:OFF]");
    lv_obj_set_style_text_font(_labelDiagRelays, &lv_font_montserrat_14, 0);
    lv_obj_align(_labelDiagRelays, LV_ALIGN_TOP_LEFT, 20, 50);

    _labelDiagInputs = lv_label_create(parent);
    lv_label_set_text(_labelDiagInputs, "Safety Inputs: Flow: OK | Hi-Limit: OK | Water Level: OK | Cover: CLOSED");
    lv_obj_set_style_text_font(_labelDiagInputs, &lv_font_montserrat_14, 0);
    lv_obj_align(_labelDiagInputs, LV_ALIGN_TOP_LEFT, 20, 85);

    _labelDiagSensors = lv_label_create(parent);
    lv_label_set_text(_labelDiagSensors, "DS18B20 Temp: 100.0 F | Offset: +0.0 F | Status: Connected");
    lv_obj_set_style_text_font(_labelDiagSensors, &lv_font_montserrat_14, 0);
    lv_obj_align(_labelDiagSensors, LV_ALIGN_TOP_LEFT, 20, 120);

    _labelDiagSystem = lv_label_create(parent);
    lv_label_set_text(_labelDiagSystem, "System: Free Heap: 220KB | Free PSRAM: 7.5MB | IP: 192.168.1.100");
    lv_obj_set_style_text_font(_labelDiagSystem, &lv_font_montserrat_14, 0);
    lv_obj_align(_labelDiagSystem, LV_ALIGN_TOP_LEFT, 20, 155);

    // Manual Emergency Shutdown Button
    lv_obj_t* btnEmergency = lv_btn_create(parent);
    lv_obj_set_size(btnEmergency, 700, 50);
    lv_obj_align(btnEmergency, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btnEmergency, lv_color_hex(0xB71C1C), 0);
    lv_obj_set_style_radius(btnEmergency, 12, 0);
    lv_obj_t* lblEmerg = lv_label_create(btnEmergency);
    lv_label_set_text(lblEmerg, "EMERGENCY HARDWARE STOP");
    lv_obj_set_style_text_font(lblEmerg, &lv_font_montserrat_18, 0);
    lv_obj_center(lblEmerg);
    lv_obj_add_event_cb(btnEmergency, [](lv_event_t* e) { Spa.emergencyShutdown(); }, LV_EVENT_CLICKED, NULL);
}

void LVGLUIManager::buildSettingsTab(lv_obj_t* parent) {
    // Screen Brightness
    lv_obj_t* lblBr = lv_label_create(parent);
    lv_label_set_text(lblBr, "Screen Brightness:");
    lv_obj_set_style_text_font(lblBr, &lv_font_montserrat_16, 0);
    lv_obj_align(lblBr, LV_ALIGN_TOP_LEFT, 20, 20);

    _sliderBrightness = lv_slider_create(parent);
    lv_obj_set_size(_sliderBrightness, 500, 25);
    lv_slider_set_range(_sliderBrightness, 10, 100);
    lv_slider_set_value(_sliderBrightness, Config.getScreenBrightness(), LV_ANIM_OFF);
    lv_obj_align(_sliderBrightness, LV_ALIGN_TOP_LEFT, 200, 20);
    lv_obj_add_event_cb(_sliderBrightness, onBrightnessChanged, LV_EVENT_VALUE_CHANGED, NULL);

    // Temperature Units Switch (°F vs °C)
    lv_obj_t* lblUnits = lv_label_create(parent);
    lv_label_set_text(lblUnits, "Temperature Units (°F / °C):");
    lv_obj_set_style_text_font(lblUnits, &lv_font_montserrat_16, 0);
    lv_obj_align(lblUnits, LV_ALIGN_TOP_LEFT, 20, 70);

    _switchUnits = lv_switch_create(parent);
    lv_obj_align(_switchUnits, LV_ALIGN_TOP_LEFT, 280, 65);
    if (Config.getTempUnit() == TemperatureUnit::CELSIUS) {
        lv_obj_add_state(_switchUnits, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(_switchUnits, onUnitsToggled, LV_EVENT_VALUE_CHANGED, NULL);

    // WiFi Information
    lv_obj_t* lblWifi = lv_label_create(parent);
    lv_label_set_text(lblWifi, "WiFi AP: 'HotTub-Spa-AP' | IP: 192.168.4.1 (Setup: connect and visit http://192.168.4.1)");
    lv_obj_set_style_text_color(lblWifi, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(lblWifi, &lv_font_montserrat_14, 0);
    lv_obj_align(lblWifi, LV_ALIGN_TOP_LEFT, 20, 130);

    // Reset Factory Defaults Button
    lv_obj_t* btnReset = lv_btn_create(parent);
    lv_obj_set_size(btnReset, 250, 45);
    lv_obj_align(btnReset, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(btnReset, lv_color_hex(0x455A64), 0);
    lv_obj_t* lblReset = lv_label_create(btnReset);
    lv_label_set_text(lblReset, "Reset All Settings");
    lv_obj_center(lblReset);
    lv_obj_add_event_cb(btnReset, [](lv_event_t* e) { Config.resetToDefaults(); }, LV_EVENT_CLICKED, NULL);
}

void LVGLUIManager::update() {
    uint32_t now = millis();
    if (now - _lastUiUpdate < 200) return; // 5Hz UI telemetry update
    _lastUiUpdate = now;

    if (!Display.lockUI()) return;

    SpaTelemetry t = Spa.getTelemetry();
    bool isC = (Config.getTempUnit() == TemperatureUnit::CELSIUS);

    // 1. Clock
    if (_labelTime) {
        lv_label_set_text(_labelTime, SpaScheduler.getFormattedTime().c_str());
    }

    // 2. Status banner
    if (_labelStatusMessage) {
        lv_label_set_text(_labelStatusMessage, t.errorString.c_str());
        if (t.errorString != "OK" && !t.errorString.startsWith("INFO")) {
            lv_obj_set_style_text_color(_labelStatusMessage, lv_color_hex(0xFF5252), 0);
        } else {
            lv_obj_set_style_text_color(_labelStatusMessage, lv_color_hex(0x81C784), 0);
        }
    }

    // 3. Flow Badge
    if (_labelFlowBadge) {
        if (t.flowOk) {
            lv_label_set_text(_labelFlowBadge, "FLOW OK");
            lv_obj_set_style_text_color(_labelFlowBadge, lv_color_hex(0x4CAF50), 0);
        } else {
            lv_label_set_text(_labelFlowBadge, "NO FLOW");
            lv_obj_set_style_text_color(_labelFlowBadge, lv_color_hex(0xF44336), 0);
        }
    }

    // 4. Main Water Temp & Arc
    if (_labelWaterTemp) {
        char tempStr[16];
        if (isC) {
            snprintf(tempStr, sizeof(tempStr), "%.1f°C", t.waterTempC);
        } else {
            snprintf(tempStr, sizeof(tempStr), "%.0f°F", t.waterTempF);
        }
        lv_label_set_text(_labelWaterTemp, tempStr);
    }

    if (_arcTemp) {
        int arcVal = (int)t.waterTempF;
        if (arcVal < 80) arcVal = 80;
        if (arcVal > 106) arcVal = 106;
        lv_arc_set_value(_arcTemp, arcVal);
    }

    // 5. Target Setpoint Label
    if (_labelTargetTemp) {
        char setStr[24];
        if (isC) {
            snprintf(setStr, sizeof(setStr), "Set: %.1f°C", t.targetTempC);
        } else {
            snprintf(setStr, sizeof(setStr), "Set: %.0f°F", t.targetTempF);
        }
        lv_label_set_text(_labelTargetTemp, setStr);
    }

    // 6. Heater Status Badge
    if (_labelHeaterStatus) {
        if (t.heaterState == HeaterState::HEATER_ON) {
            lv_label_set_text(_labelHeaterStatus, "HEATING");
            lv_obj_set_style_text_color(_labelHeaterStatus, lv_color_hex(0xFF7043), 0);
        } else if (t.heaterState == HeaterState::HEATER_PRE_FLOW) {
            lv_label_set_text(_labelHeaterStatus, "CHECKING FLOW");
            lv_obj_set_style_text_color(_labelHeaterStatus, lv_color_hex(0xFFCA28), 0);
        } else if (t.heaterState == HeaterState::HEATER_COOLDOWN) {
            lv_label_set_text(_labelHeaterStatus, "COOLING DOWN");
            lv_obj_set_style_text_color(_labelHeaterStatus, lv_color_hex(0x42A5F5), 0);
        } else {
            lv_label_set_text(_labelHeaterStatus, "HEATER OFF");
            lv_obj_set_style_text_color(_labelHeaterStatus, lv_color_hex(0x90CAF9), 0);
        }
    }

    // 7. Quick Button Colors & Labels
    if (_labelBtnJet1 && _btnJet1) {
        if (t.pump1Speed == PumpSpeed::SPEED_HIGH) {
            lv_label_set_text(_labelBtnJet1, "JETS 1\nHIGH");
            lv_obj_set_style_bg_color(_btnJet1, lv_color_hex(0x0288D1), 0);
        } else if (t.pump1Speed == PumpSpeed::SPEED_LOW) {
            lv_label_set_text(_labelBtnJet1, "JETS 1\nLOW");
            lv_obj_set_style_bg_color(_btnJet1, lv_color_hex(0x00897B), 0);
        } else {
            lv_label_set_text(_labelBtnJet1, "JETS 1\nOFF");
            lv_obj_set_style_bg_color(_btnJet1, lv_color_hex(0x2A3B5C), 0);
        }
    }

    if (_labelBtnJet2 && _btnJet2) {
        if (t.pump2Speed == PumpSpeed::SPEED_HIGH) {
            lv_label_set_text(_labelBtnJet2, "JETS 2\nON");
            lv_obj_set_style_bg_color(_btnJet2, lv_color_hex(0x0288D1), 0);
        } else {
            lv_label_set_text(_labelBtnJet2, "JETS 2\nOFF");
            lv_obj_set_style_bg_color(_btnJet2, lv_color_hex(0x2A3B5C), 0);
        }
    }

    if (_labelBtnBlower && _btnBlower) {
        if (t.blowerSpeed != BlowerSpeed::SPEED_OFF) {
            char blwBuf[24];
            snprintf(blwBuf, sizeof(blwBuf), "BLOWER\n%s", (t.blowerSpeed == BlowerSpeed::SPEED_HIGH) ? "HIGH" : (t.blowerSpeed == BlowerSpeed::SPEED_MED ? "MED" : "LOW"));
            lv_label_set_text(_labelBtnBlower, blwBuf);
            lv_obj_set_style_bg_color(_btnBlower, lv_color_hex(0x7E57C2), 0);
        } else {
            lv_label_set_text(_labelBtnBlower, "BLOWER\nOFF");
            lv_obj_set_style_bg_color(_btnBlower, lv_color_hex(0x2A3B5C), 0);
        }
    }

    if (_labelBtnLight && _btnLight) {
        if (t.lightActive) {
            lv_label_set_text(_labelBtnLight, "LIGHT\nON");
            lv_obj_set_style_bg_color(_btnLight, lv_color_hex(0xFBC02D), 0);
        } else {
            lv_label_set_text(_labelBtnLight, "LIGHT\nOFF");
            lv_obj_set_style_bg_color(_btnLight, lv_color_hex(0x2A3B5C), 0);
        }
    }

    // 8. Timers countdown text
    if (_labelJetTimer) {
        if (t.jet1RemainingSec > 0) {
            char jBuf[32];
            snprintf(jBuf, sizeof(jBuf), "Jet Timer: %02d:%02d", t.jet1RemainingSec / 60, t.jet1RemainingSec % 60);
            lv_label_set_text(_labelJetTimer, jBuf);
        } else {
            lv_label_set_text(_labelJetTimer, "Jet Timer: --");
        }
    }

    if (_labelBlowerTimer) {
        if (t.blowerRemainingSec > 0) {
            char bBuf[32];
            snprintf(bBuf, sizeof(bBuf), "Blower Timer: %02d:%02d", t.blowerRemainingSec / 60, t.blowerRemainingSec % 60);
            lv_label_set_text(_labelBlowerTimer, bBuf);
        } else {
            lv_label_set_text(_labelBlowerTimer, "Blower Timer: --");
        }
    }

    // 9. Diagnostics updates
    if (_labelDiagRelays) {
        uint8_t r = MCP23017.getRelayByte();
        char rBuf[128];
        snprintf(rBuf, sizeof(rBuf), "Relays: [P1_L:%d] [P1_H:%d] [P2:%d] [BL_L:%d] [BL_H:%d] [HEAT:%d] [OZONE:%d] [LGT:%d]",
            (r & 1) ? 1 : 0, (r & 2) ? 1 : 0, (r & 4) ? 1 : 0, (r & 8) ? 1 : 0,
            (r & 16) ? 1 : 0, (r & 32) ? 1 : 0, (r & 64) ? 1 : 0, (r & 128) ? 1 : 0);
        lv_label_set_text(_labelDiagRelays, rBuf);
    }

    if (_labelDiagSystem) {
        char sBuf[128];
        snprintf(sBuf, sizeof(sBuf), "System: Free Heap: %d KB | PSRAM: %d KB | IP: %s",
            ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024, WiFi.localIP().toString().c_str());
        lv_label_set_text(_labelDiagSystem, sBuf);
    }

    Display.unlockUI();
}
