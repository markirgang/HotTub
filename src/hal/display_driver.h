#pragma once
#include <Arduino.h>

#if __has_include("include/lv_conf.h")
#include "include/lv_conf.h"
#elif __has_include("../lv_conf.h")
#include "../lv_conf.h"
#elif __has_include("lv_conf.h")
#include "lv_conf.h"
#endif

#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

class DisplayDriver {
public:
    static DisplayDriver& getInstance() {
        static DisplayDriver instance;
        return instance;
    }

    bool begin();
    
    // UI thread synchronization mutex
    bool lockUI(TickType_t timeout = pdMS_TO_TICKS(100));
    void unlockUI();

    void setBrightness(uint8_t percent);
    uint8_t getBrightness() const { return _brightness; }

    void wakeScreen();
    void sleepScreen();
    bool isScreenAwake() const { return _screenAwake; }

private:
    DisplayDriver();
    ~DisplayDriver();

    static void lvglTask(void* pvParameters);
    static void dispFlushCallback(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p);
    static void touchReadCallback(lv_indev_drv_t* drv, lv_indev_data_t* data);

    bool initRgbPanel();
    bool initTouch();
    bool readTouchCoordinates(int16_t* x, int16_t* y);

    SemaphoreHandle_t _uiMutex;
    lv_disp_drv_t _dispDrv;
    lv_indev_drv_t _indevDrv;
    lv_color_t* _drawBuf1;
    lv_color_t* _drawBuf2;
    lv_disp_draw_buf_t _drawBufStruct;

    void* _panelHandle;
    uint8_t _touchI2cAddr;
    uint8_t _brightness;
    bool _screenAwake;
    uint32_t _lastTouchTime;
};

#define Display DisplayDriver::getInstance()
