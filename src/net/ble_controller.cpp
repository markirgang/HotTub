#include "ble_controller.h"
#include "core/spa_controller.h"
#include <ArduinoJson.h>

BLESpaController::BLESpaController()
    : _pServer(nullptr),
      _pTelemetryChar(nullptr),
      _pCommandChar(nullptr),
      _pStatusChar(nullptr),
      _deviceConnected(false),
      _oldDeviceConnected(false),
      _lastNotifyTime(0) {
}

bool BLESpaController::begin() {
    log_i("Initializing BLE GATT Server ('HotTub-Controller')...");

    BLEDevice::init("HotTub-Controller");
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(this);

    BLEService* pService = _pServer->createService(BLE_SPA_SERVICE_UUID);

    // 1. Telemetry Characteristic (Read + Notify)
    _pTelemetryChar = pService->createCharacteristic(
        BLE_CHAR_TELEMETRY_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _pTelemetryChar->addDescriptor(new BLE2902());

    // 2. Command Characteristic (Write)
    _pCommandChar = pService->createCharacteristic(
        BLE_CHAR_COMMAND_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _pCommandChar->setCallbacks(this);

    // 3. Status Characteristic (Read + Notify)
    _pStatusChar = pService->createCharacteristic(
        BLE_CHAR_SCHEDULE_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _pStatusChar->addDescriptor(new BLE2902());

    pService->start();

    // Start Advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SPA_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    log_i("BLE Spa Control Service started and advertising.");
    return true;
}

void BLESpaController::onConnect(BLEServer* pServer) {
    _deviceConnected = true;
    log_i("BLE Client connected!");
}

void BLESpaController::onDisconnect(BLEServer* pServer) {
    _deviceConnected = false;
    log_i("BLE Client disconnected. Restarting advertising...");
    BLEDevice::startAdvertising();
}

void BLESpaController::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.length() > 0) {
        log_i("Received BLE command: %s", value.c_str());
        handleBleCommand(value);
    }
}

void BLESpaController::handleBleCommand(const String& cmd) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, cmd);
    if (!err) {
        if (doc["temp"].is<float>()) {
            Spa.setTargetTemperatureF(doc["temp"].as<float>());
        }
        if (doc["jet1"].is<int>()) {
            Spa.setPump1Speed((PumpSpeed)doc["jet1"].as<int>());
        }
        if (doc["jet2"].is<int>()) {
            Spa.setPump2Speed((PumpSpeed)doc["jet2"].as<int>());
        }
        if (doc["blower"].is<int>()) {
            Spa.setBlowerSpeed((BlowerSpeed)doc["blower"].as<int>());
        }
        if (doc["blower_pct"].is<int>()) {
            Spa.setBlowerPercent(doc["blower_pct"].as<int>());
        }
        if (doc["light"].is<bool>()) {
            Spa.setLightState(doc["light"].as<bool>());
        }
        if (doc["mode"].is<int>()) {
            Spa.setOperatingMode((SpaMode)doc["mode"].as<int>());
        }
    }
}

void BLESpaController::update() {
    // Restart advertising if connection dropped
    if (!_deviceConnected && _oldDeviceConnected) {
        delay(500);
        _pServer->startAdvertising();
        _oldDeviceConnected = _deviceConnected;
    }
    if (_deviceConnected && !_oldDeviceConnected) {
        _oldDeviceConnected = _deviceConnected;
    }

    // Push notification updates at 1Hz if a BLE device is connected
    uint32_t now = millis();
    if (_deviceConnected && (now - _lastNotifyTime >= 1000)) {
        _lastNotifyTime = now;
        String json = Spa.getStatusJson();
        if (_pTelemetryChar) {
            _pTelemetryChar->setValue(json.c_str());
            _pTelemetryChar->notify();
        }
    }
}
