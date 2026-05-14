/*
 * ============================================
 * main.cpp - Entry Point IoT Tetris
 * ============================================
 * 
 * Project : IoT Tetris Game
 * Board   : NodeMCU ESP8266 (ESP-12E)
 * Display : MAX7219 LED Matrix 8x8
 * Input   : 5 Push Buttons
 * IoT     : REST API via WiFi
 * 
 * ============================================
 * Alur Program:
 * 
 * setup()
 *   ├─ Inisialisasi Serial Monitor
 *   ├─ Inisialisasi Display (MAX7219)
 *   ├─ Inisialisasi Input (5 tombol)
 *   ├─ Inisialisasi Game Engine
 *   └─ Inisialisasi WiFi + Web Server
 * 
 * loop()
 *   ├─ NetManager.update()    → Handle HTTP requests
 *   ├─ InputHandler.update()  → Baca tombol + debounce
 *   ├─ GameEngine.update()    → Logika game + input
 *   ├─ GameEngine.render()    → Tampilkan ke LED Matrix
 *   └─ NetManager.reportScore() → Kirim score periodik
 * 
 * ============================================
 * 
 * Catatan Memory ESP8266:
 * - Flash: 4MB (cukup untuk program)
 * - RAM: ~80KB (perlu hemat!)
 * - PROGMEM digunakan untuk data konstan
 * - Hindari String dinamis berlebihan
 * 
 * ============================================
 */

#include <Arduino.h>
#include "Config.h"
#include "GameEngine.h"
#include "MatrixDisplay.h"
#include "InputHandler.h"
#include "NetManager.h"

// ============================================
// INSTANCE GLOBAL MODULES
// ============================================

// Display: mengontrol LED Matrix MAX7219
MatrixDisplay display;

// Input: membaca 5 push button dengan debounce
InputHandler input;

// Game: inti logika Tetris (perlu referensi display)
GameEngine game(display);

// Network: WiFi + REST API (perlu referensi game)
NetManager net(game);

// ============================================
// SETUP: Inisialisasi satu kali saat boot
// ============================================
void setup() {
  // --- Serial Monitor untuk debugging ---
  Serial.begin(115200);
  delay(100);
  
  Serial.println(F("\n"));
  Serial.println(F("╔══════════════════════════════╗"));
  Serial.println(F("║     IoT TETRIS - ESP8266     ║"));
  Serial.println(F("║   MAX7219 LED Matrix 8x8     ║"));
  Serial.println(F("╚══════════════════════════════╝"));
  Serial.println();
  
  // --- Inisialisasi setiap modul ---
  Serial.println(F("--- Initializing modules ---"));
  
  display.begin();   // MAX7219 display
  input.begin();     // Push buttons
  game.begin();      // Game engine
  net.begin();       // WiFi + web server
  
  Serial.println(F("\n--- System Ready ---"));
  Serial.println(F("Press START button to play!"));
  
  // Tampilkan info memory yang tersedia
  Serial.print(F("[System] Free heap: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes"));
}

// ============================================
// LOOP: Main loop - berjalan terus menerus
// ============================================
void loop() {
  // 1. Handle WiFi dan HTTP requests
  //    (non-blocking, hanya proses jika ada request)
  net.update();
  
  // 2. Baca input dari semua tombol
  //    (debounce diproses di sini)
  input.update();
  
  // 3. Update logika game berdasarkan input
  //    (movement, rotation, drop, collision, scoring)
  game.update(input);
  
  // 4. Render state game ke LED Matrix
  //    (board + piece aktif → display)
  game.render();
  
  // 5. Kirim score report (periodik, non-blocking)
  net.reportScore();
  
  // yield() penting untuk ESP8266!
  // Memberi waktu untuk WiFi stack dan watchdog
  yield();
}
