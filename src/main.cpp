#include <Arduino.h>
#include "config.h"

// Hardware Abstraction Layer
#include "hal/i2c_bus.h"
#include "hal/ch422g.h"
#include "hal/mcp23017.h"
#include "hal/temp_sensor.h"
#include "hal/pwm_controller.h"
#include "hal/display_driver.h"

// Core Business Logic & State Machine
#include "core/config_manager.h"
#include "core/scheduler.h"
#include "core/spa_controller.h"

// User Interface & Connectivity
#include "ui/lvgl_ui.h"
#include "net/wifi_manager.h"
#include "net/web_server.h"
#include "net/ble_controller.h"

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n=======================================================");
    Serial.println("   WAVESHARE ESP32-S3 TOUCH 7-INCH HOT TUB CONTROLLER  ");
    Serial.println("=======================================================\n");

    // 1. Initialize Shared I2C Bus (GPIO 8 / GPIO 9)
    if (!I2CBus.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ)) {
        Serial.println("[ERROR] Failed to start I2C bus!");
    }

    // 2. Initialize Onboard CH422G IO Expander (Resets & Backlight)
    CH422G.begin();

    // 3. Initialize External MCP23017 16-bit IO Expander (Relays & Safety Inputs)
    if (!MCP23017.begin(MCP23017_I2C_ADDR)) {
        Serial.println("[WARN] MCP23017 not found at 0x20. Check wiring & I2C pullups!");
    }

    // 4. Initialize DS18B20 1-Wire Temperature Sensors
    TempSensor.begin(ONE_WIRE_BUS_PIN);

    // 5. Initialize PWM Controller (Variable Speed Blower & Dimming)
    PWMControl.begin();

    // 6. Initialize Persistent NVS Configuration
    Config.begin();

    // 7. Initialize Display & LVGL Framework (Runs on Core 1)
    if (Display.begin()) {
        UIManager.begin();
    } else {
        Serial.println("[ERROR] Display initialization failed!");
    }

    // 8. Initialize Core State Machine & Safety Interlock Engine
    Spa.begin();

    // 9. Initialize Scheduler & RTC
    SpaScheduler.begin();

    // 10. Initialize WiFi & Captive Portal
    SpaWiFi.begin();

    // 11. Initialize Web Dashboard & WebSockets Server
    SpaWeb.begin();

    // 12. Initialize Bluetooth Low Energy (BLE) GATT Server
    SpaBLE.begin();

    Serial.println("\n[SYSTEM] Spa Controller initialization completed successfully!\n");
}

void loop() {
    // 1. Core State Machine & Safety Interlocks (100ms tick)
    Spa.update();

    // 2. Local Touchscreen UI Telemetry Update (5Hz)
    UIManager.update();

    // 3. WiFi Manager & DNS Captive Portal handling
    SpaWiFi.update();

    // 4. WebSockets Telemetry Broadcast (2Hz)
    SpaWeb.update();

    // 5. Bluetooth LE Notifications (1Hz)
    SpaBLE.update();

    // Yield to FreeRTOS scheduler
    vTaskDelay(pdMS_TO_TICKS(10));
}
