#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

class BLESpaController : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    static BLESpaController& getInstance() {
        static BLESpaController instance;
        return instance;
    }

    bool begin();
    void update(); // Sends periodic BLE notify updates

    bool isClientConnected() const { return _deviceConnected; }

    // BLE Server Callbacks
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;

    // BLE Characteristic Callbacks
    void onWrite(BLECharacteristic* pCharacteristic) override;

private:
    BLESpaController();

    BLEServer* _pServer;
    BLECharacteristic* _pTelemetryChar;
    BLECharacteristic* _pCommandChar;
    BLECharacteristic* _pStatusChar;

    bool _deviceConnected;
    bool _oldDeviceConnected;
    uint32_t _lastNotifyTime;

    void handleBleCommand(const String& cmd);
};

#define SpaBLE BLESpaController::getInstance()
