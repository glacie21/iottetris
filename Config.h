/*
 * ============================================
 * Config.h - Konfigurasi Global IoT Tetris
 * ============================================
 * File ini berisi semua konstanta konfigurasi
 * yang digunakan di seluruh project.
 * 
 * Ubah nilai di sini untuk menyesuaikan:
 * - Pin wiring
 * - Pengaturan game
 * - Koneksi WiFi
 * ============================================
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// PIN KONFIGURASI - NodeMCU ESP8266
// ============================================

// MAX7219 LED Matrix (SPI Software)
#define PIN_DIN   D7   // GPIO13 - Data In
#define PIN_CS    D6   // GPIO12 - Chip Select
#define PIN_CLK   D5   // GPIO14 - Clock

// Push Button (active LOW, gunakan internal pull-up)
// Catatan: D3(GPIO0) dan D4(GPIO2) harus HIGH saat boot
//          Jangan tekan tombol ini saat menyalakan board
#define PIN_BTN_LEFT    D1   // GPIO5  - Gerak kiri
#define PIN_BTN_RIGHT   D2   // GPIO4  - Gerak kanan
#define PIN_BTN_ROTATE  D3   // GPIO0  - Rotasi piece
#define PIN_BTN_DOWN    D4   // GPIO2  - Soft drop (turun cepat)
#define PIN_BTN_START   D0   // GPIO16 - Start / Reset game

// ============================================
// PENGATURAN DISPLAY
// ============================================
#define NUM_DEVICES     1    // Jumlah modul MAX7219 (chain)
#define BRIGHTNESS      4    // Kecerahan LED (0-15)

// ============================================
// UKURAN BOARD GAME
// ============================================
#define BOARD_WIDTH     8    // Lebar board (kolom)
#define BOARD_HEIGHT    8    // Tinggi board (baris)

// ============================================
// TIMING GAME (dalam milidetik)
// ============================================
#define INITIAL_DROP_MS    800   // Kecepatan jatuh awal
#define MIN_DROP_MS        150   // Kecepatan jatuh maksimum
#define SPEED_STEP_MS      50    // Percepatan per level
#define DEBOUNCE_MS        120   // Debounce tombol
#define BLINK_MS           200   // Interval blink game over
#define LINE_CLEAR_MS      250   // Durasi animasi line clear
#define SCORE_REPORT_MS    2000  // Interval kirim score via WiFi

// ============================================
// SISTEM SCORING
// ============================================
#define SCORE_1_LINE    100   // Hapus 1 baris
#define SCORE_2_LINES   300   // Hapus 2 baris
#define SCORE_3_LINES   500   // Hapus 3 baris
#define SCORE_4_LINES   800   // Hapus 4 baris (Tetris!)
#define SCORE_DROP      1     // Bonus per cell soft drop
#define LINES_PER_LEVEL 3     // Baris untuk naik level

// ============================================
// KONFIGURASI WIFI
// ============================================
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define WEB_SERVER_PORT 80

// ============================================
// BATAS MEMORY
// ============================================
#define MAX_JSON_SIZE   256   // Ukuran buffer JSON maksimum

#endif // CONFIG_H
