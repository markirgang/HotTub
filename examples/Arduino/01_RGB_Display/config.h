#ifndef CONFIG_H
#define CONFIG_H

// ==============================================================================
// Memory & Buffer Configuration (OPTI RAM / Octal PSRAM for Page/Frame Buffer)
// ==============================================================================
// Set to 1 to enable Octal PSRAM (OPTI RAM) allocation for page/frame buffers.
// Essential for 7.0" LCDs (800x480 / 800x600 / 1024x600) to prevent internal SRAM overflow.
#define USE_OPTI_RAM_PAGE_BUFFER  1

// Bounce buffer size in pixels (allocated in SRAM for smooth DMA transfer)
#define BOUNCE_BUFFER_SIZE_PX     (800 * 20)

// ==============================================================================
// Display Resolution Parameters
// ==============================================================================
#define TFT_WIDTH   800
#define TFT_HEIGHT  600

// ==============================================================================
// GPIO Pin Assignments (Waveshare ESP32-S3-Touch-LCD-7B)
// ==============================================================================
#define TFT_DE      5
#define TFT_VSYNC   3
#define TFT_HSYNC   46
#define TFT_PCLK    7

// Red Data Lines (R3 - R7)
#define TFT_R3      1
#define TFT_R4      2
#define TFT_R5      42
#define TFT_R6      41
#define TFT_R7      40

// Green Data Lines (G2 - G7)
#define TFT_G2      39
#define TFT_G3      0
#define TFT_G4      45
#define TFT_G5      48
#define TFT_G6      47
#define TFT_G7      21

// Blue Data Lines (B3 - B7)
#define TFT_B3      14
#define TFT_B4      38
#define TFT_B5      18
#define TFT_B6      17
#define TFT_B7      10

// ==============================================================================
// Display Timing Parameters
// ==============================================================================
#define HSYNC_POLARITY     1
#define HSYNC_FRONT_PORCH  48
#define HSYNC_PULSE_WIDTH  162
#define HSYNC_BACK_PORCH   152

#define VSYNC_POLARITY     1
#define VSYNC_FRONT_PORCH  3
#define VSYNC_PULSE_WIDTH  45
#define VSYNC_BACK_PORCH   13

#define PCLK_ACTIVE_NEG    1
#define PREFER_SPEED       16000000

#endif // CONFIG_H
