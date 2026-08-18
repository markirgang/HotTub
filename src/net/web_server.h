#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "config.h"

class SpaWebServer {
public:
    static SpaWebServer& getInstance() {
        static SpaWebServer instance;
        return instance;
    }

    bool begin();
    void update(); // Broadcasts WebSocket telemetry updates (2Hz)

    void broadcastStatus();

private:
    SpaWebServer();

    AsyncWebServer _server;
    AsyncWebSocket _ws;
    
    uint32_t _lastBroadcastTime;

    void registerRoutes();
    void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
    void handleWsMessage(AsyncWebSocketClient* client, const String& msg);
};

#define SpaWeb SpaWebServer::getInstance()
