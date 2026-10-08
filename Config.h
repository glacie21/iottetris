/*
 * ============================================================
 *  Config.h  –  Konfigurasi Global IoT Tetris
 * ============================================================
 *  File ini adalah SATU-SATUNYA tempat kamu perlu mengubah
 *  pengaturan sebelum meng-upload firmware ke NodeMCU ESP8266.
 *
 *  CARA PENGGUNAAN:
 *  1. Salin Secrets.example.h menjadi Secrets.h, lalu isi
 *     WIFI_SSID, WIFI_PASSWORD, dan API_KEY.
 *  2. Sesuaikan PIN jika wiring kamu berbeda dari default.
 *  3. Optionally, tweak parameter game / timing / scoring.
 *  4. Upload ke board, lalu buka Serial Monitor (115200 baud).
 *
 *  CATATAN PLATFORM:
 *  - Target board  : NodeMCU v2 (ESP8266, 80 MHz)
 *  - Framework     : Arduino (PlatformIO)
 *  - Library utama : LedControl, ArduinoJson
 * ============================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
//  BAGIAN 1 – KONFIGURASI WiFi                    [WAJIB DIISI]
// ============================================================
//  SSID, password WiFi, dan API key disimpan di Secrets.h
//  (tidak ikut di-commit). Salin Secrets.example.h menjadi
//  Secrets.h lalu isi nilainya.
//  - Gunakan jaringan 2.4 GHz (ESP8266 tidak mendukung 5 GHz).
//  - Jika WiFi gagal konek, game tetap berjalan secara offline.
//    Score hanya akan dilaporkan melalui Serial Monitor.
// ============================================================

#if __has_include("Secrets.h")
  #include "Secrets.h"
#else
  #warning "Secrets.h tidak ditemukan - memakai Secrets.example.h (WiFi tidak akan konek)"
  #include "Secrets.example.h"
#endif

//  Port web server bawaan.
//  Ubah jika port 80 sudah terpakai di jaringanmu.
#define WEB_SERVER_PORT  80

//  Durasi timeout percobaan koneksi WiFi (dalam detik).
//  Jika tidak terkoneksi dalam waktu ini, game berjalan offline.
#define WIFI_TIMEOUT_SEC  15

//  Device name yang muncul di JSON API response.
//  Berguna jika kamu menjalankan lebih dari satu unit.
#define DEVICE_NAME      "IoT-Tetris"

//  Origin yang diizinkan mengakses REST API dari browser (CORS).
//  "*" = semua origin (dibutuhkan jika dashboard dibuka dari file
//  lokal). Ganti dengan origin dashboard-mu, mis. "http://192.168.1.10",
//  agar website lain tidak bisa membaca API.
#define CORS_ALLOW_ORIGIN  "*"

//  Proteksi brute force API key: setelah AUTH_MAX_FAILURES percobaan
//  gagal, endpoint terkunci selama AUTH_LOCKOUT_MS milidetik.
#define AUTH_MAX_FAILURES  5
#define AUTH_LOCKOUT_MS    30000

// ============================================================
//  BAGIAN 2 – PIN KONFIGURASI (NodeMCU ESP8266)
// ============================================================
//  Sesuaikan nomor pin jika kabel kamu terhubung berbeda.
//
//  Legenda label pin NodeMCU:
//    D0 = GPIO16,  D1 = GPIO5,  D2 = GPIO4,  D3 = GPIO0
//    D4 = GPIO2,   D5 = GPIO14, D6 = GPIO12, D7 = GPIO13
//
// ┌─────────────────────────────────────────────────────────┐
//  SUB-BAGIAN 2A – MAX7219 LED Matrix (SPI Software / Bit-bang)
//  Hubungkan modul MAX7219 ke pin berikut:
//    NodeMCU D7 → DIN  (Data In)
//    NodeMCU D6 → CS   (Chip Select / LOAD)
//    NodeMCU D5 → CLK  (Clock)
//    NodeMCU 3V3 atau VCC 5V → VCC modul
//    NodeMCU GND → GND modul
// └─────────────────────────────────────────────────────────┘

#define PIN_DIN   D7   // GPIO13 – Data In  ke MAX7219
#define PIN_CS    D6   // GPIO12 – Chip Select (LOAD) ke MAX7219
#define PIN_CLK   D5   // GPIO14 – Clock ke MAX7219

// ┌─────────────────────────────────────────────────────────┐
//  SUB-BAGIAN 2B – Push Button (Active LOW + Internal Pull-Up)
//
//  PERINGATAN BOOT:
//    D3 (GPIO0) dan D4 (GPIO2) HARUS dalam kondisi HIGH
//    (tidak ditekan) saat board dinyalakan / di-reset.
//    Menekan tombol ROTATE atau DOWN saat boot dapat
//    menyebabkan ESP8266 masuk ke mode flash / fail boot.
//
//  Skema wiring tombol (masing-masing):
//    Satu kaki → pin Dx
//    Kaki lain → GND
//    (Tidak perlu resistor eksternal, pull-up internal aktif)
//
//  PENGECUALIAN D0 (GPIO16):
//    GPIO16 TIDAK punya pull-up internal. Tombol START WAJIB
//    diberi resistor pull-up eksternal 10kΩ dari D0 ke 3V3.
//    Tanpa resistor ini pin mengambang dan bisa memicu
//    restart game secara acak.
// └─────────────────────────────────────────────────────────┘

#define PIN_BTN_LEFT    D1   // GPIO5  – Gerak kiri
#define PIN_BTN_RIGHT   D2   // GPIO4  – Gerak kanan
#define PIN_BTN_ROTATE  D3   // GPIO0  – Rotasi piece  [boot-sensitive!]
#define PIN_BTN_DOWN    D4   // GPIO2  – Soft drop     [boot-sensitive!]
#define PIN_BTN_START   D0   // GPIO16 – Start / Reset game [butuh pull-up eksternal!]

// ============================================================
//  BAGIAN 3 – PENGATURAN DISPLAY (MAX7219)
// ============================================================
//  NUM_DEVICES : Berapa modul MAX7219 yang di-chain (daisy-chain).
//               Untuk satu modul 8x8, isi 1.
//               Jika kamu menyambung dua modul, isi 2, dst.
//
//  BRIGHTNESS  : Kecerahan LED, rentang 0 (redup) hingga 15 (terang).
//               Nilai 4-8 direkomendasikan agar tidak terlalu silau
//               dan menghemat daya USB.
// ============================================================

#define NUM_DEVICES   1    // Jumlah modul MAX7219 yang terhubung (1-8)
#define BRIGHTNESS    4    // Kecerahan LED: 0 (min) - 15 (max)

// ============================================================
//  BAGIAN 4 – UKURAN BOARD GAME
// ============================================================
//  Default board 8x8 sesuai dengan satu modul LED Matrix 8x8.
//
//  Jangan ubah nilai ini kecuali kamu menggunakan display
//  custom yang berbeda ukurannya. Nilai harus didukung
//  konfigurasi NUM_DEVICES di atas.
// ============================================================

#define BOARD_WIDTH   8    // Lebar board dalam kolom (default: 8)
#define BOARD_HEIGHT  8    // Tinggi board dalam baris (default: 8)

// ============================================================
//  BAGIAN 5 – TIMING GAME (dalam milidetik)
// ============================================================
//  Semua nilai dalam satuan millisecond (ms).
//
//  INITIAL_DROP_MS  : Kecepatan jatuh saat memulai game.
//                     Nilai lebih besar = lebih lambat (lebih mudah).
//                     Rentang wajar: 500-1200 ms.
//
//  MIN_DROP_MS      : Kecepatan jatuh maksimum (batas kecepatan tertinggi).
//                     Nilai tidak akan turun di bawah ini meski level tinggi.
//                     Rentang wajar: 80-200 ms.
//
//  SPEED_STEP_MS    : Percepatan per kenaikan level.
//                     Drop interval berkurang sebesar ini setiap level baru.
//                     Nilai lebih besar = kenaikan kesulitan lebih cepat.
//
//  DEBOUNCE_MS      : Waktu debounce tombol fisik.
//                     Naikkan jika tombolmu sering "memantul" (double press).
//                     Turunkan jika tombol terasa lambat merespons.
//
//  BLINK_MS         : Interval kedipan animasi game over.
//
//  LINE_CLEAR_MS    : Durasi animasi saat baris penuh dihapus.
//
//  SCORE_REPORT_MS  : Seberapa sering score dicetak ke Serial Monitor
//                     dan tersedia via API WiFi. Default: setiap 2 detik.
// ============================================================

#define INITIAL_DROP_MS    800   // Kecepatan awal jatuh piece (ms)
#define MIN_DROP_MS        150   // Kecepatan jatuh maksimum / batas bawah (ms)
#define SPEED_STEP_MS       50   // Pengurangan drop interval per level naik (ms)
#define DEBOUNCE_MS        120   // Debounce tombol fisik (ms)
#define BLINK_MS           200   // Interval blink animasi game over (ms)
#define LINE_CLEAR_MS      250   // Durasi animasi line clear (ms)
#define SCORE_REPORT_MS   2000   // Interval pelaporan score ke Serial / API (ms)

// ============================================================
//  BAGIAN 6 – SISTEM SCORING
// ============================================================
//  Poin yang diberikan berdasarkan jumlah baris yang dihapus
//  sekaligus dalam satu gerakan (standar Tetris klasik).
//
//  SCORE_DROP      : Bonus poin per satu sel saat soft drop (tombol DOWN).
//                    Set ke 0 untuk menonaktifkan bonus soft drop.
//
//  LINES_PER_LEVEL : Jumlah total baris yang harus dihapus
//                    untuk naik satu level.
//                    Nilai kecil = game lebih cepat meningkat sulit.
// ============================================================

#define SCORE_1_LINE     100   // Hapus 1 baris sekaligus
#define SCORE_2_LINES    300   // Hapus 2 baris sekaligus
#define SCORE_3_LINES    500   // Hapus 3 baris sekaligus
#define SCORE_4_LINES    800   // Hapus 4 baris sekaligus (Tetris!)
#define SCORE_DROP         1   // Bonus poin per sel saat soft drop
#define LINES_PER_LEVEL    3   // Total baris untuk naik 1 level

// ============================================================
//  BAGIAN 7 – KONFIGURASI SERIAL & DEBUG
// ============================================================
//  SERIAL_BAUD  : Baud rate Serial Monitor.
//                 Pastikan Serial Monitor di IDE/PlatformIO
//                 diatur ke nilai yang sama (default: 115200).
//
//  DEBUG_ENABLE : Set ke 1 untuk mengaktifkan pesan debug tambahan
//                 (print state internal, timing, dll.).
//                 Set ke 0 untuk produksi / menghemat flash.
// ============================================================

#define SERIAL_BAUD    115200  // Baud rate Serial (harus sesuai platformio.ini)
#define DEBUG_ENABLE       0   // Debug verbose: 1 = aktif, 0 = nonaktif

// ============================================================
//  BAGIAN 8 – BATAS MEMORI & BUFFER
// ============================================================
//  MAX_JSON_SIZE : Ukuran buffer StaticJsonDocument untuk response API.
//                  256 byte cukup untuk payload status saat ini.
//                  Naikkan jika kamu menambahkan field JSON baru.
//                  ESP8266 punya ~40 KB heap, jadi jangan terlalu besar.
// ============================================================

#define MAX_JSON_SIZE  256   // Ukuran buffer JSON (bytes)

// ============================================================
//  [OPSIONAL] BAGIAN 9 – FITUR TAMBAHAN (dinonaktifkan default)
// ============================================================
//  Uncomment (hapus //) untuk mengaktifkan fitur-fitur berikut:
//
//  #define ENABLE_HOLD_PIECE         // Aktifkan mekanisme "hold" piece
//  #define ENABLE_GHOST_PIECE        // Tampilkan bayangan jatuh piece
//  #define ENABLE_NEXT_PIECE_PREVIEW // Preview piece berikutnya
//
//  Catatan: fitur di atas membutuhkan implementasi tambahan
//  di GameEngine.cpp dan MatrixDisplay.cpp.
// ============================================================

// ============================================================
//  VALIDASI KONFIGURASI – Jangan ubah bagian ini
// ============================================================
//  Pemeriksaan otomatis saat kompilasi untuk memastikan nilai
//  konfigurasi berada dalam rentang yang valid.
// ============================================================

#if BRIGHTNESS < 0 || BRIGHTNESS > 15
  #error "Config.h: BRIGHTNESS harus bernilai antara 0 dan 15."
#endif

#if NUM_DEVICES < 1 || NUM_DEVICES > 8
  #error "Config.h: NUM_DEVICES harus bernilai antara 1 dan 8."
#endif

#if BOARD_WIDTH != 8
  #error "Config.h: BOARD_WIDTH saat ini hanya mendukung nilai 8."
#endif

#if BOARD_HEIGHT != 8
  #error "Config.h: BOARD_HEIGHT saat ini hanya mendukung nilai 8."
#endif

#if MIN_DROP_MS >= INITIAL_DROP_MS
  #error "Config.h: MIN_DROP_MS harus lebih kecil dari INITIAL_DROP_MS."
#endif

#if LINES_PER_LEVEL < 1
  #error "Config.h: LINES_PER_LEVEL harus minimal 1."
#endif

#if MAX_JSON_SIZE < 128
  #error "Config.h: MAX_JSON_SIZE terlalu kecil, minimal 128 bytes."
#endif

#endif // CONFIG_H
