# 📦 Penjelasan Modul - IoT Tetris

## Daftar Modul

| # | Modul | File | Tanggung Jawab |
|---|-------|------|----------------|
| 1 | Config | `Config.h` | Konstanta global |
| 2 | TetrominoManager | `Tetromino.h/cpp` | Bentuk & rotasi piece |
| 3 | MatrixDisplay | `MatrixDisplay.h/cpp` | Kontrol LED Matrix |
| 4 | InputHandler | `InputHandler.h/cpp` | Baca tombol |
| 5 | GameEngine | `GameEngine.h/cpp` | Logika inti game |
| 6 | NetManager | `NetManager.h/cpp` | WiFi & REST API |
| 7 | Main | `main.cpp` | Entry point & orchestrator |

---

## 1. Config (`Config.h`)

**Fungsi**: Menyimpan semua konstanta konfigurasi di satu tempat.

**Mengapa terpisah?** Agar perubahan setting (pin, timing, WiFi) tidak perlu mengedit banyak file. Cukup ubah di `Config.h`.

**Isi utama:**
- Pin definitions (MAX7219 & buttons)
- Game timing (drop speed, debounce)
- Scoring values
- WiFi credentials
- Memory limits

---

## 2. TetrominoManager (`Tetromino.h/cpp`)

**Fungsi**: Mengelola 7 jenis tetromino dan 4 rotasi masing-masing.

**Teknik utama:**
- **16-bit encoding**: Setiap rotasi piece disimpan sebagai 1 `uint16_t` (2 byte). Grid 4×4 = 16 sel = 16 bit. Total data: 7 pieces × 4 rotasi × 2 byte = **56 byte saja**.
- **PROGMEM**: Data disimpan di Flash memory (bukan RAM) karena bersifat konstan.
- **`pgm_read_word()`**: Membaca data dari Flash.

**Encoding format:**
```
Bit position dalam uint16_t:
┌────┬────┬────┬────┐
│ 15 │ 14 │ 13 │ 12 │  Row 0
├────┼────┼────┼────┤
│ 11 │ 10 │  9 │  8 │  Row 1
├────┼────┼────┼────┤
│  7 │  6 │  5 │  4 │  Row 2
├────┼────┼────┼────┤
│  3 │  2 │  1 │  0 │  Row 3
└────┴────┴────┴────┘

Contoh I-piece horizontal:
0000 = 0x0
1111 = 0xF
0000 = 0x0
0000 = 0x0
→ 0x0F00
```

---

## 3. MatrixDisplay (`MatrixDisplay.h/cpp`)

**Fungsi**: Abstraksi hardware LED Matrix MAX7219.

**Library**: Menggunakan `LedControl` oleh Eberhard Fahle.

**Metode utama:**
| Metode | Fungsi |
|--------|--------|
| `begin()` | Inisialisasi MAX7219 (keluar dari sleep, set brightness) |
| `clear()` | Matikan semua LED |
| `setPixel(col, row, state)` | Nyalakan/matikan 1 LED |
| `setRow(row, value)` | Set 1 baris sekaligus (bitmask) |
| `render(board)` | Render seluruh board ke display |
| `blinkAll()` | Animasi game over |
| `animateLineClear(row)` | Animasi blink saat baris dihapus |

**Representasi baris:**
```
Bit 7 = Kolom 0 (kiri)
Bit 6 = Kolom 1
...
Bit 0 = Kolom 7 (kanan)

Contoh: 0b11001010
→ Kolom 0,1,3,5 menyala
```

---

## 4. InputHandler (`InputHandler.h/cpp`)

**Fungsi**: Membaca 5 push button dengan debounce software.

**Konsep debounce:**
Push button mekanis menghasilkan "bouncing" (sinyal naik-turun cepat saat ditekan). Debounce menunggu sinyal stabil selama `DEBOUNCE_MS` sebelum menerima perubahan.

```
Sinyal tanpa debounce:     Sinyal dengan debounce:
  ─┐ ┌┐ ┌┐ ┌──              ─────┐
   │ ││ ││ │                      │
   └─┘└─┘└─┘                     └──────
   (bouncing)               (clean press)
```

**Edge detection**: Hanya mendeteksi transisi HIGH→LOW (saat tombol ditekan), bukan saat ditahan. Kecuali tombol DOWN yang mendukung "hold" untuk soft drop.

**Khusus D0 (GPIO16):**
GPIO16 tidak mendukung `INPUT_PULLUP` internal, sehingga menggunakan `INPUT` biasa dan perlu resistor pull-up eksternal 10kΩ.

