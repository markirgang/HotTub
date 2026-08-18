#include "display_driver.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "i2c_bus.h"
#include "ch422g.h"

DisplayDriver::DisplayDriver()
    : _uiMutex(nullptr),
      _drawBuf1(nullptr),
      _drawBuf2(nullptr),
      _panelHandle(nullptr),
      _touchI2cAddr(TOUCH_GT911_I2C_ADDR_1),
      _brightness(100),
      _screenAwake(true),
      _lastTouchTime(0) {
    _uiMutex = xSemaphoreCreateMutex();
}

DisplayDriver::~DisplayDriver() {
    if (_uiMutex) {
        vSemaphoreDelete(_uiMutex);
    }
}

bool DisplayDriver::lockUI(TickType_t timeout) {
    if (!_uiMutex) return false;
    return (xSemaphoreTake(_uiMutex, timeout) == pdTRUE);
}

void DisplayDriver::unlockUI() {
    if (_uiMutex) {
        xSemaphoreGive(_uiMutex);
    }
}

void DisplayDriver::dispFlushCallback(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    
    // Draw directly to ESP-IDF RGB panel
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_p);
    
    lv_disp_flush_ready(drv);
}

void DisplayDriver::touchReadCallback(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    DisplayDriver* self = (DisplayDriver*)drv->user_data;
    int16_t touchX = 0, touchY = 0;
    
    if (self->readTouchCoordinates(&touchX, &touchY)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
        self->_lastTouchTime = millis();
        if (!self->_screenAwake) {
            self->wakeScreen();
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

bool DisplayDriver::readTouchCoordinates(int16_t* x, int16_t* y) {
    uint8_t status = 0;
    
    // Read status register 0x814E
    uint8_t regAddr[2] = {0x81, 0x4E};
    if (!I2CBus.lock()) return false;
    
    TwoWire& wire = I2CBus.getWire();
    wire.beginTransmission(_touchI2cAddr);
    wire.write(regAddr[0]);
    wire.write(regAddr[1]);
    if (wire.endTransmission(false) != 0) {
        I2CBus.unlock();
        return false;
    }
    
    if (wire.requestFrom(_touchI2cAddr, (uint8_t)1) == 1) {
        status = wire.read();
    }
    I2CBus.unlock();

    bool ready = (status & 0x80) != 0;
    uint8_t touchCount = status & 0x0F;

    if (ready && touchCount > 0) {
        uint8_t ptBuf[6];
        uint8_t ptAddr[2] = {0x81, 0x50};
        
        if (I2CBus.lock()) {
            wire.beginTransmission(_touchI2cAddr);
            wire.write(ptAddr[0]);
            wire.write(ptAddr[1]);
            wire.endTransmission(false);
            
            if (wire.requestFrom(_touchI2cAddr, (uint8_t)6) == 6) {
                for (int i = 0; i < 6; i++) {
                    ptBuf[i] = wire.read();
                }
            }
            
            // Clear status register
            wire.beginTransmission(_touchI2cAddr);
            wire.write(0x81);
            wire.write(0x4E);
            wire.write(0x00);
            wire.endTransmission();
            
            I2CBus.unlock();
            
            int16_t rawX = ptBuf[1] | (ptBuf[2] << 8);
            int16_t rawY = ptBuf[3] | (ptBuf[4] << 8);
            
            // Constrain to 800x480 bounds
            if (rawX < 0) rawX = 0;
            if (rawX >= LCD_H_RES) rawX = LCD_H_RES - 1;
            if (rawY < 0) rawY = 0;
            if (rawY >= LCD_V_RES) rawY = LCD_V_RES - 1;
            
            *x = rawX;
            *y = rawY;
            return true;
        }
    } else if (ready) {
        // Clear status byte even when 0 touches to acknowledge
        if (I2CBus.lock()) {
            wire.beginTransmission(_touchI2cAddr);
            wire.write(0x81);
            wire.write(0x4E);
            wire.write(0x00);
            wire.endTransmission();
            I2CBus.unlock();
        }
    }
    
    return false;
}

bool DisplayDriver::initRgbPanel() {
    log_i("Initializing ESP32-S3 RGB LCD Panel (%dx%d)...", LCD_H_RES, LCD_V_RES);
    
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = LCD_PIXEL_CLOCK_HZ,
            .h_res = LCD_H_RES,
            .v_res = LCD_V_RES,
            .hsync_pulse_width = LCD_HSYNC_PULSE_WIDTH,
            .hsync_back_porch = LCD_HSYNC_BACK_PORCH,
            .hsync_front_porch = LCD_HSYNC_FRONT_PORCH,
            .vsync_pulse_width = LCD_VSYNC_PULSE_WIDTH,
            .vsync_back_porch = LCD_VSYNC_BACK_PORCH,
            .vsync_front_porch = LCD_VSYNC_FRONT_PORCH,
            .flags = {
                .hsync_idle_low = (LCD_HSYNC_POLARITY == 0),
                .vsync_idle_low = (LCD_VSYNC_POLARITY == 0),
                .de_idle_high = 0,
                .pclk_active_neg = LCD_PCLK_ACTIVE_NEG,
                .pclk_idle_high = 0,
            },
        },
        .data_width = 16,
        .bits_per_pixel = 16,
        .num_fbs = 2,
        .bounce_buffer_size_px = 0,
        .sram_trans_align = 4,
        .psram_trans_align = 64,
        .hsync_gpio_num = LCD_PIN_HSYNC,
        .vsync_gpio_num = LCD_PIN_VSYNC,
        .de_gpio_num = LCD_PIN_DE,
        .pclk_gpio_num = LCD_PIN_PCLK,
        .disp_gpio_num = -1,
        .data_gpio_nums = {
            LCD_PIN_B3, LCD_PIN_B4, LCD_PIN_B5, LCD_PIN_B6, LCD_PIN_B7,
            LCD_PIN_G2, LCD_PIN_G3, LCD_PIN_G4, LCD_PIN_G5, LCD_PIN_G6, LCD_PIN_G7,
            LCD_PIN_R3, LCD_PIN_R4, LCD_PIN_R5, LCD_PIN_R6, LCD_PIN_R7
        },
        .flags = {
            .disp_active_low = 0,
            .relax_on_idle = 0,
            .fb_in_psram = 1,
        },
    };

    esp_lcd_panel_handle_t panel = NULL;
    esp_err_t err = esp_lcd_new_rgb_panel(&panel_config, &panel);
    if (err != ESP_OK) {
        log_e("Failed to create RGB panel! Err: 0x%X", err);
        return false;
    }

    err = esp_lcd_panel_reset(panel);
    if (err != ESP_OK) log_w("Panel reset returned: 0x%X", err);

    err = esp_lcd_panel_init(panel);
    if (err != ESP_OK) {
        log_e("Panel init failed! Err: 0x%X", err);
        return false;
    }

    _panelHandle = (void*)panel;
    log_i("RGB LCD panel initialized successfully.");
    return true;
}

bool DisplayDriver::initTouch() {
    log_i("Initializing GT911 Touch Controller...");
    
    // Check which I2C address GT911 responds to (0x5D or 0x14)
    if (I2CBus.ping(TOUCH_GT911_I2C_ADDR_1)) {
        _touchI2cAddr = TOUCH_GT911_I2C_ADDR_1;
    } else if (I2CBus.ping(TOUCH_GT911_I2C_ADDR_2)) {
        _touchI2cAddr = TOUCH_GT911_I2C_ADDR_2;
    } else {
        log_w("GT911 Touch controller not responding on standard I2C addresses (0x5D/0x14)");
    }
    
    log_i("GT911 active on I2C address 0x%02X", _touchI2cAddr);
    return true;
}

void DisplayDriver::lvglTask(void* pvParameters) {
    DisplayDriver* self = (DisplayDriver*)pvParameters;
    log_i("LVGL GUI Task running on Core %d", xPortGetCoreID());
    
    while (1) {
        if (self->lockUI()) {
            lv_timer_handler();
            self->unlockUI();
        }
        vTaskDelay(pdMS_TO_TICKS(15)); // ~66 FPS loop
    }
}

bool DisplayDriver::begin() {
    log_i("Starting Display Subsystem...");

    // 1. Reset LCD & Touch via CH422G
    CH422G.resetDisplayAndTouch();

    // 2. Initialize RGB LCD
    if (!initRgbPanel()) {
        log_e("Display Driver RGB initialization failed!");
        return false;
    }

    // 3. Initialize Touch
    initTouch();

    // 4. Initialize LVGL
    lv_init();

    // 5. Allocate draw buffers (40 lines of 800px in PSRAM)
    size_t bufSize = LCD_H_RES * 40 * sizeof(lv_color_t);
    _drawBuf1 = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    _drawBuf2 = (lv_color_t*)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    
    if (!_drawBuf1) {
        log_e("Failed to allocate LVGL draw buffer 1 in PSRAM! Falling back to internal RAM...");
        _drawBuf1 = (lv_color_t*)malloc(LCD_H_RES * 20 * sizeof(lv_color_t));
    }

    lv_disp_draw_buf_init(&_drawBufStruct, _drawBuf1, _drawBuf2, _drawBuf2 ? (LCD_H_RES * 40) : (LCD_H_RES * 20));

    // 6. Register Display Driver in LVGL
    lv_disp_drv_init(&_dispDrv);
    _dispDrv.hor_res = LCD_H_RES;
    _dispDrv.ver_res = LCD_V_RES;
    _dispDrv.flush_cb = dispFlushCallback;
    _dispDrv.draw_buf = &_drawBufStruct;
    _dispDrv.user_data = _panelHandle;
    lv_disp_drv_register(&_dispDrv);

    // 7. Register Touch Input Device in LVGL
    lv_indev_drv_init(&_indevDrv);
    _indevDrv.type = LV_INDEV_TYPE_POINTER;
    _indevDrv.read_cb = touchReadCallback;
    _indevDrv.user_data = this;
    lv_indev_drv_register(&_indevDrv);

    // 8. Turn on LCD Backlight via CH422G
    CH422G.setBacklight(true);
    _screenAwake = true;

    // 9. Launch LVGL processing task on Core 1
    xTaskCreatePinnedToCore(
        lvglTask,
        "lvgl_task",
        8192,
        this,
        2,
        NULL,
        1
    );

    log_i("Display and LVGL GUI Task initialized successfully!");
    return true;
}

void DisplayDriver::setBrightness(uint8_t percent) {
    _brightness = (percent > 100) ? 100 : percent;
    if (_brightness == 0) {
        CH422G.setBacklight(false);
    } else {
        CH422G.setBacklight(true);
    }
}

void DisplayDriver::wakeScreen() {
    if (!_screenAwake) {
        CH422G.setBacklight(true);
        _screenAwake = true;
        log_i("Screen woke up.");
    }
}

void DisplayDriver::sleepScreen() {
    if (_screenAwake) {
        CH422G.setBacklight(false);
        _screenAwake = false;
        log_i("Screen entered sleep mode.");
    }
}
