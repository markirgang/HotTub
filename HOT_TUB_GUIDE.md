# Smart Hot Tub Controller User & Safety Guide

Welcome to the **Smart Hot Tub Controller** for the Waveshare ESP32-S3 Touch LCD 7" / 7B development board.

---

## 🌟 Features Overview

1. **800x480 High-Resolution Touchscreen UI (LVGL v8)**:
   - **Dashboard**: Temperature gauge, target setpoint adjustment (`+` / `-`), status header (FLOW OK, HEATER ON, FILTER), and quick toggles (Jets 1, Jets 2, Blower, Light).
   - **Active Countdown Badges**: 15-minute auto-shutoff timers for High Jets and Blower with real-time countdown display (`14m 32s`).
   - **Hardware Diagnostics & I/O Monitor**: Live view of all 7 relay outputs, digital input states, DS18B20 temp reading, free heap/PSRAM, and system uptime.
   - **Modes & Unit Settings**: Toggle between °F and °C, and select heating modes (**Standard**, **Economy**, **Sleep**).

2. **Safety-Critical Interlocks**:
   - **Flow Switch Verification**: Requires continuous verified flow for 10 seconds before the heater relay energizes.
   - **Instant Flow Trip**: Immediate heater shutoff if flow switch opens.
   - **Heater Cooldown Flush**: Circulation pump maintains water flow for 30 seconds after heating turns off to flush residual element heat.
   - **Over-Temperature Lockout**: Soft cutoff at 104.0°F (40.0°C), emergency trip at 106.0°F (41.1°C).
   - **Freeze Protection**: Auto-starts circulation pump and heater if water drops below 44.0°F (6.7°C).

3. **Persistent Configuration (NVS)**:
   - Target setpoint, heating mode, temperature units (°F/°C), and calibration settings are retained across power cycles.

---

## ⚡ Hardware GPIO Mapping

| Relay / Input | ESP32-S3 GPIO | Function |
| :--- | :--- | :--- |
| **Relay 1** | `GPIO 8` | Heater Relay |
| **Relay 2** | `GPIO 9` | Circulation / Low-Speed Pump Relay |
| **Relay 3** | `GPIO 10` | High-Speed Jet Pump 1 Relay |
| **Relay 4** | `GPIO 11` | High-Speed Jet Pump 2 Relay |
| **Relay 5** | `GPIO 12` | Air Blower Relay |
| **Relay 6** | `GPIO 13` | Spa Light Relay |
| **Relay 7** | `GPIO 14` | Ozone Sanitizer Relay |
| **Flow Input**| `GPIO 15` | Flow Switch (Active LOW when flow present) |
| **High Limit**| `GPIO 16` | Emergency Over-temp Switch |
| **1-Wire Temp**| `GPIO 4` | DS18B20 Temperature Probe |

---

## 🚀 How to Build & Flash

### Prerequisites
1. PlatformIO CLI or VS Code extension with `espidf` framework.

### Building
```powershell
& "C:\Users\marki\.platformio\penv\Scripts\pio.exe" run -e 16_LVGL_UI
```

### Flashing to ESP32-S3
```powershell
& "C:\Users\marki\.platformio\penv\Scripts\pio.exe" run -e 16_LVGL_UI -t upload
```
