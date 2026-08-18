#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>

#if __has_include("include/config.h")
#include "include/config.h"
#elif __has_include("../config.h")
#include "../config.h"
#else
#include "config.h"
#endif

class WiFiManager {
public:
    static WiFiManager& getInstance() {
        static WiFiManager instance;
        return instance;
    }

    bool begin();
    void update(); // Handles reconnects and captive portal DNS

    bool isConnected() const { return _connected; }
    bool isAPMode() const { return _apMode; }
    String getIPAddress() const;
    String getSSID() const { return _currentSSID; }

    void connectToWiFi(const String& ssid, const String& pass);
    void startAccessPoint();

private:
    WiFiManager();

    bool _connected;
    bool _apMode;
    String _currentSSID;
    DNSServer _dnsServer;
    uint32_t _lastReconnectAttempt;
};

#define SpaWiFi WiFiManager::getInstance()
