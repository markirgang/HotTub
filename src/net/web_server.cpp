#include "web_server.h"
#include "core/spa_controller.h"
#include "core/config_manager.h"
#include "core/scheduler.h"
#include "net/wifi_manager.h"
#include <ArduinoJson.h>

// Embedded fallback HTML webpage stored in flash
static const char INDEX_HTML_PROGMEM[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Hot Tub Smart Controller</title>
  <link rel="stylesheet" href="/style.css">
  <style>
    :root { --bg: #0d131f; --card: #162032; --accent: #00d2ff; --text: #f0f4f8; --subtext: #8fa0b5; --orange: #ff7849; --green: #2ecc71; --red: #e74c3c; }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background: var(--bg); color: var(--text); padding: 16px; display: flex; justify-content: center; }
    .container { max-width: 600px; width: 100%; display: flex; flex-direction: column; gap: 16px; }
    .card { background: var(--card); border-radius: 16px; padding: 20px; box-shadow: 0 4px 20px rgba(0,0,0,0.3); border: 1px solid #23334d; }
    .header { display: flex; justify-content: space-between; align-items: center; }
    .title { font-size: 20px; font-weight: 700; color: var(--accent); }
    .status-badge { font-size: 13px; padding: 4px 10px; border-radius: 20px; background: rgba(46, 204, 113, 0.2); color: var(--green); font-weight: 600; }
    .temp-display { text-align: center; margin: 20px 0; }
    .temp-current { font-size: 64px; font-weight: 800; letter-spacing: -2px; }
    .temp-target { font-size: 18px; color: var(--subtext); margin-top: 4px; }
    .temp-controls { display: flex; justify-content: center; gap: 20px; align-items: center; }
    .btn-circle { width: 56px; height: 56px; border-radius: 50%; border: none; font-size: 28px; font-weight: bold; color: #fff; cursor: pointer; display: flex; align-items: center; justify-content: center; transition: 0.1s; }
    .btn-down { background: #2980b9; }
    .btn-up { background: #c0392b; }
    .btn-circle:active { transform: scale(0.92); }
    .grid-2 { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }
    .control-btn { background: #23334d; border: 1px solid #334769; color: var(--text); padding: 16px; border-radius: 12px; font-size: 16px; font-weight: 600; cursor: pointer; text-align: center; display: flex; flex-direction: column; gap: 6px; transition: 0.2s; }
    .control-btn.active { background: #0077b6; border-color: var(--accent); color: #fff; }
    .control-btn.active-green { background: #1b4332; border-color: var(--green); color: #a7f3d0; }
    .control-btn.active-purple { background: #4a154b; border-color: #d946ef; color: #fbcfe8; }
    .control-btn.active-orange { background: #7c2d12; border-color: var(--orange); color: #fed7aa; }
    .timer-text { font-size: 12px; color: var(--subtext); }
    .slider-box { margin-top: 14px; }
    .slider-label { display: flex; justify-content: space-between; font-size: 14px; margin-bottom: 6px; color: var(--subtext); }
    input[type=range] { width: 100%; height: 8px; border-radius: 4px; accent-color: var(--accent); }
    .mode-bar { display: flex; gap: 8px; margin-top: 10px; }
    .mode-btn { flex: 1; padding: 10px; font-size: 13px; font-weight: 600; border-radius: 8px; border: 1px solid #334769; background: #1c283c; color: var(--subtext); cursor: pointer; }
    .mode-btn.active { background: var(--accent); color: #000; border-color: var(--accent); }
  </style>
</head>
<body>
  <div class="container">
    <div class="card header">
      <div class="title">&#9832; Hot Tub Control</div>
      <div id="statusBadge" class="status-badge">Ready</div>
    </div>

    <!-- Temperature Card -->
    <div class="card temp-display">
      <div id="waterTemp" class="temp-current">--&deg;F</div>
      <div id="targetTemp" class="temp-target">Set: --&deg;F</div>
      <div class="temp-controls" style="margin-top: 16px;">
        <button class="btn-circle btn-down" onclick="adjustTemp(-1)">-</button>
        <button class="btn-circle btn-up" onclick="adjustTemp(1)">+</button>
      </div>
    </div>

    <!-- Main Controls Grid -->
    <div class="grid-2">
      <button id="btnJet1" class="control-btn" onclick="toggleJet1()">
        <span>Jets 1</span>
        <span id="jet1Status" style="font-size:13px; color:var(--accent);">OFF</span>
        <span id="jet1Timer" class="timer-text">--</span>
      </button>

      <button id="btnJet2" class="control-btn" onclick="toggleJet2()">
        <span>Jets 2</span>
        <span id="jet2Status" style="font-size:13px; color:var(--accent);">OFF</span>
        <span class="timer-text">Aux Jet</span>
      </button>

      <button id="btnBlower" class="control-btn" onclick="toggleBlower()">
        <span>Air Blower</span>
        <span id="blowerStatus" style="font-size:13px; color:#d946ef;">OFF</span>
        <span id="blowerTimer" class="timer-text">--</span>
      </button>

      <button id="btnLight" class="control-btn" onclick="toggleLight()">
        <span>Spa Light</span>
        <span id="lightStatus" style="font-size:13px; color:#f59e0b;">OFF</span>
        <span class="timer-text">LED Underwater</span>
      </button>
    </div>

    <!-- Blower Speed Slider Card -->
    <div class="card">
      <div class="slider-label">
        <span>Blower Speed (PWM / Variable)</span>
        <span id="blowerPctText">0%</span>
      </div>
      <input id="sliderBlower" type="range" min="0" max="100" value="0" oninput="onBlowerSlider(this.value)">
    </div>

    <!-- Operating Modes -->
    <div class="card">
      <div style="font-size: 14px; color: var(--subtext); margin-bottom: 8px;">Operating Mode</div>
      <div class="mode-bar">
        <button id="mode0" class="mode-btn" onclick="setMode(0)">Standard</button>
        <button id="mode1" class="mode-btn" onclick="setMode(1)">Eco</button>
        <button id="mode2" class="mode-btn" onclick="setMode(2)">Party</button>
        <button id="mode3" class="mode-btn" onclick="setMode(3)">Clean</button>
      </div>
    </div>

    <!-- Diagnostics / Safety Info -->
    <div class="card" style="font-size: 13px; color: var(--subtext); line-height: 1.6;">
      <div><strong>Flow Switch:</strong> <span id="diagFlow" style="color:var(--green)">OK</span> | <strong>Heater:</strong> <span id="diagHeater">OFF</span></div>
      <div><strong>Safety Limit:</strong> <span id="diagLimit" style="color:var(--green)">OK</span> | <strong>Water Level:</strong> <span id="diagLevel">OK</span></div>
      <div><strong>Bluetooth App:</strong> <a href="/ble_app.html" style="color:var(--accent); text-decoration:none;">Open Web BLE Direct Client &rarr;</a></div>
    </div>
  </div>

  <script>
    let ws;
    let spaData = {};

    function connectWs() {
      const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
      ws = new WebSocket(protocol + '//' + location.host + '/ws');
      
      ws.onmessage = (event) => {
        spaData = JSON.parse(event.data);
        renderState(spaData);
      };
      
      ws.onclose = () => {
        setTimeout(connectWs, 2000);
      };
    }

    function renderState(d) {
      document.getElementById('waterTemp').innerHTML = d.water_temp_f + '&deg;' + d.unit;
      document.getElementById('targetTemp').innerHTML = 'Set: ' + d.target_temp_f + '&deg;' + d.unit;
      document.getElementById('statusBadge').innerText = d.status || 'Ready';

      // Jets 1
      const j1 = document.getElementById('btnJet1');
      const j1s = document.getElementById('jet1Status');
      const j1t = document.getElementById('jet1Timer');
      j1.className = 'control-btn' + (d.pump1 > 0 ? (d.pump1 === 2 ? ' active' : ' active-green') : '');
      j1s.innerText = d.pump1 === 2 ? 'HIGH' : (d.pump1 === 1 ? 'LOW' : 'OFF');
      j1t.innerText = d.jet_remaining > 0 ? Math.floor(d.jet_remaining/60) + ':' + (d.jet_remaining%60).toString().padStart(2,'0') : '--';

      // Jets 2
      const j2 = document.getElementById('btnJet2');
      const j2s = document.getElementById('jet2Status');
      j2.className = 'control-btn' + (d.pump2 > 0 ? ' active' : '');
      j2s.innerText = d.pump2 > 0 ? 'ON' : 'OFF';

      // Blower
      const blw = document.getElementById('btnBlower');
      const blws = document.getElementById('blowerStatus');
      const blwt = document.getElementById('blowerTimer');
      blw.className = 'control-btn' + (d.blower > 0 ? ' active-purple' : '');
      blws.innerText = d.blower === 3 ? 'HIGH' : (d.blower === 2 ? 'MED' : (d.blower === 1 ? 'LOW' : 'OFF'));
      blwt.innerText = d.blower_remaining > 0 ? Math.floor(d.blower_remaining/60) + ':' + (d.blower_remaining%60).toString().padStart(2,'0') : '--';
      document.getElementById('sliderBlower').value = d.blower_pct || 0;
      document.getElementById('blowerPctText').innerText = (d.blower_pct || 0) + '%';

      // Light
      const lgt = document.getElementById('btnLight');
      const lgts = document.getElementById('lightStatus');
      lgt.className = 'control-btn' + (d.light ? ' active-orange' : '');
      lgts.innerText = d.light ? 'ON' : 'OFF';

      // Modes
      for (let i=0; i<4; i++) {
        const m = document.getElementById('mode' + i);
        if (m) m.className = 'mode-btn' + (d.mode === i ? ' active' : '');
      }

      // Safety
      document.getElementById('diagFlow').innerText = d.flow_ok ? 'OK' : 'FAULT (NO FLOW)';
      document.getElementById('diagFlow').style.color = d.flow_ok ? 'var(--green)' : 'var(--red)';
      document.getElementById('diagHeater').innerText = d.heater === 2 ? 'HEATING' : (d.heater === 1 ? 'PRE-FLOW' : 'OFF');
      document.getElementById('diagLimit').innerText = d.high_limit_ok ? 'OK' : 'TRIPPED';
      document.getElementById('diagLevel').innerText = d.water_level_ok ? 'OK' : 'LOW';
    }

    function sendCmd(obj) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(obj));
      }
    }

    function adjustTemp(delta) {
      const cur = spaData.target_temp_f || 100;
      sendCmd({ action: 'set_temp', temp: cur + delta });
    }

    function toggleJet1() {
      const next = (spaData.pump1 === 0) ? 1 : ((spaData.pump1 === 1) ? 2 : 0);
      sendCmd({ action: 'set_jet1', speed: next });
    }

    function toggleJet2() {
      sendCmd({ action: 'set_jet2', speed: spaData.pump2 > 0 ? 0 : 2 });
    }

    function toggleBlower() {
      const next = (spaData.blower === 0) ? 1 : ((spaData.blower === 1) ? 2 : ((spaData.blower === 2) ? 3 : 0));
      sendCmd({ action: 'set_blower', speed: next });
    }

    function onBlowerSlider(val) {
      document.getElementById('blowerPctText').innerText = val + '%';
      sendCmd({ action: 'set_blower_pct', pct: parseInt(val) });
    }

    function toggleLight() {
      sendCmd({ action: 'set_light', state: !spaData.light });
    }

    function setMode(m) {
      sendCmd({ action: 'set_mode', mode: m });
    }

    window.onload = connectWs;
  </script>
</body>
</html>
)rawliteral";

SpaWebServer::SpaWebServer()
    : _server(80),
      _ws("/ws"),
      _lastBroadcastTime(0) {
}

bool SpaWebServer::begin() {
    log_i("Starting Spa Web Server & WebSockets...");

    // Mount LittleFS if available
    if (LittleFS.begin(false)) {
        log_i("LittleFS mounted successfully.");
    } else {
        log_w("LittleFS mount failed! Formatting or using PROGMEM web app...");
    }

    // Attach WebSockets
    _ws.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onWsEvent(server, client, type, arg, data, len);
    });
    _server.addHandler(&_ws);

    // Register HTTP & REST Routes
    registerRoutes();

    _server.begin();
    log_i("HTTP & WebSocket server listening on port 80.");
    return true;
}

void SpaWebServer::registerRoutes() {
    // 1. Web dashboard home
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (LittleFS.exists("/index.html")) {
            request->send(LittleFS, "/index.html", "text/html");
        } else {
            request->send_P(200, "text/html", INDEX_HTML_PROGMEM);
        }
    });

    // 2. Serve static files from LittleFS
    _server.serveStatic("/", LittleFS, "/");

    // 3. REST API - Status
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        String json = Spa.getStatusJson();
        request->send(200, "application/json", json);
    });

    // 4. REST API - Control
    _server.on("/api/control", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, (const char*)data);
        if (!err) {
            if (doc["target_temp"].is<float>()) {
                Spa.setTargetTemperatureF(doc["target_temp"].as<float>());
            }
            if (doc["pump1"].is<int>()) {
                Spa.setPump1Speed((PumpSpeed)doc["pump1"].as<int>());
            }
            if (doc["pump2"].is<int>()) {
                Spa.setPump2Speed((PumpSpeed)doc["pump2"].as<int>());
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
            request->send(200, "application/json", "{\"status\":\"ok\"}");
            SpaWeb.broadcastStatus();
        } else {
            request->send(400, "application/json", "{\"error\":\"bad_json\"}");
        }
    });

    // 5. REST API - Schedules
    _server.on("/api/schedule", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        if (!deserializeJson(doc, (const char*)data)) {
            if (doc["f1_start_hr"].is<int>()) {
                FilterCycleConfig f1 = Config.getFilterCycle1();
                f1.startHour = doc["f1_start_hr"].as<int>();
                if (doc["f1_dur"].is<int>()) f1.durationMins = doc["f1_dur"].as<int>();
                Config.setFilterCycle1(f1);
            }
            if (doc["f2_start_hr"].is<int>()) {
                FilterCycleConfig f2 = Config.getFilterCycle2();
                f2.startHour = doc["f2_start_hr"].as<int>();
                if (doc["f2_dur"].is<int>()) f2.durationMins = doc["f2_dur"].as<int>();
                Config.setFilterCycle2(f2);
            }
            request->send(200, "application/json", "{\"status\":\"saved\"}");
        } else {
            request->send(400, "application/json", "{\"error\":\"bad_json\"}");
        }
    });

    // 6. REST API - WiFi Setup
    _server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        if (!deserializeJson(doc, (const char*)data)) {
            String ssid = doc["ssid"] | "";
            String pass = doc["pass"] | "";
            if (ssid.length() > 0) {
                Config.setWifiCredentials(ssid, pass);
                request->send(200, "application/json", "{\"status\":\"wifi_saved_rebooting\"}");
                delay(1000);
                ESP.restart();
            }
        }
        request->send(400, "application/json", "{\"error\":\"invalid_ssid\"}");
    });
}