---

## 5. GameEngine (`GameEngine.h/cpp`)

**Fungsi**: Inti logika permainan Tetris.

### State Machine

Game memiliki 3 state:

| State | Tampilan | Aksi Tersedia |
|-------|----------|---------------|
| `IDLE` | Checkerboard animasi | START → mulai game |
| `PLAYING` | Board + piece aktif | LEFT, RIGHT, ROTATE, DOWN, START(restart) |
| `GAMEOVER` | Semua LED blink | START → new game |

### Board Representation

Board disimpan sebagai array `uint8_t[8]`:
```cpp
uint8_t _board[BOARD_HEIGHT]; // 8 byte total!
```
Setiap elemen = 1 baris, setiap bit = 1 kolom.

### Collision Detection

Saat piece bergerak/turun, dicek apakah:
1. Keluar batas kiri/kanan (dinding)
2. Keluar batas bawah (lantai)
3. Overlap dengan blok yang sudah ada di board

Jika ada collision, gerakan dibatalkan.

### Wall Kick

Saat rotasi gagal karena collision, dicoba geser 1 pixel ke kiri, lalu ke kanan. Jika keduanya gagal, rotasi dibatalkan.

### Line Clearing

Scan dari bawah ke atas. Baris penuh = `0xFF` (semua 8 bit = 1). Saat ditemukan:
1. Animasi blink pada baris tersebut
2. Geser semua baris di atas turun 1
3. Baris atas menjadi kosong (0x00)

### Scoring

Mengikuti sistem Nintendo Tetris:
```
1 line  → 100 + (level × 10)
2 lines → 300 + (level × 20)
3 lines → 500 + (level × 30)
4 lines → 800 + (level × 40)  ← "TETRIS!"
```

### Render Buffer

Board dan piece aktif digabung ke `_renderBuffer[]` sebelum dikirim ke display:
```
renderBuffer = board OR piece
```
Menggunakan operasi bitwise OR agar piece terlihat di atas board.

---

## 6. NetManager (`NetManager.h/cpp`)

**Fungsi**: Koneksi WiFi dan REST API server.

### WiFi Connection
- Mode: Station (STA) - terhubung ke router
- Timeout: 15 detik
- Graceful degradation: game tetap berjalan jika WiFi gagal

### REST API Endpoints

| Method | Path | Response |
|--------|------|----------|
| GET | `/` | Halaman info HTML |
| GET | `/api/status` | JSON status game |
| POST | `/api/restart` | Restart game |

### CORS Support
Semua endpoint mendukung Cross-Origin Resource Sharing (CORS) agar dashboard HTML yang dibuka di browser bisa mengakses API.

### JSON Response
Menggunakan ArduinoJson v6 dengan `StaticJsonDocument<256>` untuk efisiensi memory (alokasi di stack, bukan heap).

---

## 7. Main (`main.cpp`)

**Fungsi**: Entry point dan orchestrator.

### `setup()`
Inisialisasi semua modul secara berurutan:
1. Serial (debugging)
2. Display (MAX7219)
3. Input (buttons)
4. Game (engine)
5. Network (WiFi + server)

### `loop()`
Non-blocking game loop:
1. `net.update()` - Handle HTTP requests
2. `input.update()` - Baca tombol + debounce
3. `game.update(input)` - Logika game
4. `game.render()` - Tampilkan ke LED
5. `net.reportScore()` - Kirim score periodik
6. `yield()` - **Penting untuk ESP8266!**

### Mengapa `yield()`?
ESP8266 memiliki watchdog timer dan WiFi stack yang perlu diproses secara periodik. `yield()` memberikan waktu untuk proses internal ini. Tanpa `yield()`, board bisa restart karena watchdog timeout.

---

## Contoh Payload JSON

### Status Game (GET /api/status)

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

### Restart Game (POST /api/restart)

**Request**: Tidak perlu body

**Response**:
```json
{
  "message": "Game restart requested",
  "success": true
}
```

### Game Over

```json
{
  "device": "IoT-Tetris",
  "status": "gameover",
  "score": 2800,
  "highScore": 3200,
  "level": 5,
  "lines": 14,
  "uptime": 523
}
```

### Idle (Belum Mulai)

```json
{
  "device": "IoT-Tetris",
  "status": "idle",
  "score": 0,
  "highScore": 3200,
  "level": 1,
  "lines": 0,
  "uptime": 10
}
```
