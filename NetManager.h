/*
 * ============================================
 * NetManager.h - WiFi & Web Server Header
 * ============================================
 * Mengelola koneksi WiFi ESP8266 dan
 * menyediakan REST API untuk dashboard IoT.
 * 
 * Endpoints:
 *   GET  /api/status  → Data game (JSON)
 *   POST /api/restart → Restart game
 *   GET  /           → Redirect ke dashboard info
 * ============================================
 */

#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "GameEngine.h"

class NetManager {
public:
  /**
   * Constructor: terima referensi ke GameEngine
   * untuk membaca data score dan status.
   */
  NetManager(GameEngine& game);

  /** Inisialisasi WiFi dan web server */
  void begin();

  /** Handle client requests (panggil di loop) */
  void update();

  /** Kirim score update ke Serial (debug) */
  void reportScore();

  /** Cek apakah WiFi terkoneksi */
  bool isConnected() const;

  /** Dapatkan IP address */
  String getIP() const;

private:
  GameEngine& _game;                // Referensi ke game engine
  ESP8266WebServer _server;          // Web server instance
  unsigned long _lastReport;         // Waktu report terakhir
  bool _wifiConnected;               // Status koneksi WiFi

  // === Handler untuk HTTP endpoints ===
  void handleRoot();                 // GET /
  void handleApiStatus();            // GET /api/status
  void handleApiRestart();           // POST /api/restart
  void handleNotFound();             // 404

  /** Konversi GameState ke string */
  const char* stateToString(GameState state) const;
};

#endif // NET_MANAGER_H
