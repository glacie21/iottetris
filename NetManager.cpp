/*
 * ============================================
 * NetManager.cpp - Implementasi WiFi & Web Server
 * ============================================
 * Fitur:
 * 1. Koneksi WiFi otomatis dengan timeout & auto-reconnect
 * 2. REST API server untuk dashboard
 * 3. CORS support untuk akses dari browser
 * 4. Endpoint restart dilindungi API key + lockout brute force
 * 5. Periodic score reporting via Serial
 *
 * JSON Response Format:
 * {
 *   "device": "IoT-Tetris",
 *   "status": "playing",
 *   "score": 1500,
 *   "highScore": 3200,
 *   "level": 3,
 *   "lines": 8,
 *   "uptime": 120
 * }
 * ============================================
 */

#include "NetManager.h"

// API key yang terlalu pendek mudah ditebak
static_assert(sizeof(API_KEY) == 1 || sizeof(API_KEY) > 16,
              "Secrets.h: API_KEY harus kosong atau minimal 16 karakter.");

// Nama header yang dibaca dari request
static const char HEADER_API_KEY[] = "X-API-Key";

// Halaman info disimpan di flash agar tidak memakan heap
static const char ROOT_HTML[] PROGMEM =
  "<!DOCTYPE html><html><head>"
  "<meta charset='utf-8'>"
  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
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
  "<p>POST /api/restart (header X-API-Key)</p>"
  "<hr style='border-color:#0f0'>"
  "<p>Open dashboard HTML file</p>"
  "<p>and enter this IP address.</p>"
  "</div></body></html>";

// ============================================
// HELPER: Perbandingan string constant-time
// ============================================
// Waktu eksekusi tidak bergantung pada posisi karakter
// pertama yang berbeda, sehingga API key tidak bisa
// ditebak lewat timing attack.
// ============================================
static bool constantTimeEquals(const String& input, const char* secret) {
  size_t secretLen = strlen(secret);
  size_t inputLen = input.length();
  uint8_t diff = (inputLen == secretLen) ? 0 : 1;
  for (size_t i = 0; i < secretLen; i++) {
    char c = (i < inputLen) ? input[i] : 0;
    diff |= (uint8_t)(c ^ secret[i]);
  }
  return diff == 0;
}

// ============================================
// CONSTRUCTOR
// ============================================
NetManager::NetManager(GameEngine& game)
  : _game(game),
    _server(WEB_SERVER_PORT),
    _lastReport(0),
    _wifiConnected(false),
    _authFailures(0),
    _lockoutStart(0) {
}

// ============================================
// BEGIN: Koneksi WiFi dan start server
// ============================================
void NetManager::begin() {
  Serial.println(F("\n[WiFi] Connecting..."));
  Serial.print(F("[WiFi] SSID: "));
  Serial.println(WIFI_SSID);

  // Mode Station (client). persistent(false) mencegah
  // kredensial ditulis ulang ke flash setiap boot.
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Tunggu koneksi sampai WIFI_TIMEOUT_SEC detik
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < (unsigned long)WIFI_TIMEOUT_SEC * 1000UL) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  _wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (_wifiConnected) {
    printConnectionInfo();
  } else {
    Serial.println(F("[WiFi] Connection failed!"));
    Serial.println(F("[WiFi] Game will run offline, retrying in background."));
  }

  // === Setup HTTP Routes ===
  // Server tetap disiapkan walau WiFi gagal, supaya langsung
  // bisa dipakai saat auto-reconnect berhasil.

  // Root endpoint: info halaman
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });

  // API: Status game (score, level, dll)
  _server.on("/api/status", HTTP_GET, [this]() { handleApiStatus(); });

  // API: Restart game (butuh API key)
  _server.on("/api/restart", HTTP_POST, [this]() { handleApiRestart(); });

  // Handle CORS preflight (OPTIONS requests)
  _server.on("/api/status", HTTP_OPTIONS, [this]() { handleOptions("GET, OPTIONS"); });
  _server.on("/api/restart", HTTP_OPTIONS, [this]() { handleOptions("POST, OPTIONS"); });

  // 404 handler
  _server.onNotFound([this]() { handleNotFound(); });

  // Header yang perlu dibaca oleh server
  _server.collectHeaders(HEADER_API_KEY);

  // Start server
  _server.begin();
  Serial.println(F("[WiFi] Web server started"));

  if (sizeof(API_KEY) == 1) {
    Serial.println(F("[WiFi] API_KEY kosong: POST /api/restart dinonaktifkan"));
  }
}

