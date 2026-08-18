#pragma once
#include <Arduino.h>

/*******************************************************************************
 * WAVESHARE ESP32-S3 TOUCH LCD 7 / 7B HARDWARE DEFINITIONS
 *******************************************************************************/

// LCD Panel Resolution
#define LCD_H_RES               800
#define LCD_V_RES               480

// ST7262 RGB Panel Timings
#define LCD_PIXEL_CLOCK_HZ      (16 * 1000 * 1000)
#define LCD_HSYNC_POLARITY      0
#define LCD_HSYNC_FRONT_PORCH   40
#define LCD_HSYNC_PULSE_WIDTH   48
#define LCD_HSYNC_BACK_PORCH    40
#define LCD_VSYNC_POLARITY      0
#define LCD_VSYNC_FRONT_PORCH   13
#define LCD_VSYNC_PULSE_WIDTH   3
#define LCD_VSYNC_BACK_PORCH    32
#define LCD_PCLK_ACTIVE_NEG     1

// LCD RGB Pin Mapping (ESP32-S3 Direct GPIOs)
#define LCD_PIN_PCLK            7
#define LCD_PIN_VSYNC           3
#define LCD_PIN_HSYNC           46
#define LCD_PIN_DE              5

// RGB565 Data Pins
#define LCD_PIN_R3              1
#define LCD_PIN_R4              2
#define LCD_PIN_R5              42
#define LCD_PIN_R6              41
#define LCD_PIN_R7              40

#define LCD_PIN_G2              39
#define LCD_PIN_G3              0
#define LCD_PIN_G4              45
#define LCD_PIN_G5              48
#define LCD_PIN_G6              47
#define LCD_PIN_G7              21

#define LCD_PIN_B3              14
#define LCD_PIN_B4              38
#define LCD_PIN_B5              18
#define LCD_PIN_B6              17
#define LCD_PIN_B7              10

// Shared I2C Bus Pins (Controls GT911, CH422G, and MCP23017)
#define I2C_SDA_PIN             8
#define I2C_SCL_PIN             9
#define I2C_FREQ_HZ             100000  // 100kHz for reliable multi-device arbitration

// GT911 Capacitive Touch Controller
#define TOUCH_GT911_I2C_ADDR_1  0x5D
#define TOUCH_GT911_I2C_ADDR_2  0x14
#define TOUCH_GT911_INT_PIN     4

// Onboard CH422G I/O Expander
#define CH422G_I2C_ADDR         0x24
#define CH422G_PIN_TP_RST       1
#define CH422G_PIN_LCD_BL       2
#define CH422G_PIN_LCD_RST      3
#define CH422G_PIN_SD_CS        4
#define CH422G_PIN_USB_SEL      5

/*******************************************************************************
 * MCP23017 16-BIT I2C I/O EXPANDER CONFIGURATION
 * Default Address: 0x20 (A0=GND, A1=GND, A2=GND)
 *******************************************************************************/
#define MCP23017_I2C_ADDR       0x20

// MCP23017 Port A: High-Power Outputs / Relays (Active LOW typical for relay modules)
#define RELAY_ACTIVE_LEVEL      LOW     // Change to HIGH if using active-high relay board

#define MCP_PIN_PUMP1_LOW       0       // GPA0 - Jet Pump 1 (Low Speed / Circ)
#define MCP_PIN_PUMP1_HIGH      1       // GPA1 - Jet Pump 1 (High Speed)
#define MCP_PIN_PUMP2_HIGH      2       // GPA2 - Jet Pump 2 / Aux Pump
#define MCP_PIN_BLOWER_LOW      3       // GPA3 - Air Blower Low
#define MCP_PIN_BLOWER_HIGH     4       // GPA4 - Air Blower High
#define MCP_PIN_HEATER          5       // GPA5 - Heater Contactor
#define MCP_PIN_OZONE           6       // GPA6 - Ozone / UV Sanitizer
#define MCP_PIN_LIGHT_RELAY     7       // GPA7 - Spa Main Light Relay

