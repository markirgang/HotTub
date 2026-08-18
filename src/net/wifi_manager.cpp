#include "wifi_manager.h"
#include "core/config_manager.h"
#include "core/scheduler.h"

WiFiManager::WiFiManager()
    : _connected(false),
      _apMode(false),
      _currentSSID(""),
      _lastReconnectAttempt(0) {
}

bool WiFiManager::begin() {
    String storedSSID = Config.getWifiSSID();
    String storedPass = Config.getWifiPass();

    if (storedSSID.length() > 0) {
        connectToWiFi(storedSSID, storedPass);
    } else {
        startAccessPoint();
    }
    return true;
}

void WiFiManager::connectToWiFi(const String& ssid, const String& pass) {
    _currentSSID = ssid;
    _apMode = false;
    _connected = false;

    log_i("Connecting to WiFi: %s ...", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(DEFAULT_HOSTNAME);
    WiFi.begin(ssid.c_str(), pass.c_str());

    uint32_t startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt < 10000)) {
        delay(250);
        yield();
    }

    if (WiFi.status() == WL_CONNECTED) {
        _connected = true;
        log_i("WiFi Connected! IP Address: %s", WiFi.localIP().toString().c_str());
        SpaScheduler.syncNTP();
    } else {
        log_w("WiFi Connection failed! Starting fallback SoftAP...");
        startAccessPoint();
    }
}

void WiFiManager::startAccessPoint() {
    _apMode = true;
    _connected = false;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASS);
    
    // Start DNS captive portal on port 53 redirecting to AP IP 192.168.4.1
    _dnsServer.start(53, "*", WiFi.softAPIP());

    log_i("Hot Tub SoftAP Started: SSID '%s' (Password: '%s')", DEFAULT_AP_SSID, DEFAULT_AP_PASS);
    log_i("AP Web Interface reachable at: http://%s", WiFi.softAPIP().toString().c_str());
}

void WiFiManager::update() {
    if (_apMode) {
        _dnsServer.processNextRequest();
    } else {
        // Auto-reconnect if connection dropped
        if (WiFi.status() != WL_CONNECTED) {
            _connected = false;
            uint32_t now = millis();
            if (now - _lastReconnectAttempt >= 30000) {
                _lastReconnectAttempt = now;
                log_i("Attempting to reconnect to WiFi: %s ...", _currentSSID.c_str());
                WiFi.reconnect();
            }
        } else {
            _connected = true;
        }
    }
}

String WiFiManager::getIPAddress() const {
    if (_apMode) {
        return WiFi.softAPIP().toString();
    } else if (_connected) {
        return WiFi.localIP().toString();
    }
    return "Disconnected";
}