// ============================================
// UPDATE: Handle client requests
// ============================================
void NetManager::update() {
  bool connected = (WiFi.status() == WL_CONNECTED);

  // Deteksi perubahan status koneksi
  if (connected != _wifiConnected) {
    _wifiConnected = connected;
    if (connected) {
      Serial.println(F("[WiFi] Reconnected!"));
      printConnectionInfo();
    } else {
      Serial.println(F("[WiFi] Connection lost! Reconnecting..."));
    }
  }

  if (!_wifiConnected) return;

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
// SEND COMMON HEADERS: CORS + keamanan
// ============================================
void NetManager::sendCommonHeaders() {
  _server.sendHeader(F("Access-Control-Allow-Origin"), F(CORS_ALLOW_ORIGIN));
  _server.sendHeader(F("X-Content-Type-Options"), F("nosniff"));
  _server.sendHeader(F("Cache-Control"), F("no-store"));
}

// ============================================
// SEND ERROR: Response error dalam format JSON
// ============================================
void NetManager::sendError(int code, const __FlashStringHelper* message) {
  StaticJsonDocument<96> doc;
  doc["error"] = message;
  doc["code"]  = code;

  String body;
  serializeJson(doc, body);
  _server.send(code, F("application/json"), body);
}

// ============================================
// HANDLE OPTIONS: CORS preflight
// ============================================
void NetManager::handleOptions(const char* methods) {
  sendCommonHeaders();
  _server.sendHeader(F("Access-Control-Allow-Methods"), methods);
  _server.sendHeader(F("Access-Control-Allow-Headers"), F("Content-Type, X-API-Key"));
  _server.sendHeader(F("Access-Control-Max-Age"), F("600"));
  _server.send(204);
}

// ============================================
// AUTHORIZE: Validasi API key
// ============================================
bool NetManager::authorize() {
  // Endpoint dinonaktifkan jika API_KEY tidak diisi
  if (sizeof(API_KEY) == 1) {
    sendError(403, F("Endpoint disabled: set API_KEY in Secrets.h"));
    return false;
  }

  // Masih dalam masa lockout setelah terlalu banyak percobaan gagal
  if (_authFailures >= AUTH_MAX_FAILURES) {
    if (millis() - _lockoutStart < AUTH_LOCKOUT_MS) {
      _server.sendHeader(F("Retry-After"), String(AUTH_LOCKOUT_MS / 1000));
      sendError(429, F("Too many failed attempts"));
      return false;
    }
    _authFailures = 0;  // Lockout selesai
  }

  if (!constantTimeEquals(_server.header(HEADER_API_KEY), API_KEY)) {
    _authFailures++;
    if (_authFailures >= AUTH_MAX_FAILURES) {
      _lockoutStart = millis();
      Serial.println(F("[WiFi] Too many invalid API keys, locking restart endpoint"));
    }
    sendError(401, F("Unauthorized"));
    return false;
  }

  _authFailures = 0;
  return true;
}

// ============================================
// HANDLE ROOT: Halaman utama
// ============================================
void NetManager::handleRoot() {
  sendCommonHeaders();
  _server.send_P(200, "text/html", ROOT_HTML);
}

// ============================================
// HANDLE API STATUS: Return game data as JSON
// ============================================
void NetManager::handleApiStatus() {
  sendCommonHeaders();

  // Bangun JSON response menggunakan ArduinoJson
  StaticJsonDocument<MAX_JSON_SIZE> doc;

  doc["device"]    = DEVICE_NAME;
  doc["status"]    = stateToString(_game.getState());
  doc["score"]     = _game.getScore();
  doc["highScore"] = _game.getHighScore();
  doc["level"]     = _game.getLevel();
  doc["lines"]     = _game.getLines();
  doc["uptime"]    = millis() / 1000;  // Uptime dalam detik

  // Serialize ke string
  String jsonStr;
  serializeJson(doc, jsonStr);

  _server.send(200, F("application/json"), jsonStr);
}

// ============================================
// HANDLE API RESTART: Restart game via API
// ============================================
void NetManager::handleApiRestart() {
  sendCommonHeaders();
  if (!authorize()) return;

  // Restart dieksekusi oleh GameEngine pada update() berikutnya
  _game.requestRestart();

  _server.send(200, F("application/json"),
    F("{\"message\":\"Game restart requested\",\"success\":true}"));

  Serial.println(F("[WiFi] Restart requested via API"));
}

// ============================================
// HANDLE NOT FOUND: 404 response
// ============================================
void NetManager::handleNotFound() {
  sendCommonHeaders();
  sendError(404, F("Not Found"));
}

// ============================================
// PRINT CONNECTION INFO: Info WiFi ke Serial
// ============================================
void NetManager::printConnectionInfo() const {
  Serial.println(F("[WiFi] Connected!"));
  Serial.print(F("[WiFi] IP Address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("[WiFi] Dashboard: http://"));
  Serial.print(WiFi.localIP());
  Serial.println(F("/"));
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