// MCP23017 Port B: Safety Inputs & Sensors (Active LOW with internal pull-ups)
#define MCP_PIN_FLOW_SWITCH     8       // GPB0 - Water Flow / Pressure Switch
#define MCP_PIN_HILIMIT_SWITCH  9       // GPB1 - High-Limit Thermal Switch (NC)
#define MCP_PIN_WATER_LEVEL     10      // GPB2 - Water Level Float Switch
#define MCP_PIN_COVER_SWITCH    11      // GPB3 - Spa Cover Magnetic Sensor
#define MCP_PIN_BTN_JETS        12      // GPB4 - Optional Physical Jet Pushbutton
#define MCP_PIN_BTN_BLOWER      13      // GPB5 - Optional Physical Blower Pushbutton
#define MCP_PIN_BTN_LIGHT       14      // GPB6 - Optional Physical Light Pushbutton
#define MCP_PIN_AUX_INPUT       15      // GPB7 - Auxiliary / Leak Sensor

/*******************************************************************************
 * TEMPERATURE SENSOR (DS18B20 1-WIRE) & OPTIONAL PWM PINS
 *******************************************************************************/
#define ONE_WIRE_BUS_PIN        15      // External 1-Wire GPIO (Requires 4.7k pullup)

// Optional Variable Speed / Dimming PWM Channels (ESP32-S3 LEDC)
#define PIN_BLOWER_PWM          16      // Optional 0-10V or PWM Blower Control
#define PIN_LIGHT_PWM           6       // Optional 12V Dimming / RGB PWM

#define PWM_FREQ_HZ             5000
#define PWM_RESOLUTION_BITS     8       // 0 - 255

/*******************************************************************************
 * SAFETY THRESHOLDS, LIMITS & TIMER CONSTANTS
 *******************************************************************************/
#define TEMP_MIN_SETPOINT_F     80.0f   // 26.7°C
#define TEMP_MAX_SETPOINT_F     104.0f  // 40.0°C (Standard residential hot tub limit)
#define TEMP_DEFAULT_SETPOINT_F 100.0f  // 37.8°C
#define TEMP_OVERHEAT_LIMIT_F   105.0f  // 40.5°C (Safety warning)
#define TEMP_EMERGENCY_TRIP_F   108.0f  // 42.2°C (Hard emergency shutdown)
#define TEMP_FREEZE_PROTECT_F   44.0f   // 6.7°C (Auto-start circ + heat)
#define TEMP_HYSTERESIS_F       1.0f    // Heating hysteresis band (+/- 0.5°F)

// Timing & Safety Delays (Seconds)
#define JET_HIGH_TIMEOUT_SEC    (15 * 60)   // 15-minute auto-off for High Jets
#define BLOWER_TIMEOUT_SEC      (15 * 60)   // 15-minute auto-off for Air Blower
#define LIGHT_TIMEOUT_SEC       (60 * 60)   // 60-minute auto-off for Spa Lights
#define HEATER_FLOW_VERIFY_SEC  15          // Require 15s verified flow before heater on
#define HEATER_COOLDOWN_SEC     30          // Pump runs 30s after heater turns off
#define HEATER_MIN_OFF_TIME_SEC 120         // Anti-short-cycle delay (2 minutes)
#define PURGE_CYCLE_DURATION_S  30          // Daily line purge cycle duration

/*******************************************************************************
 * NETWORKING & BLUETOOTH DEFAULTS
 *******************************************************************************/
#define DEFAULT_HOSTNAME        "hottub-controller"
#define DEFAULT_AP_SSID         "HotTub-Spa-AP"
#define DEFAULT_AP_PASS         "spa12345"
#define WS_PORT                 80
#define NTP_SERVER_1            "pool.ntp.org"
#define NTP_SERVER_2            "time.nist.gov"
#define DEFAULT_TIMEZONE_OFFSET (-5 * 3600) // EST default (-5)

// Bluetooth GATT UUIDs
#define BLE_SPA_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHAR_TELEMETRY_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Read/Notify
#define BLE_CHAR_COMMAND_UUID       "beb5483f-36e1-4688-b7f5-ea07361b26a9" // Write
#define BLE_CHAR_SCHEDULE_UUID      "beb54840-36e1-4688-b7f5-ea07361b26aa" // Read/Write