void SpaWebServer::onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        log_i("WebSocket client #%u connected from %s", client->id(), client->remoteIP().toString().c_str());
        client->text(Spa.getStatusJson());
    } else if (type == WS_EVT_DISCONNECT) {
        log_i("WebSocket client #%u disconnected", client->id());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            String msg = "";
            for (size_t i = 0; i < len; i++) {
                msg += (char)data[i];
            }
            handleWsMessage(client, msg);
        }
    }
}

void SpaWebServer::handleWsMessage(AsyncWebSocketClient* client, const String& msg) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) return;

    String action = doc["action"] | "";

    if (action == "set_temp") {
        float temp = doc["temp"] | 100.0f;
        Spa.setTargetTemperatureF(temp);
    } else if (action == "set_jet1") {
        int spd = doc["speed"] | 0;
        Spa.setPump1Speed((PumpSpeed)spd);
    } else if (action == "set_jet2") {
        int spd = doc["speed"] | 0;
        Spa.setPump2Speed((PumpSpeed)spd);
    } else if (action == "set_blower") {
        int spd = doc["speed"] | 0;
        Spa.setBlowerSpeed((BlowerSpeed)spd);
    } else if (action == "set_blower_pct") {
        int pct = doc["pct"] | 0;
        Spa.setBlowerPercent(pct);
    } else if (action == "set_light") {
        bool state = doc["state"] | false;
        Spa.setLightState(state);
    } else if (action == "set_mode") {
        int m = doc["mode"] | 0;
        Spa.setOperatingMode((SpaMode)m);
    }

    broadcastStatus();
}

void SpaWebServer::broadcastStatus() {
    if (_ws.count() > 0) {
        String json = Spa.getStatusJson();
        _ws.textAll(json);
    }
}

void SpaWebServer::update() {
    uint32_t now = millis();
    if (now - _lastBroadcastTime >= 500) { // 2Hz live stream to connected browsers
        _lastBroadcastTime = now;
        broadcastStatus();
    }
}
