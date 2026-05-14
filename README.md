# 🎮 IoT Tetris - ESP8266 + MAX7219

[![Platform](https://img.shields.io/badge/Platform-ESP8266-blue?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-teal?style=for-the-badge&logo=arduino)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)

> Game Tetris klasik yang berjalan di **NodeMCU ESP8266** dengan tampilan **MAX7219 LED Matrix 8×8**, dilengkapi fitur **IoT** untuk monitoring score secara real-time via web dashboard.

![Game Preview](docs/assets/preview.png)

---

## 📋 Daftar Isi

- [Fitur](#-fitur)
- [Hardware](#-hardware)
- [Wiring Diagram](#-wiring-diagram)
- [Struktur Project](#-struktur-project)
- [Instalasi](#-instalasi)
- [Upload ke ESP8266](#-upload-ke-esp8266)
- [Cara Bermain](#-cara-bermain)
- [Web Dashboard](#-web-dashboard)
- [REST API](#-rest-api)
- [Arsitektur Software](#-arsitektur-software)
- [Optimasi Memory](#-optimasi-memory)
- [Troubleshooting](#-troubleshooting)
- [Lisensi](#-lisensi)

---

## ✨ Fitur

### 🎮 Game
- ✅ 7 jenis Tetromino standard (I, O, T, S, Z, L, J)
- ✅ Gerakan kiri & kanan
- ✅ Rotasi block (clockwise + wall kick)
- ✅ Soft drop (turun cepat)
- ✅ Collision detection (dinding, lantai, blok lain)
- ✅ Line clearing dengan animasi
- ✅ Sistem scoring (Nintendo-style)
- ✅ Level system (semakin tinggi = semakin cepat)
- ✅ Game over detection & animasi
- ✅ Start / Restart game

### 🌐 IoT
- ✅ Koneksi WiFi otomatis
- ✅ REST API server di ESP8266
- ✅ Web dashboard real-time
- ✅ Monitoring: score, high score, level, status
- ✅ Remote restart game via API
- ✅ Graceful degradation (berjalan offline jika WiFi gagal)

---

## 🔧 Hardware

| Komponen | Jumlah | Keterangan |
|----------|--------|------------|
| NodeMCU ESP8266 (ESP-12E) | 1 | Mikrokontroler utama |
| MAX7219 LED Matrix 8×8 | 1 | Display game |
| Push Button | 5 | Input kontrol |
| Resistor 10kΩ | 1 | Pull-up untuk tombol START (D0) |
| Breadboard | 1 | Prototyping |
| Kabel jumper | ±15 | Koneksi komponen |
| Kabel USB Micro | 1 | Upload & power |

---

## 🔌 Wiring Diagram

### MAX7219 → NodeMCU

| MAX7219 Pin | NodeMCU Pin | GPIO |
|-------------|-------------|------|
| VCC | 3V3 atau VIN (5V) | - |
| GND | GND | - |
| DIN | D7 | GPIO13 |
| CS (LOAD) | D6 | GPIO12 |
| CLK | D5 | GPIO14 |

### Push Buttons → NodeMCU

| Tombol | NodeMCU Pin | GPIO | Catatan |
|--------|-------------|------|---------|
| LEFT | D1 | GPIO5 | Internal pull-up |
| RIGHT | D2 | GPIO4 | Internal pull-up |
| ROTATE | D3 | GPIO0 | Internal pull-up ⚠️ |
| DOWN | D4 | GPIO2 | Internal pull-up ⚠️ |
| START | D0 | GPIO16 | **Perlu resistor pull-up eksternal 10kΩ** |

> ⚠️ **Catatan Boot**: Pin D3 (GPIO0) dan D4 (GPIO2) harus HIGH saat boot. Jangan tekan tombol ROTATE atau DOWN saat menyalakan board.

### Skema Tombol

```
         NodeMCU Pin (D1/D2/D3/D4)
              │
              ├──── Push Button ──── GND
              │
        (Internal Pull-up)

         NodeMCU Pin D0 (GPIO16)
              │
              ├──── 10kΩ ──── 3V3  (Pull-up eksternal)
              │
              └──── Push Button ──── GND
```

---

## 📁 Struktur Project

```
IoT_Project/
├── platformio.ini          # Konfigurasi PlatformIO
├── README.md               # Dokumentasi (file ini)
├── src/                    # Source code utama
│   ├── main.cpp            # Entry point program
│   ├── Config.h            # Konstanta konfigurasi
│   ├── Tetromino.h         # Header tetromino manager
│   ├── Tetromino.cpp       # Implementasi tetromino
│   ├── MatrixDisplay.h     # Header display MAX7219
│   ├── MatrixDisplay.cpp   # Implementasi display
│   ├── InputHandler.h      # Header input handler
│   ├── InputHandler.cpp    # Implementasi input
│   ├── GameEngine.h        # Header game engine
│   ├── GameEngine.cpp      # Implementasi game engine
│   ├── NetManager.h        # Header WiFi manager
│   └── NetManager.cpp      # Implementasi WiFi + API
├── dashboard/              # Web dashboard IoT
│   └── index.html          # Dashboard single-page
└── docs/                   # Dokumentasi tambahan
    ├── wiring_diagram.md   # Detail wiring
    ├── flowchart.md        # Flowchart game loop
    └── modules.md          # Penjelasan modul
```

---

## 🚀 Instalasi

### Opsi 1: PlatformIO (Direkomendasikan)

1. **Install VS Code** + Extension **PlatformIO IDE**
2. **Clone / Download** project ini
3. **Buka folder** project di VS Code
4. PlatformIO akan otomatis install dependencies
5. Edit `src/Config.h` → isi **SSID** dan **Password** WiFi

### Opsi 2: Arduino IDE

1. **Install Arduino IDE** versi 1.8+ atau 2.x
2. **Tambahkan Board Manager ESP8266**:
   - File → Preferences → Additional Board URLs:
   - `https://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. **Install Board**: Tools → Board Manager → cari "ESP8266" → Install
4. **Install Libraries** (Sketch → Include Library → Manage Libraries):
   - `LedControl` by Eberhard Fahle
   - `ArduinoJson` by Benoit Blanchon (v6.x)
5. **Pilih Board**: Tools → Board → NodeMCU 1.0 (ESP-12E Module)
6. Salin semua file dari `src/` ke folder sketch

---

## 📤 Upload ke ESP8266

### Langkah Upload

1. **Hubungkan** NodeMCU ke PC via kabel USB Micro
2. **Pastikan driver** CH340/CP2102 terinstall
3. **Edit konfigurasi WiFi** di `src/Config.h`:
   ```cpp
   #define WIFI_SSID       "NamaWiFiAnda"
   #define WIFI_PASSWORD   "PasswordWiFi"
   ```
4. **Upload**:
   - PlatformIO: Klik tombol → (Upload) di status bar
   - Arduino IDE: Sketch → Upload (Ctrl+U)
5. **Buka Serial Monitor** (115200 baud) untuk melihat log
6. **Catat IP address** yang ditampilkan di Serial Monitor

### Pengaturan Upload Arduino IDE

| Setting | Nilai |
|---------|-------|
| Board | NodeMCU 1.0 (ESP-12E Module) |
| Upload Speed | 921600 |
| CPU Frequency | 80 MHz |
| Flash Size | 4MB (FS:2MB OTA:~1019KB) |
| Port | COM port yang sesuai |

---

## 🕹️ Cara Bermain

| Tombol | Fungsi |
|--------|--------|
| **LEFT** (D1) | Geser piece ke kiri |
| **RIGHT** (D2) | Geser piece ke kanan |
| **ROTATE** (D3) | Putar piece 90° |
| **DOWN** (D4) | Soft drop (turun cepat) |
| **START** (D0) | Mulai / Restart game |

### Alur Permainan

1. Nyalakan board → tampil animasi idle (checkerboard)
2. Tekan **START** → game dimulai
3. Piece muncul dari atas, jatuh otomatis
4. Gunakan tombol untuk mengatur posisi piece
5. Isi baris penuh → baris terhapus + dapat score
6. Game over saat piece baru tidak bisa muncul
7. Tekan **START** untuk main lagi

### Sistem Score

| Aksi | Poin |
|------|------|
| 1 baris | 100 + (level × 10) |
| 2 baris | 300 + (level × 20) |
| 3 baris | 500 + (level × 30) |
| 4 baris (Tetris!) | 800 + (level × 40) |

---

## 🌐 Web Dashboard

Dashboard IoT untuk monitoring game secara real-time.

### Cara Menggunakan

1. Buka file `dashboard/index.html` di browser
2. Masukkan **IP address** ESP8266 (lihat Serial Monitor)
3. Klik **Connect**
4. Dashboard akan menampilkan data real-time:
   - Current Score
   - High Score
   - Level & Lines
   - Game Status
   - Uptime

### Fitur Dashboard
- 🎨 Desain cyberpunk dengan animasi
- 📊 Update data setiap 1 detik
- 🔄 Tombol restart game dari browser
- 📡 Log JSON response dari API
- 📱 Responsive design (mobile-friendly)

---

## 📡 REST API

ESP8266 menyediakan REST API berikut:

### `GET /api/status`

Mendapatkan status game saat ini.

**Response:**
```json
{
  "device": "IoT-Tetris",
  "status": "playing",
  "score": 1500,
  "highScore": 3200,
  "level": 3,
  "lines": 8,
  "uptime": 245
}
```

| Field | Tipe | Keterangan |
|-------|------|------------|
| `device` | string | Nama device |
| `status` | string | `idle` / `playing` / `gameover` |
| `score` | number | Score saat ini |
| `highScore` | number | Score tertinggi |
| `level` | number | Level saat ini |
| `lines` | number | Total baris yang dihapus |
| `uptime` | number | Waktu aktif (detik) |

### `POST /api/restart`

Mengirim perintah restart game.

**Response:**
```json
{
  "message": "Game restart requested",
  "success": true
}
```

### Contoh Penggunaan (cURL)

```bash
# Ambil status game
curl http://192.168.1.100/api/status

# Restart game
curl -X POST http://192.168.1.100/api/restart
```

---

## 🏗️ Arsitektur Software

```
┌─────────────────────────────────────────────┐
│                  main.cpp                    │
│            (Orchestrator Loop)               │
├─────────┬──────────┬──────────┬─────────────┤
│         │          │          │             │
│  Input  │  Game    │ Matrix   │   Net       │
│ Handler │  Engine  │ Display  │  Manager    │
│         │          │          │             │
│ 5 Btn   │ Tetris   │ MAX7219  │ WiFi+API   │
│ Debounce│ Logic    │ LedCtrl  │ REST       │
│         │          │          │             │
│    D1   │ Collision│   D5     │ ESP8266    │
│    D2   │ Scoring  │   D6     │ WebServer  │
│    D3   │ States   │   D7     │ ArduinoJson│
│    D4   │          │          │             │
│    D0   │ Tetromino│          │             │
│         │ Manager  │          │             │
└─────────┴──────────┴──────────┴─────────────┘
```

### Penjelasan Modul

| Modul | File | Fungsi |
|-------|------|--------|
| **Config** | `Config.h` | Semua konstanta dan pin definitions |
| **TetrominoManager** | `Tetromino.h/cpp` | Definisi bentuk 7 piece, spawn random, decode shape |
| **MatrixDisplay** | `MatrixDisplay.h/cpp` | Kontrol MAX7219 via LedControl, render board |
| **InputHandler** | `InputHandler.h/cpp` | Baca 5 tombol dengan debounce, map ke game action |
| **GameEngine** | `GameEngine.h/cpp` | Inti game: movement, collision, line clear, scoring |
| **NetManager** | `NetManager.h/cpp` | WiFi connection, REST API server, score reporting |

---

## 💾 Optimasi Memory

ESP8266 hanya memiliki ~80KB RAM. Berikut optimasi yang diterapkan:

| Teknik | Penjelasan |
|--------|------------|
| **PROGMEM** | Data tetromino shapes disimpan di Flash (bukan RAM) |
| **Bitmask board** | Board 8×8 hanya menggunakan 8 byte (1 byte per baris) |
| **16-bit shape encoding** | Setiap rotasi piece hanya 2 byte (vs 16 byte array) |
| **`F()` macro** | String literal disimpan di Flash |
| **StaticJsonDocument** | JSON buffer di stack, bukan heap |
| **`uint8_t`** | Tipe data terkecil yang cukup |
| **Non-blocking loop** | Tidak ada `delay()` di game loop utama |
| **`yield()`** | Mencegah watchdog timer reset |

### Estimasi Penggunaan Memory

| Komponen | RAM (bytes) |
|----------|-------------|
| Board array | 8 |
| Render buffer | 8 |
| Tetromino data | ~10 |
| Button states | ~60 |
| JSON buffer | 256 |
| WiFi stack | ~8,000 |
| **Total estimasi** | **~10 KB** |

---

## ❓ Troubleshooting

| Masalah | Solusi |
|---------|--------|
| LED tidak menyala | Cek wiring DIN/CLK/CS, cek power supply |
| Tampilan terbalik | Sesuaikan orientasi di `MatrixDisplay.cpp` |
| Tombol tidak responsif | Cek kabel, cek pull-up resistor untuk D0 |
| WiFi gagal connect | Cek SSID/password di Config.h, jarak ke router |
| Board restart terus | Jangan tekan D3/D4 saat boot, cek power supply |
| Dashboard tidak konek | Pastikan PC dan ESP8266 di jaringan WiFi yang sama |
| Upload gagal | Install driver CH340, coba tekan FLASH saat upload |

---

## 📜 Lisensi

Project ini dilisensikan di bawah [MIT License](LICENSE).

Dibuat untuk pembelajaran IoT & Embedded Systems. 🎓

---

<p align="center">
  <b>⭐ Jika project ini bermanfaat, berikan star di GitHub! ⭐</b>
</p>
