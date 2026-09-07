# Waveshare ESP32-S3-Touch-LCD-7B Product Engineering Sample Program

[中文](README_ZH.md)

It supports 2.4GHz WiFi and BLE 5, integrates large-capacity Flash and PSRAM, and has an on-board 5-inch wide capacitive touch LCD screen, 
which can smoothly run GUI interface programs such as LVGL; 
it combines multiple peripheral interfaces (such as CAN, I2C and RS485 interfaces) to quickly develop ESP32-S3 HMI and other applications.

- [Purchase Link](https://www.waveshare.com/product/esp32-s3-lcd-7b.htm)
- [Documentation](https://docs.waveshare.net/ESP32-S3-Touch-LCD-7B)

![Product Image](./assets/Product-1.webp)

---

## 🔧 Configuration

You can find detailed configuration information on the product wiki page.

---

## 📺 RGB Panel Driver Initialization (Arduino_GFX / esp_lcd)

### Problem & Root Cause
The 7.0" LCD uses a 16-bit parallel RGB interface (`DE=GPIO 5`, `VSYNC=GPIO 3`, `HSYNC=GPIO 46`, `PCLK=GPIO 7`, plus 16 RGB data lines).
Flashing standard firmware without initializing the `Arduino_ESP32RGBPanel` or `esp_lcd_panel_rgb` driver leaves the pixel clock (`PCLK`) and sync lines undriven, resulting in a blank or uninitialized display.

### Arduino IDE Fix & Setup
1. Install **GFX Library for Arduino** by Moon On Our Nation in Arduino IDE Library Manager.
2. Ensure **PSRAM / OPTI RAM** is enabled in board options (`OPI PSRAM` / `board_build.arduino.memory_type = qio_opi`) so the large frame buffer (page buffer) is allocated in Octal RAM rather than internal SRAM.
3. Configure `config.h` to set `#define USE_OPTI_RAM_PAGE_BUFFER 1` and instantiate `Arduino_ESP32RGBPanel`:

```cpp
#include <Arduino_GFX_Library.h>
#include "config.h"

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
    TFT_R3, TFT_R4, TFT_R5, TFT_R6, TFT_R7,
    TFT_G2, TFT_G3, TFT_G4, TFT_G5, TFT_G6, TFT_G7,
    TFT_B3, TFT_B4, TFT_B5, TFT_B6, TFT_B7,
    HSYNC_POLARITY, HSYNC_FRONT_PORCH, HSYNC_PULSE_WIDTH, HSYNC_BACK_PORCH,
    VSYNC_POLARITY, VSYNC_FRONT_PORCH, VSYNC_PULSE_WIDTH, VSYNC_BACK_PORCH,
    PCLK_ACTIVE_NEG, PREFER_SPEED,
    false, /* use_big_endian */
    BOUNCE_BUFFER_SIZE_PX, /* SRAM bounce buffer for DMA */
    USE_OPTI_RAM_PAGE_BUFFER /* Allocate page/frame buffer in OPTI RAM (Octal PSRAM) */
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(TFT_WIDTH, TFT_HEIGHT, rgbpanel);
```

An example sketch, header, and `platformio.ini` are available under [examples/Arduino/01_RGB_Display/](examples/Arduino/01_RGB_Display/).

### ⚙️ PlatformIO Board Configuration (`platformio.ini`)
> [!IMPORTANT]
> Do **NOT** set `board = esp32` in PlatformIO. The Waveshare ESP32-S3-Touch-LCD-7B uses an **ESP32-S3** chip. Setting `board = esp32` will fail with an invalid/wrong board error.

Use the correct ESP32-S3 board definition (`board = esp32-s3-devkitc-1`) and 8MB OPI PSRAM options:

```ini
[env:esp32s3_arduino]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200

board_build.mcu = esp32s3
board_build.f_cpu = 240000000L
board_build.f_flash = 80000000L
board_build.flash_mode = qio
board_build.flash_size = 16MB
board_build.arduino.memory_type = qio_opi
board_build.psram_type = opi

build_flags = 
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1

lib_deps = 
    moononournation/GFX Library for Arduino@^1.4.9
```

---

## 🛠️ Contributing

We welcome contributions! Here’s how you can help:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Commit your changes with clear descriptions.
4. Submit a pull request for review.

---

## 🧩 Issues and Support

If you encounter any issues:

- Check the [Issues](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-7B/issues) section.
- Create a new issue with detailed information.
- Refer to the documentation for troubleshooting tips.
- Contact the Waveshare team and provide the order number to obtain technical support.

---

## 📜 License

This repository is licensed under the Apache License License. See the [LICENSE](LICENSE) file for details.

---

## 🙌 Acknowledgments

- Waveshare for their excellent hardware platforms and software support
- The Espressif Team for their continuous support.
- Open-source contributors who make these projects possible.

---

Thank you for using Waveshare Electronics Products! 🚀