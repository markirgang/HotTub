#include "ui_hottub.h"
#include "lvgl.h"
#include "spa_controller.h"
#include "hal_hottub.h"
#include "spa_scheduler.h"
#include "esp_log.h"
#include "esp_system.h"
#include <stdio.h>

static const char *TAG = "UI_HOTTUB";

// UI Widgets
static lv_obj_t *s_tabview = NULL;

// Tab 1: Dashboard Widgets
static lv_obj_t *s_lbl_header_status = NULL;
static lv_obj_t *s_arc_temp = NULL;
static lv_obj_t *s_lbl_current_temp = NULL;
static lv_obj_t *s_lbl_setpoint_val = NULL;
static lv_obj_t *s_lbl_status_msg = NULL;

static lv_obj_t *s_btn_jet1 = NULL;
static lv_obj_t *s_lbl_jet1 = NULL;
static lv_obj_t *s_btn_jet2 = NULL;
static lv_obj_t *s_lbl_jet2 = NULL;
static lv_obj_t *s_btn_blower = NULL;
static lv_obj_t *s_lbl_blower = NULL;
static lv_obj_t *s_btn_light = NULL;
static lv_obj_t *s_lbl_light = NULL;

// Tab 3: Modes & Unit Widgets
static lv_obj_t *s_btn_mode_std = NULL;
static lv_obj_t *s_btn_mode_econ = NULL;
static lv_obj_t *s_btn_mode_sleep = NULL;
static lv_obj_t *s_sw_unit_celsius = NULL;

// Tab 4: Diagnostics Widgets
static lv_obj_t *s_lbl_diag_relays = NULL;
static lv_obj_t *s_lbl_diag_inputs = NULL;
static lv_obj_t *s_lbl_diag_sys = NULL;

// Callbacks
static void cb_btn_temp_plus(lv_event_t *e) {
    spa_status_t status;
    spa_controller_get_status(&status);
    float step = status.is_celsius ? 0.5f : 1.0f;
    float current_sp = status.is_celsius ? (status.setpoint_f - 32.0f) * 5.0f / 9.0f : status.setpoint_f;
    float new_sp = current_sp + step;
    float new_sp_f = status.is_celsius ? (new_sp * 9.0f / 5.0f) + 32.0f : new_sp;
    spa_controller_set_setpoint(new_sp_f);
}

static void cb_btn_temp_minus(lv_event_t *e) {
    spa_status_t status;
    spa_controller_get_status(&status);
    float step = status.is_celsius ? 0.5f : 1.0f;
    float current_sp = status.is_celsius ? (status.setpoint_f - 32.0f) * 5.0f / 9.0f : status.setpoint_f;
    float new_sp = current_sp - step;
    float new_sp_f = status.is_celsius ? (new_sp * 9.0f / 5.0f) + 32.0f : new_sp;
    spa_controller_set_setpoint(new_sp_f);
}

static void cb_btn_jet1(lv_event_t *e) { spa_controller_toggle_jet1(); }
static void cb_btn_jet2(lv_event_t *e) { spa_controller_toggle_jet2(); }
static void cb_btn_blower(lv_event_t *e) { spa_controller_toggle_blower(); }
static void cb_btn_light(lv_event_t *e) { spa_controller_toggle_light(); }

static void cb_btn_mode_std(lv_event_t *e) { spa_controller_set_heat_mode(HEAT_MODE_STANDARD); }
static void cb_btn_mode_econ(lv_event_t *e) { spa_controller_set_heat_mode(HEAT_MODE_ECONOMY); }
static void cb_btn_mode_sleep(lv_event_t *e) { spa_controller_set_heat_mode(HEAT_MODE_SLEEP); }

static void cb_sw_unit(lv_event_t *e) {
    bool is_c = lv_obj_has_state(s_sw_unit_celsius, LV_STATE_CHECKED);
    spa_controller_set_celsius(is_c);
}

