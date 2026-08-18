# Arduino IDE Setup & Flashing Guide

This guide provides step-by-step instructions for compiling and uploading the **Hot Tub Controller** firmware to the **Waveshare ESP32-S3 Touch LCD 7" / 7B** board using the **Arduino IDE** (version 2.x recommended).

---

## 1. Install ESP32 Board Package

1. Open Arduino IDE and go to **File > Preferences** (or `Ctrl+,` / `Cmd+,`).
2. In the **Additional boards manager URLs** field, add the official Espressif URL:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Open **Tools > Board > Boards Manager...** (or click the board icon on the left sidebar).
4. Search for `esp32` by **Espressif Systems** and install version **`2.0.14`** or **`3.0.x`**.

---

## 2. Install Required Libraries

Open the **Library Manager** (**Sketch > Include Library > Manage Libraries...** or `Ctrl+Shift+I`) and install the following:

| Library Name | Author | Recommended Version |
| :--- | :--- | :--- |
| **`lvgl`** | kisvegabor | **`8.3.11`** *(Note: Select 8.3.x from the dropdown, not 9.x)* |
| **`ArduinoJson`** | Benoit Blanchon | **`7.0.x`** or latest 7.x |
| **`ESPAsyncWebServer`** | Mathieu Carbou | **`3.1.5`** or latest |
| **`AsyncTCP`** | Mathieu Carbou | **`3.1.4`** or latest |
| **`OneWire`** | Paul Stoffregen | **`2.3.8`** |
| **`DallasTemperature`**| Miles Burton | **`3.11.0`** |

---

## 3. Configure LVGL (`lv_conf.h`)

LVGL requires its configuration file in your Arduino libraries directory:

1. Locate your Arduino sketchbook directory (default: `Documents/Arduino/libraries/`).
2. Copy the file `include/lv_conf.h` from this project and paste it directly into:
   ```text
   Documents/Arduino/libraries/lv_conf.h
   ```
   *(It must sit directly inside `libraries/`, in the same folder as the `lvgl/` directory, NOT inside `lvgl/src/`).*

---

## 4. Arduino IDE Tools Menu Settings

Select the following configuration under the **Tools** menu for the **Waveshare ESP32-S3 Touch LCD 7" / 7B**:

| Setting | Value to Select | Why It's Critical |
| :--- | :--- | :--- |
| **Board** | `ESP32S3 Dev Module` | Standard ESP32-S3 target |
| **Port** | Select your COM / Serial port | Port appearing when plugged in |
| **CPU Frequency** | `240MHz (WiFi)` | High clock for smooth 60fps LVGL rendering |
| **Flash Size** | `16MB (128Mb)` | Waveshare board has 16MB SPI Flash |
| **Flash Mode** | `QIO 80MHz` | Quad I/O High Speed |
| **Partition Scheme** | `16M Flash (3MB APP/9.9MB FATFS)` or `16M Flash (2MB APP/12.5MB SPIFFS)` | Accommodates LittleFS web storage & OTA |
| **PSRAM** | **`OPI PSRAM`** | **CRITICAL: Required for 800x480 RGB framebuffers** |
| **USB CDC On Boot** | **`Enabled`** | Enables Serial Monitor output via native USB-C port |
| **USB Firmware MSC** | `Disabled` | Standard firmware execution |
| **USB DFU On Boot** | `Disabled` | Standard boot |
| **Upload Mode** | `UART0 / Hardware CDC` | Standard flashing mode |
| **Upload Speed** | `921600` | Fast flashing |
| **Core Debug Level** | `Info` (or `None`) | Serial debugging information |

> [!IMPORTANT]
> **PSRAM Setting**: You **MUST** set **PSRAM: OPI PSRAM** (Octal SPI PSRAM). If this is set to *Disabled* or *QSPI*, the ESP32-S3 will not have enough internal RAM to allocate the 800x480 RGB display framebuffers and will crash on boot.

---

## 5. Compile & Upload

1. Double-click **`HotTub.ino`** in the `HotTub` folder to open the project in Arduino IDE.
2. Click the **Verify / Compile** button (checkmark icon or `Ctrl+R`) to confirm clean build.
3. Connect your Waveshare board to your PC using a USB-C cable (connected to the port labelled `USB` or `UART`).
4. Select the corresponding COM port in **Tools > Port**.
5. Click **Upload** (arrow icon or `Ctrl+U`).

---

## 6. Uploading Web Files to LittleFS (Optional)

The controller has an **embedded fallback web interface stored directly in flash**, so the web dashboard works out of the box immediately after flashing `HotTub.ino`.

If you wish to upload the standalone HTML/CSS/JS files from the `data/` folder to LittleFS:
1. Install the **Arduino ESP32 LittleFS Filesystem Uploader** plugin in Arduino IDE.
2. Run **Tools > ESP32 LittleFS Data Upload**.
