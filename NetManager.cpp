/*
 * ============================================
 * NetManager.cpp - Implementasi WiFi & Web Server
 * ============================================
 * Fitur:
 * 1. Koneksi WiFi otomatis dengan retry
 * 2. REST API server untuk dashboard
 * 3. CORS support untuk akses dari browser
 * 4. Periodic score reporting via Serial
 * 
 * JSON Response Format:
 * {
 *   "device": "IoT-Tetris",
 *   "status": "playing",
 *   "score": 1500,
 *   "highScore": 3200,
 *   "level": 3,
 *   "lines": 8
 * }
 * ============================================
 */

#include "NetManager.h"

// ============================================
// CONSTRUCTOR
// ============================================
NetManager::NetManager(GameEngine& game)
  : _game(game),
    _server(WEB_SERVER_PORT),
    _lastReport(0),
    _wifiConnected(false) {
}

// ============================================
// BEGIN: Koneksi WiFi dan start server
// ============================================
void NetManager::begin() {
  Serial.println(F("\n[WiFi] Connecting..."));
  Serial.print(F("[WiFi] SSID: "));
  Serial.println(WIFI_SSID);
  
  // Mode Station (client)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Tunggu koneksi (timeout 15 detik)
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(F("."));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    _wifiConnected = true;
    Serial.println(F("\n[WiFi] Connected!"));
    Serial.print(F("[WiFi] IP Address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("[WiFi] Dashboard: http://"));
    Serial.print(WiFi.localIP());
    Serial.println(F("/"));
  } else {
    _wifiConnected = false;
    Serial.println(F("\n[WiFi] Connection failed!"));
    Serial.println(F("[WiFi] Game will run offline."));
    Serial.println(F("[WiFi] Score reported via Serial only."));
    return;  // Skip server setup jika WiFi gagal
  }
  
  // === Setup HTTP Routes ===
  
  // Root endpoint: info halaman
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  
  // API: Status game (score, level, dll)
  _server.on("/api/status", HTTP_GET, [this]() { handleApiStatus(); });
  
  // API: Restart game
  _server.on("/api/restart", HTTP_POST, [this]() { handleApiRestart(); });
  
  // Handle CORS preflight (OPTIONS requests)
  _server.on("/api/status", HTTP_OPTIONS, [this]() {
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    _server.send(204);
  });
  
  _server.on("/api/restart", HTTP_OPTIONS, [this]() {
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    _server.send(204);
  });
  
  // 404 handler
  _server.onNotFound([this]() { handleNotFound(); });
  
  // Start server
  _server.begin();
  Serial.println(F("[WiFi] Web server started"));
}

// ============================================
// UPDATE: Handle client requests
// ============================================
void NetManager::update() {
  if (!_wifiConnected) return;
  
  // Cek ulang koneksi WiFi
  if (WiFi.status() != WL_CONNECTED) {
    _wifiConnected = false;
    Serial.println(F("[WiFi] Connection lost!"));
    return;
  }
  
  // Handle incoming HTTP requests
  _server.handleClient();
}

// ============================================
// REPORT SCORE: Kirim score periodik ke Serial
// ============================================
void NetManager::reportScore() {
  // Hanya report setiap SCORE_REPORT_MS
  if (millis() - _lastReport < SCORE_REPORT_MS) return;
  _lastReport = millis();
  
  // Cetak status ke Serial Monitor
  Serial.print(F("[Score] State: "));
  Serial.print(stateToString(_game.getState()));
  Serial.print(F(" | Score: "));
  Serial.print(_game.getScore());
  Serial.print(F(" | High: "));
  Serial.print(_game.getHighScore());
  Serial.print(F(" | Level: "));
  Serial.print(_game.getLevel());
  Serial.print(F(" | Lines: "));
  Serial.println(_game.getLines());
}

// ============================================
// IS CONNECTED: Cek status WiFi
// ============================================
bool NetManager::isConnected() const {
  return _wifiConnected;
}

// ============================================
// GET IP: Dapatkan IP address
// ============================================
String NetManager::getIP() const {
  if (_wifiConnected) {
    return WiFi.localIP().toString();
  }
  return F("Not connected");
}

// ============================================
// HANDLE ROOT: Halaman utama
// ============================================
void NetManager::handleRoot() {
  _server.sendHeader("Access-Control-Allow-Origin", "*");
  
  // Kirim halaman info sederhana
  String html = F(
    "<!DOCTYPE html><html><head>"
    "<meta charset='utf-8'>"
    "<title>IoT Tetris - ESP8266</title>"
    "<style>"
    "body{font-family:monospace;background:#0a0a0a;color:#0f0;"
    "display:flex;justify-content:center;align-items:center;"
    "min-height:100vh;margin:0;}"
    ".box{border:2px solid #0f0;padding:30px;border-radius:8px;"
    "text-align:center;max-width:400px;}"
    "h1{font-size:24px;margin-bottom:20px;}"
    "p{margin:8px 0;font-size:14px;}"
    "a{color:#0ff;}"
    "</style></head><body>"
    "<div class='box'>"
    "<h1>&#127918; IoT TETRIS</h1>"
    "<p>ESP8266 Game Server</p>"
    "<hr style='border-color:#0f0'>"
    "<p>API Endpoint:</p>"
    "<p>GET <a href='/api/status'>/api/status</a></p>"
    "<p>POST /api/restart</p>"
    "<hr style='border-color:#0f0'>"
    "<p>Open dashboard HTML file</p>"
    "<p>and enter this IP address.</p>"
    "</div></body></html>"
  );
  
  _server.send(200, "text/html", html);
}

// ============================================
// HANDLE API STATUS: Return game data as JSON
// ============================================
void NetManager::handleApiStatus() {
  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.sendHeader("Access-Control-Allow-Methods", "GET");
  
  // Bangun JSON response menggunakan ArduinoJson
  StaticJsonDocument<MAX_JSON_SIZE> doc;
  
  doc["device"]    = F("IoT-Tetris");
  doc["status"]    = stateToString(_game.getState());
  doc["score"]     = _game.getScore();
  doc["highScore"] = _game.getHighScore();
  doc["level"]     = _game.getLevel();
  doc["lines"]     = _game.getLines();
  doc["uptime"]    = millis() / 1000;  // Uptime dalam detik
  
  // Serialize ke string
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  _server.send(200, "application/json", jsonStr);
}

// ============================================
// HANDLE API RESTART: Restart game via API
// ============================================
void NetManager::handleApiRestart() {
  _server.sendHeader("Access-Control-Allow-Origin", "*");
  
  // Kirim response sukses
  String response = F("{\"message\":\"Game restart requested\",\"success\":true}");
  _server.send(200, "application/json", response);
  
  Serial.println(F("[WiFi] Restart requested via API"));
  
  // Note: Actual restart dilakukan oleh GameEngine
  // saat detect ACTION_START atau melalui mekanisme lain
}

// ============================================
// HANDLE NOT FOUND: 404 response
// ============================================
void NetManager::handleNotFound() {
  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.send(404, "application/json", 
    F("{\"error\":\"Not Found\",\"code\":404}"));
}

// ============================================
// STATE TO STRING: Konversi enum ke teks
// ============================================
const char* NetManager::stateToString(GameState state) const {
  switch (state) {
    case STATE_IDLE:     return "idle";
    case STATE_PLAYING:  return "playing";
    case STATE_GAMEOVER: return "gameover";
    default:             return "unknown";
  }
}