void ui_hottub_init(void)
{
    ESP_LOGI(TAG, "Initializing 800x480 LVGL v8 Hot Tub UI");

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x121824), LV_PART_MAIN);

    // 1. Header Bar
    lv_obj_t *header = lv_obj_create(scr);
    lv_obj_set_size(header, 800, 50);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_color(header, lv_color_hex(0x334155), LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 5, LV_PART_MAIN);

    lv_obj_t *lbl_title = lv_label_create(header);
    lv_label_set_text(lbl_title, "♨ HOT TUB CONTROLLER");
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x38BDF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(lbl_title, LV_ALIGN_LEFT_MID, 10, 0);

    s_lbl_header_status = lv_label_create(header);
    lv_label_set_text(s_lbl_header_status, "INITIALIZING...");
    lv_obj_set_style_text_color(s_lbl_header_status, lv_color_hex(0x4ADE80), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_lbl_header_status, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(s_lbl_header_status, LV_ALIGN_RIGHT_MID, -10, 0);

    // 2. Main Tabview (800x430)
    s_tabview = lv_tabview_create(scr, LV_DIR_TOP, 40);
    lv_obj_set_size(s_tabview, 800, 430);
    lv_obj_align(s_tabview, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(s_tabview, lv_color_hex(0x121824), LV_PART_MAIN);

    lv_obj_t *tab_dash  = lv_tabview_add_tab(s_tabview, "Dashboard");
    lv_obj_t *tab_ctrl  = lv_tabview_add_tab(s_tabview, "Controls");
    lv_obj_t *tab_sched = lv_tabview_add_tab(s_tabview, "Schedules");
    lv_obj_t *tab_diag  = lv_tabview_add_tab(s_tabview, "Diagnostics");

    // ==========================================
    // TAB 1: DASHBOARD
    // ==========================================
    // Left Side: Temperature Arc Gauge
    s_arc_temp = lv_arc_create(tab_dash);
    lv_obj_set_size(s_arc_temp, 220, 220);
    lv_arc_set_range(s_arc_temp, 80, 106);
    lv_arc_set_value(s_arc_temp, 100);
    lv_obj_align(s_arc_temp, LV_ALIGN_TOP_LEFT, 30, 20);
    lv_obj_set_style_arc_color(s_arc_temp, lv_color_hex(0x334155), LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_arc_temp, lv_color_hex(0xF97316), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_arc_temp, 16, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_temp, 16, LV_PART_INDICATOR);

    s_lbl_current_temp = lv_label_create(s_arc_temp);
    lv_label_set_text(s_lbl_current_temp, "100.0°F");
    lv_obj_set_style_text_color(s_lbl_current_temp, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_lbl_current_temp, &lv_font_montserrat_44, LV_PART_MAIN);
    lv_obj_align(s_lbl_current_temp, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *lbl_water_lbl = lv_label_create(s_arc_temp);
    lv_label_set_text(lbl_water_lbl, "WATER TEMP");
    lv_obj_set_style_text_color(lbl_water_lbl, lv_color_hex(0x94A3B8), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_water_lbl, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(lbl_water_lbl, LV_ALIGN_CENTER, 0, 25);

    // Center: Target Setpoint Control Card
    lv_obj_t *card_sp = lv_obj_create(tab_dash);
    lv_obj_set_size(card_sp, 260, 220);
    lv_obj_align(card_sp, LV_ALIGN_TOP_LEFT, 270, 20);
    lv_obj_set_style_bg_color(card_sp, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_border_color(card_sp, lv_color_hex(0x334155), LV_PART_MAIN);

    lv_obj_t *lbl_sp_title = lv_label_create(card_sp);
    lv_label_set_text(lbl_sp_title, "SETPOINT");
    lv_obj_set_style_text_color(lbl_sp_title, lv_color_hex(0x94A3B8), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_sp_title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(lbl_sp_title, LV_ALIGN_TOP_MID, 0, 5);

    s_lbl_setpoint_val = lv_label_create(card_sp);
    lv_label_set_text(s_lbl_setpoint_val, "102.0°F");
    lv_obj_set_style_text_color(s_lbl_setpoint_val, lv_color_hex(0x38BDF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_lbl_setpoint_val, &lv_font_montserrat_44, LV_PART_MAIN);
    lv_obj_align(s_lbl_setpoint_val, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *btn_minus = lv_btn_create(card_sp);
    lv_obj_set_size(btn_minus, 70, 50);
    lv_obj_align(btn_minus, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_event_cb(btn_minus, cb_btn_temp_minus, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_m = lv_label_create(btn_minus);
    lv_label_set_text(lbl_m, "-");
    lv_obj_set_style_text_font(lbl_m, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_center(lbl_m);

    lv_obj_t *btn_plus = lv_btn_create(card_sp);
    lv_obj_set_size(btn_plus, 70, 50);
    lv_obj_align(btn_plus, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_add_event_cb(btn_plus, cb_btn_temp_plus, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_p = lv_label_create(btn_plus);
    lv_label_set_text(lbl_p, "+");
    lv_obj_set_style_text_font(lbl_p, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_center(lbl_p);

    // Right Side: Quick Equipment Action Toggles
    s_btn_jet1 = lv_btn_create(tab_dash);
    lv_obj_set_size(s_btn_jet1, 100, 95);
    lv_obj_align(s_btn_jet1, LV_ALIGN_TOP_LEFT, 550, 20);
    lv_obj_add_event_cb(s_btn_jet1, cb_btn_jet1, LV_EVENT_CLICKED, NULL);
    s_lbl_jet1 = lv_label_create(s_btn_jet1);
    lv_label_set_text(s_lbl_jet1, "JETS 1\nOFF");
    lv_obj_center(s_lbl_jet1);

    s_btn_jet2 = lv_btn_create(tab_dash);
    lv_obj_set_size(s_btn_jet2, 100, 95);
    lv_obj_align(s_btn_jet2, LV_ALIGN_TOP_LEFT, 665, 20);
    lv_obj_add_event_cb(s_btn_jet2, cb_btn_jet2, LV_EVENT_CLICKED, NULL);
    s_lbl_jet2 = lv_label_create(s_btn_jet2);
    lv_label_set_text(s_lbl_jet2, "JETS 2\nOFF");
    lv_obj_center(s_lbl_jet2);

    s_btn_blower = lv_btn_create(tab_dash);
    lv_obj_set_size(s_btn_blower, 100, 95);
    lv_obj_align(s_btn_blower, LV_ALIGN_TOP_LEFT, 550, 130);
    lv_obj_add_event_cb(s_btn_blower, cb_btn_blower, LV_EVENT_CLICKED, NULL);
    s_lbl_blower = lv_label_create(s_btn_blower);
    lv_label_set_text(s_lbl_blower, "BLOWER\nOFF");
    lv_obj_center(s_lbl_blower);

    s_btn_light = lv_btn_create(tab_dash);
    lv_obj_set_size(s_btn_light, 100, 95);
    lv_obj_align(s_btn_light, LV_ALIGN_TOP_LEFT, 665, 130);
    lv_obj_add_event_cb(s_btn_light, cb_btn_light, LV_EVENT_CLICKED, NULL);
    s_lbl_light = lv_label_create(s_btn_light);
    lv_label_set_text(s_lbl_light, "LIGHT\nOFF");
    lv_obj_center(s_lbl_light);

    // Bottom Status Banner
    s_lbl_status_msg = lv_label_create(tab_dash);
    lv_label_set_text(s_lbl_status_msg, "Status: System Ready");
    lv_obj_set_style_text_color(s_lbl_status_msg, lv_color_hex(0x94A3B8), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_lbl_status_msg, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(s_lbl_status_msg, LV_ALIGN_BOTTOM_LEFT, 30, -15);

    // ==========================================
    // TAB 2: CONTROLS
    // ==========================================
    lv_obj_t *lbl_ctrl_title = lv_label_create(tab_ctrl);
    lv_label_set_text(lbl_ctrl_title, "Manual Equipment Controls");
    lv_obj_set_style_text_font(lbl_ctrl_title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(lbl_ctrl_title, LV_ALIGN_TOP_LEFT, 20, 10);

    // ==========================================
    // TAB 3: SCHEDULES & MODES
    // ==========================================
    lv_obj_t *lbl_mode_hdr = lv_label_create(tab_sched);
    lv_label_set_text(lbl_mode_hdr, "Heating Mode Select:");
    lv_obj_set_style_text_font(lbl_mode_hdr, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(lbl_mode_hdr, LV_ALIGN_TOP_LEFT, 20, 20);

    s_btn_mode_std = lv_btn_create(tab_sched);
    lv_obj_set_size(s_btn_mode_std, 150, 45);
    lv_obj_align(s_btn_mode_std, LV_ALIGN_TOP_LEFT, 20, 50);
    lv_obj_add_event_cb(s_btn_mode_std, cb_btn_mode_std, LV_EVENT_CLICKED, NULL);
    lv_obj_t *l_std = lv_label_create(s_btn_mode_std);
    lv_label_set_text(l_std, "STANDARD");
    lv_obj_center(l_std);

    s_btn_mode_econ = lv_btn_create(tab_sched);
    lv_obj_set_size(s_btn_mode_econ, 150, 45);
    lv_obj_align(s_btn_mode_econ, LV_ALIGN_TOP_LEFT, 180, 50);
    lv_obj_add_event_cb(s_btn_mode_econ, cb_btn_mode_econ, LV_EVENT_CLICKED, NULL);
    lv_obj_t *l_econ = lv_label_create(s_btn_mode_econ);
    lv_label_set_text(l_econ, "ECONOMY");
    lv_obj_center(l_econ);

    s_btn_mode_sleep = lv_btn_create(tab_sched);
    lv_obj_set_size(s_btn_mode_sleep, 150, 45);
    lv_obj_align(s_btn_mode_sleep, LV_ALIGN_TOP_LEFT, 340, 50);
    lv_obj_add_event_cb(s_btn_mode_sleep, cb_btn_mode_sleep, LV_EVENT_CLICKED, NULL);
    lv_obj_t *l_slp = lv_label_create(s_btn_mode_sleep);
    lv_label_set_text(l_slp, "SLEEP");
    lv_obj_center(l_slp);

    lv_obj_t *lbl_unit_hdr = lv_label_create(tab_sched);
    lv_label_set_text(lbl_unit_hdr, "Temperature Units (°F / °C):");
    lv_obj_set_style_text_font(lbl_unit_hdr, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(lbl_unit_hdr, LV_ALIGN_TOP_LEFT, 20, 120);

    s_sw_unit_celsius = lv_switch_create(tab_sched);
    lv_obj_align(s_sw_unit_celsius, LV_ALIGN_TOP_LEFT, 250, 115);
    lv_obj_add_event_cb(s_sw_unit_celsius, cb_sw_unit, LV_EVENT_VALUE_CHANGED, NULL);

    // ==========================================
    // TAB 4: HARDWARE DIAGNOSTICS & IO MONITOR
    // ==========================================
    s_lbl_diag_relays = lv_label_create(tab_diag);
    lv_label_set_text(s_lbl_diag_relays, "Relays: HEATER:OFF | CIRC:OFF | JET1:OFF | JET2:OFF | BLOWER:OFF | LIGHT:OFF");
    lv_obj_set_style_text_font(s_lbl_diag_relays, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_lbl_diag_relays, LV_ALIGN_TOP_LEFT, 20, 20);

    s_lbl_diag_inputs = lv_label_create(tab_diag);
    lv_label_set_text(s_lbl_diag_inputs, "Inputs: FLOW SWITCH: OK | OVERTEMP LIMIT: NORMAL");
    lv_obj_set_style_text_font(s_lbl_diag_inputs, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_lbl_diag_inputs, LV_ALIGN_TOP_LEFT, 20, 60);

    s_lbl_diag_sys = lv_label_create(tab_diag);
    lv_label_set_text(s_lbl_diag_sys, "System: Free Heap: --- KB | Uptime: --- s");
    lv_obj_set_style_text_font(s_lbl_diag_sys, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(s_lbl_diag_sys, LV_ALIGN_TOP_LEFT, 20, 100);
}

void ui_hottub_update(void)
{
    spa_status_t status;
    spa_controller_get_status(&status);

    hal_io_states_t io_states;
    hal_get_io_states(&io_states);

    // 1. Header Status Update
    lv_label_set_text(s_lbl_header_status, status.status_msg);

    // 2. Temperature Display Update
    char temp_str[32];
    char sp_str[32];
    if (status.is_celsius) {
        snprintf(temp_str, sizeof(temp_str), "%.1f°C", status.current_temp_c);
        float sp_c = (status.setpoint_f - 32.0f) * 5.0f / 9.0f;
        snprintf(sp_str, sizeof(sp_str), "%.1f°C", sp_c);
    } else {
        snprintf(temp_str, sizeof(temp_str), "%.1f°F", status.current_temp_f);
        snprintf(sp_str, sizeof(sp_str), "%.1f°F", status.setpoint_f);
    }
    lv_label_set_text(s_lbl_current_temp, temp_str);
    lv_label_set_text(s_lbl_setpoint_val, sp_str);
    lv_arc_set_value(s_arc_temp, (int32_t)status.current_temp_f);

    // 3. Equipment Button States & Timer Labels
    char btn_txt[32];
    if (status.jet1_active) {
        snprintf(btn_txt, sizeof(btn_txt), "JETS 1\n%dm %ds", status.jet1_timer_sec / 60, status.jet1_timer_sec % 60);
        lv_obj_set_style_bg_color(s_btn_jet1, lv_color_hex(0x22C55E), LV_PART_MAIN);
    } else {
        snprintf(btn_txt, sizeof(btn_txt), "JETS 1\nOFF");
        lv_obj_set_style_bg_color(s_btn_jet1, lv_color_hex(0x334155), LV_PART_MAIN);
    }
    lv_label_set_text(s_lbl_jet1, btn_txt);

    if (status.jet2_active) {
        snprintf(btn_txt, sizeof(btn_txt), "JETS 2\n%dm %ds", status.jet2_timer_sec / 60, status.jet2_timer_sec % 60);
        lv_obj_set_style_bg_color(s_btn_jet2, lv_color_hex(0x22C55E), LV_PART_MAIN);
    } else {
        snprintf(btn_txt, sizeof(btn_txt), "JETS 2\nOFF");
        lv_obj_set_style_bg_color(s_btn_jet2, lv_color_hex(0x334155), LV_PART_MAIN);
    }
    lv_label_set_text(s_lbl_jet2, btn_txt);

    if (status.blower_active) {
        snprintf(btn_txt, sizeof(btn_txt), "BLOWER\n%dm %ds", status.blower_timer_sec / 60, status.blower_timer_sec % 60);
        lv_obj_set_style_bg_color(s_btn_blower, lv_color_hex(0x22C55E), LV_PART_MAIN);
    } else {
        snprintf(btn_txt, sizeof(btn_txt), "BLOWER\nOFF");
        lv_obj_set_style_bg_color(s_btn_blower, lv_color_hex(0x334155), LV_PART_MAIN);
    }
    lv_label_set_text(s_lbl_blower, btn_txt);

    if (status.light_active) {
        lv_label_set_text(s_lbl_light, "LIGHT\nON");
        lv_obj_set_style_bg_color(s_btn_light, lv_color_hex(0xEAB308), LV_PART_MAIN);
    } else {
        lv_label_set_text(s_lbl_light, "LIGHT\nOFF");
        lv_obj_set_style_bg_color(s_btn_light, lv_color_hex(0x334155), LV_PART_MAIN);
    }

    // 4. Diagnostics Tab Updates
    char diag_relays[128];
    snprintf(diag_relays, sizeof(diag_relays), "Relays: HEATER:%s | CIRC:%s | JET1:%s | JET2:%s | BLOWER:%s | LIGHT:%s",
             io_states.heater_relay ? "ON" : "OFF",
             io_states.circ_pump_relay ? "ON" : "OFF",
             io_states.jet1_relay ? "ON" : "OFF",
             io_states.jet2_relay ? "ON" : "OFF",
             io_states.blower_relay ? "ON" : "OFF",
             io_states.light_relay ? "ON" : "OFF");
    lv_label_set_text(s_lbl_diag_relays, diag_relays);

    char diag_inputs[128];
    snprintf(diag_inputs, sizeof(diag_inputs), "Inputs: FLOW SWITCH: %s | OVERTEMP LIMIT: %s",
             io_states.flow_switch_active ? "VERIFIED OK" : "OPEN (NO FLOW)",
             io_states.high_limit_active ? "TRIPPED" : "NORMAL");
    lv_label_set_text(s_lbl_diag_inputs, diag_inputs);

    char diag_sys[128];
    snprintf(diag_sys, sizeof(diag_sys), "System: Free Heap: %d KB | Uptime: %d s",
             (int)(esp_get_free_heap_size() / 1024),
             (int)spa_scheduler_get_uptime_sec());
    lv_label_set_text(s_lbl_diag_sys, diag_sys);
}
