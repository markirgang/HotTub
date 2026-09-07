#include <Arduino_GFX_Library.h>
#include "config.h"

/*
 * 7.0" LCD 16-bit Parallel RGB Panel Initialization using config.h
 * 
 * Configured with OPTI RAM / Octal PSRAM page buffer allocation
 * to resolve blank screen issues caused by internal SRAM memory exhaustion.
 */

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

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing ESP32-S3 7.0\" RGB LCD...");
    Serial.printf("OPTI RAM Page Buffer Allocation: %s\n", USE_OPTI_RAM_PAGE_BUFFER ? "ENABLED" : "DISABLED");

    // Initialize RGB Panel and GFX Library
    if (!gfx->begin()) {
        Serial.println("RGB Display Initialization Failed! Check PSRAM / OPTI RAM settings.");
        return;
    }

    gfx->fillScreen(WHITE);
    gfx->setTextColor(BLACK);
    gfx->setTextSize(3);
    gfx->setCursor(50, 50);
    gfx->println("ESP32-S3 7.0\" Touch LCD");
    gfx->setCursor(50, 100);
    gfx->println("RGB Panel Driver Initialized");
    gfx->setCursor(50, 150);
    gfx->println("Page Buffer: OPTI RAM Active");
}

void loop() {
    delay(1000);
}
