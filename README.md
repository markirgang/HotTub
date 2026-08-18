# Smart Hot Tub Controller for Waveshare ESP32-S3 Touch LCD 7" / 7B

A complete, safety-critical, smart residential hot tub control system powered by the **Waveshare ESP32-S3 Touch LCD 7" (and 7B)** development board, **MCP23017 16-bit I/O Expander**, **LVGL 8.3 Touchscreen GUI**, **Asynchronous Web Dashboard**, and **Bluetooth Low Energy (BLE)** GATT Server.

---

## 🌟 Key Features

1. **Local 7-inch High-Resolution Touchscreen UI (LVGL v8)**:
   - 800x480 resolution dark theme with high-contrast, finger-friendly touch controls.
   - Live circular temperature gauge with setpoint controls.
   - Individual pump and blower controls with active auto-off countdown timers.
   - Daily automatic circulation & filtration schedule configuration.
   - Live hardware I/O diagnostics tab (real-time relay states, input sensor monitor, system heap/PSRAM stats).

2. **Safety-Critical Interlocks & Temperature Regulation**:
   - **Flow Switch Verification**: Requires continuous verified flow for 15s before heater relay energizes.
   - **Instant Flow Trip**: Immediate element shutoff if flow switch opens.
   - **Heater Cooldown Flush**: Pump maintains circulation for 30s after heater turns off.
   - **Over-Temperature Lockout**: Hard cutoff at 105°F, emergency hardware trip at 108°F.
   - **Freeze Protection**: Auto-starts circulation pump and heater if water temp drops below 44°F (6.7°C).
   - **Auto-Shutoff Timers**: 15-minute timer for High Jets and Blower to prevent motor overheating.

3. **Responsive Web Dashboard (PWA)**:
   - Real-time bi-directional WebSockets (`/ws`) stream for instant response.
   - Works on mobile phones, tablets, and desktop browsers.
   - Captive portal fallback (`HotTub-Spa-AP`) for simple WiFi configuration.

4. **Bluetooth Low Energy (BLE) & Web Bluetooth**:
   - ESP32 BLE GATT Server with Spa Control Service.
   - Includes standalone **Web Bluetooth Client** (`/ble_app.html`) to control the hot tub from Chrome or Bluefy without installing an App Store app!

---

## 🛠 Project Structure

```
HotTub/
├── platformio.ini              # PlatformIO configuration for ESP32-S3 (16MB Flash, 8MB PSRAM)
├── partitions.csv              # 16MB partition table (Dual OTA + 8.8MB LittleFS)
├── WIRING_GUIDE.md             # Complete electrical schematic & high-voltage safety guide
├── include/
│   ├── config.h                # Hardware pinouts, limits, and timing constants
│   └── lv_conf.h               # LVGL 8.3 configuration
├── src/
│   ├── main.cpp                # Firmware initialization and main execution loops
│   ├── hal/                    # Hardware Abstraction Layer
│   │   ├── i2c_bus.h / .cpp    # Mutex-protected shared I2C bus manager
│   │   ├── ch422g.h / .cpp     # Onboard IO expander (LCD backlight, reset)
│   │   ├── mcp23017.h / .cpp   # 16-bit IO expander (Relays & safety inputs)
│   │   ├── display_driver.h/.cpp # ESP32-S3 RGB LCD + GT911 touch + LVGL task
│   │   ├── temp_sensor.h / .cpp# DS18B20 1-Wire temperature acquisition
│   │   └── pwm_controller.h/.cpp# LEDC PWM (Blower & Light dimming)
│   ├── core/                   # Core Business Logic
│   │   ├── spa_controller.h/.cpp# State machine & safety interlocks
│   │   ├── scheduler.h / .cpp  # Daily filtration & economy heating scheduler
│   │   └── config_manager.h/.cpp# Persistent NVS settings manager
│   ├── ui/                     # Local Touchscreen UI
│   │   └── lvgl_ui.h / .cpp    # LVGL screens, widgets, and callbacks
│   └── net/                    # Network & Wireless
│       ├── wifi_manager.h/.cpp # WiFi Station & SoftAP captive portal
│       ├── web_server.h / .cpp # AsyncWebServer + WebSockets + REST API
│       └── ble_controller.h/.cpp# BLE GATT Server & command handler
└── data/                       # LittleFS Web Assets
    ├── index.html              # Mobile-first Web Application
    ├── style.css               # Dark mode glassmorphism stylesheet
    ├── app.js                  # WebSocket client logic
    └── ble_app.html            # Web Bluetooth direct client
```

---

## 🚀 How to Build & Flash

### Prerequisites
1. Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO IDE Extension](https://platformio.org/).
2. Connect your Waveshare ESP32-S3 Touch LCD 7" board via USB (USB-C port labelled `USB` or `UART`).

### Compilation & Upload
1. Open the `HotTub` folder in VS Code / PlatformIO.
2. Build the firmware:
   ```bash
   pio run
   ```
3. Upload firmware to ESP32-S3:
   ```bash
   pio run --target upload
   ```
4. Upload Web files to LittleFS partition:
   ```bash
   pio run --target uploadfs
   ```

---

## 📱 Connecting to the Hot Tub

### 1. Local Touchscreen
- Immediately boots to the 800x480 dashboard on power-up.
- Touch `+` / `-` to change temperature, tap `Jets 1`, `Jets 2`, `Blower`, `Light`, or switch tabs.

### 2. Wi-Fi Web Dashboard
1. On initial boot, connect to the WiFi network:
   - **SSID**: `HotTub-Spa-AP`
   - **Password**: `spa12345`
2. Open your browser and navigate to `http://192.168.4.1`.
3. In the Settings card, enter your home WiFi credentials to join your home network.

### 3. Bluetooth Direct Control
1. Open Google Chrome, Microsoft Edge, or Bluefy (on iOS).
2. Visit `http://<controller-ip>/ble_app.html` or open the local file.
3. Click **Connect BLE** and pair with **`HotTub-Controller`**.
