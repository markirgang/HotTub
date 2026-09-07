# Waveshare ESP32-S3-Touch-LCD-7B 产品工程示例程序

[English](README.md)

其支持 2.4GHz WiFi 和 BLE 5，集成大容量 Flash 和 PSRAM，板载 5 英寸宽电容触摸LCD屏，可流畅运行 LVGL 等 GUI 界面程序；结合多种外设接口(如：CAN、I2C和RS485等接口)，快速开发 ESP32-S3 的 HMI 等应用。

- [购买链接](https://www.waveshare.net/shop/ESP32-S3-Touch-LCD-7B.htm)
- [产品文档](https://docs.waveshare.net/ESP32-S3-Touch-LCD-7B/)

![主图](./assets/Product-1.webp)

---

## 🔧 配置

您可以在产品 Wiki 页面上找到详细的配置信息。

---

## 📺 RGB 面板驱动初始化 (Arduino_GFX / esp_lcd)

### 问题与根本原因
7.0 英寸 LCD 使用 16 位并行 RGB 接口（`DE=GPIO 5`, `VSYNC=GPIO 3`, `HSYNC=GPIO 46`, `PCLK=GPIO 7` 以及 16 条 RGB 数据线）。
若直接烧录标准固件而未初始化 `Arduino_ESP32RGBPanel` 或 `esp_lcd_panel_rgb` 驱动，会导致像素时钟（`PCLK`）和同步信号线处于未驱动状态，从而导致屏幕黑屏或无法正常显示。

### Arduino IDE 修复与配置
1. 在 Arduino IDE 库管理器中安装由 Moon On Our Nation 开发的 **GFX Library for Arduino**。
2. 确保在开发板设置中启用了 **PSRAM / OPTI RAM**（`OPI PSRAM` / `board_build.arduino.memory_type = qio_opi`），以确保显存（Page Buffer / Frame Buffer）分配在 Octal RAM 中，避免内部 SRAM 内存溢出导致黑屏。
3. 在 `config.h` 中设置 `#define USE_OPTI_RAM_PAGE_BUFFER 1`，并实例化 `Arduino_ESP32RGBPanel`：

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
    BOUNCE_BUFFER_SIZE_PX, /* SRAM DMA 弹跳缓冲区 */
    USE_OPTI_RAM_PAGE_BUFFER /* 在 OPTI RAM (Octal PSRAM) 中分配 Page/Frame Buffer */
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(TFT_WIDTH, TFT_HEIGHT, rgbpanel);
```

完整示例程序及配置文件请参考 [examples/Arduino/01_RGB_Display/](examples/Arduino/01_RGB_Display/)。

---

## 🛠️ 贡献

我们欢迎您的贡献！您可以通过以下方式提供帮助：

1. Fork 本仓库。
2. 为您的新功能或 Bug 修复创建一个新分支。
3. 提交您的更改并附上清晰的描述。
4. 提交 Pull Request 以供审核。

---

## 🧩 问题与支持

如果您遇到任何问题：

- 请先查看 [Issues](https://gitee.com/waveshare/esp32-s3-touch-lcd-7B/issues) 版块。
- 创建一个新的 Issue 并提供详细信息。
- 参考文档获取故障排除提示。
- 联系微雪团队并提供订单号以获取技术支持。

---

## 📜 许可

本仓库遵循 Apache License 许可。详情请参阅 [LICENSE](LICENSE) 文件。

---

## 🙌 致谢

- 感谢微雪电子提供的优秀硬件平台和软件支持。
- 感谢乐鑫团队的持续支持。
- 感谢让这些项目成为可能的开源贡献者。

---

感谢您使用微雪电子产品！🚀