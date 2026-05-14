# 🔌 Wiring Diagram - IoT Tetris

## Skema Koneksi Lengkap

```
  ┌──────────────────────────────────────────────────────┐
  │                   NodeMCU ESP8266                     │
  │                                                      │
  │  3V3 ●──────────────────────────────── VCC (MAX7219) │
  │  GND ●──────────────────────────────── GND (MAX7219) │
  │                                                      │
  │   D7 ●──── GPIO13 ─── DIN  (MAX7219 Data In)        │
  │   D6 ●──── GPIO12 ─── CS   (MAX7219 Chip Select)    │
  │   D5 ●──── GPIO14 ─── CLK  (MAX7219 Clock)          │
  │                                                      │
  │   D1 ●──── GPIO5  ─── BTN_LEFT ──── GND             │
  │   D2 ●──── GPIO4  ─── BTN_RIGHT ─── GND             │
  │   D3 ●──── GPIO0  ─── BTN_ROTATE ── GND             │
  │   D4 ●──── GPIO2  ─── BTN_DOWN ──── GND             │
  │   D0 ●──── GPIO16 ─┬─ BTN_START ─── GND             │
  │                     │                                 │
  │  3V3 ●──── 10kΩ ───┘  (Pull-up eksternal)           │
  │                                                      │
  └──────────────────────────────────────────────────────┘
```

## Detail Koneksi MAX7219

```
  NodeMCU              MAX7219 Module
  ───────              ──────────────
                       ┌────────────────────┐
  D7 (GPIO13) ────────►│ DIN    ┌──────┐   │
  D5 (GPIO14) ────────►│ CLK    │8x8   │   │
  D6 (GPIO12) ────────►│ CS     │LED   │   │
  3V3 / VIN ──────────►│ VCC    │Matrix│   │
  GND ────────────────►│ GND    └──────┘   │
                       └────────────────────┘
```

> 💡 **Catatan Power**: MAX7219 lebih baik diberi power 5V (dari pin VIN NodeMCU). Jika menggunakan 3V3, kecerahan bisa berkurang.

## Detail Koneksi Push Button

### Tombol dengan Internal Pull-up (D1, D2, D3, D4)

```
  NodeMCU Pin ────┬──── Push Button ──── GND
                  │
           (Internal Pull-up ke 3V3)
           (Tidak perlu resistor eksternal)
```

**Prinsip kerja:**
- Tidak ditekan → Pin HIGH (3V3 dari pull-up)
- Ditekan → Pin LOW (terhubung ke GND)

### Tombol START (D0/GPIO16) - Perlu Pull-up Eksternal

```
  3V3 ──── 10kΩ ────┬──── Push Button ──── GND
                     │
                NodeMCU D0
```

> ⚠️ **GPIO16 tidak mendukung internal pull-up**, sehingga perlu resistor pull-up eksternal 10kΩ ke 3V3.

## Layout Breadboard (Saran)

```
  ┌─────────────────────────────────────────────────────┐
  │                    BREADBOARD                        │
  │                                                      │
  │   [MAX7219]     [NodeMCU ESP8266]     [Buttons]      │
  │   ┌──────┐      ┌──────────────┐                     │
  │   │ LED  │      │              │     [L] [R] [↻]     │
  │   │ 8x8  │◄────►│   ESP8266    │                     │
  │   │      │      │              │     [↓] [▶]         │
  │   └──────┘      └──────────────┘                     │
  │                                                      │
  │   ── Power Rail (+) ──────────────────────────────   │
  │   ── Ground Rail (-) ──────────────────────────────  │
  └─────────────────────────────────────────────────────┘
  
  L = LEFT    R = RIGHT    ↻ = ROTATE
  ↓ = DOWN    ▶ = START/RESET
```

## Tabel Pin NodeMCU ESP8266

| Label | GPIO | Fungsi di Project | Catatan |
|-------|------|-------------------|---------|
| D0 | GPIO16 | BTN_START | Tidak ada interrupt, perlu pull-up eksternal |
| D1 | GPIO5 | BTN_LEFT | Aman digunakan |
| D2 | GPIO4 | BTN_RIGHT | Aman digunakan |
| D3 | GPIO0 | BTN_ROTATE | Harus HIGH saat boot |
| D4 | GPIO2 | BTN_DOWN | Harus HIGH saat boot, ada built-in LED |
| D5 | GPIO14 | MAX7219 CLK | SPI Clock |
| D6 | GPIO12 | MAX7219 CS | Chip Select |
| D7 | GPIO13 | MAX7219 DIN | SPI MOSI |
| D8 | GPIO15 | *Tidak digunakan* | Harus LOW saat boot |
| A0 | ADC0 | *Tidak digunakan* | Analog input |

## Catatan Penting

1. **Power Supply**: Pastikan power supply cukup. MAX7219 dengan semua LED menyala bisa menarik ~300mA. USB biasanya cukup untuk 1 modul.

2. **Ground Bersama**: Semua GND harus terhubung (common ground antara NodeMCU, MAX7219, dan tombol).

3. **Kabel Pendek**: Untuk koneksi SPI (DIN, CLK, CS), gunakan kabel sependek mungkin untuk mengurangi noise.

4. **Boot Mode**: Saat upload firmware, pastikan tidak ada tombol yang ditekan (khususnya D3 dan D4).
