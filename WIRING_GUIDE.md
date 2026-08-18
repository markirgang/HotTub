# Residential Hot Tub Controller - Hardware Wiring & Safety Guide

This document covers the electrical wiring, safety interlocks, relay connections, and sensor integration for the **Waveshare ESP32-S3 Touch LCD 7" / 7B** and **MCP23017 16-Bit I/O Expander**.

---

## ⚠️ High-Voltage Safety Warnings

> [!CAUTION]
> **DANGER: RISK OF ELECTRICAL SHOCK OR ELECTROCUTION**
> - Hot tubs operate on high-voltage AC (120V AC 15A/20A or 240V AC 30A/40A/50A).
> - **Always power all equipment through a dedicated Ground Fault Circuit Interrupter (GFCI / RCD) breaker.**
> - The ESP32-S3 and MCP23017 operate at 3.3V DC / 5V DC and **MUST BE OPTICALLY ISOLATED** from AC mains.
> - Never connect microcontrollers directly to high-voltage lines. Use certified optocoupler relay boards and UL-rated contactors for high-power heating elements.

---

## 1. ESP32-S3 (Waveshare 7" Board) to MCP23017 Pinout

The MCP23017 communicates over the shared I2C bus exposed on the Waveshare header:

| Waveshare ESP32-S3 Pin | MCP23017 Pin | Description | Note |
| :--- | :--- | :--- | :--- |
| **3.3V / 5V** | Pin 9 (`VDD`) & Pin 18 (`\RESET`) | Power Supply & Reset | Connect `\RESET` to VDD |
| **GND** | Pin 10 (`VSS`) | Ground | Common system ground |
| **GPIO 8** | Pin 13 (`SDA`) | I2C Serial Data | Shared with GT911/CH422G |
| **GPIO 9** | Pin 12 (`SCL`) | I2C Serial Clock | Shared with GT911/CH422G |
| **GND** | Pin 15 (`A0`), 16 (`A1`), 17 (`A2`) | I2C Hardware Address | Sets address to `0x20` |

---

## 2. MCP23017 Pin Assignment Table

### Port A (Relay Outputs - GPA0 to GPA7)

| MCP23017 Pin | Signal Name | Target Load | Recommended Contactor / Relay |
| :--- | :--- | :--- | :--- |
| **Pin 21 (GPA0)** | `PUMP1_LOW` | Jet Pump 1 - Low / Circulation | 10A / 16A Opto-Relay Channel 1 |
| **Pin 22 (GPA1)** | `PUMP1_HIGH` | Jet Pump 1 - High Speed Boost | 10A / 16A Opto-Relay Channel 2 |
| **Pin 23 (GPA2)** | `PUMP2_HIGH` | Jet Pump 2 / Auxiliary Jets | 10A / 16A Opto-Relay Channel 3 |
| **Pin 24 (GPA3)** | `BLOWER_LOW` | Air Blower - Low Speed | 10A Opto-Relay Channel 4 |
| **Pin 25 (GPA4)** | `BLOWER_HIGH` | Air Blower - High Speed | 10A Opto-Relay Channel 5 |
| **Pin 26 (GPA5)** | `HEATER_RELAY`| Heater Element | **30A/40A Heavy-Duty Contactor Coil** |
| **Pin 27 (GPA6)** | `OZONE_RELAY` | Ozone / UV Sanitizer Unit | 5A / 10A Opto-Relay Channel 7 |
| **Pin 28 (GPA7)** | `LIGHT_RELAY` | Spa LED Underwater Light | 5A / 10A Opto-Relay Channel 8 |

> [!IMPORTANT]
> **Heater Contactor Requirement**: Do NOT drive a 4kW - 5.5kW spa heater element directly with standard 10A blue PCB relays! Instead, connect `GPA5` to switch a 12V or 120V coil on a dedicated **30A/40A 2-Pole Definite Purpose Contactor**.

### Port B (Digital Safety Inputs - GPB0 to GPB7)
*All Port B pins utilize MCP23017 internal 100kΩ pull-up resistors to 3.3V / 5V.*

| MCP23017 Pin | Signal Name | Connected Sensor | Trigger Condition |
| :--- | :--- | :--- | :--- |
| **Pin 1 (GPB0)** | `FLOW_SWITCH` | Water Pressure / Flow Switch | **Closed to GND = Normal Flow (OK)** |
| **Pin 2 (GPB1)** | `HILIMIT_SWITCH` | High-Limit Thermal Switch (NC) | **Closed to GND = Normal (<108°F)** |
| **Pin 3 (GPB2)** | `WATER_LEVEL` | Float Switch Sensor | **Closed to GND = Water Level OK** |
| **Pin 4 (GPB3)** | `COVER_SWITCH` | Magnetic Reed Switch on Cover | Closed to GND = Cover Closed |
| **Pin 5 (GPB4)** | `BTN_JETS` | Optional Deck Pushbutton | Momentary press to GND (Cycle Jets) |
| **Pin 6 (GPB5)** | `BTN_BLOWER` | Optional Deck Pushbutton | Momentary press to GND (Cycle Blower) |
| **Pin 7 (GPB6)** | `BTN_LIGHT` | Optional Deck Pushbutton | Momentary press to GND (Toggle Light) |
| **Pin 8 (GPB7)** | `AUX_INPUT` | Bilge / Leak Detection Sensor | Contact to GND |

---

## 3. Temperature Sensors (DS18B20 1-Wire)

- **Data Pin**: Connect `DATA` (Yellow/White wire) to **GPIO 15** on the ESP32-S3.
- **Power**: Connect `VCC` (Red wire) to **3.3V** or **5V**.
- **Ground**: Connect `GND` (Black wire) to **GND**.
- **Pull-Up Resistor**: Place a **4.7kΩ resistor** between `DATA` (GPIO 15) and `3.3V`.

---

## 4. Variable Speed Controls (LEDC PWM)

| ESP32-S3 GPIO | Function | Output Format | Application |
| :--- | :--- | :--- | :--- |
| **GPIO 16** | Blower Speed | 5kHz PWM (0 - 100%) | Connect to 0-10V converter or VFD for variable brushless blower |
| **GPIO 6** | Spa Light Dimming | 5kHz PWM (0 - 100%) | Connect to MOSFET driver for 12V LED underwater lighting |

---

## 5. Inductive Snubber Circuits (RC Snubbers)

When switching inductive loads (pumps, blowers, contactor coils), high-voltage flyback spikes can cause electrical noise or I2C bus interference:
- Install an **RC Snubber (0.1µF 630V capacitor + 100Ω 2W resistor in series)** across the AC contacts of the pump and contactor relays.
- Keep the low-voltage ESP32 and I2C wiring physically separated (>6 inches) from high-voltage 120V/240V AC conduits.
