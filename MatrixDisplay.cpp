/*
 * ============================================
 * MatrixDisplay.cpp - Implementasi MAX7219 Display
 * ============================================
 * Menangani semua interaksi dengan hardware
 * LED Matrix MAX7219 melalui library LedControl.
 * ============================================
 */

#include "MatrixDisplay.h"

namespace {
// LedControl mengharapkan bit 0 = kolom paling kiri,
// sedangkan board game menyimpan bit 7 = kolom paling kiri.
// Balik urutan bit agar tampilan tidak terbalik.
uint8_t reverseBits8(uint8_t value) {
  value = ((value & 0xF0) >> 4) | ((value & 0x0F) << 4);
  value = ((value & 0xCC) >> 2) | ((value & 0x33) << 2);
  value = ((value & 0xAA) >> 1) | ((value & 0x55) << 1);
  return value;
}
}

// ============================================
// CONSTRUCTOR
// LedControl(DIN, CLK, CS, numDevices)
// ============================================
MatrixDisplay::MatrixDisplay()
  : _lc(PIN_DIN, PIN_CLK, PIN_CS, NUM_DEVICES),
    _blinkState(false),
    _frame{},
    _frameValid(false) {
}

// ============================================
// BEGIN: Inisialisasi display
// ============================================
void MatrixDisplay::begin() {
  // Loop untuk setiap device yang di-chain
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.shutdown(i, false);          // Nyalakan display (keluar dari sleep)
    _lc.setIntensity(i, BRIGHTNESS); // Set kecerahan
    _lc.clearDisplay(i);             // Bersihkan semua LED
  }
  
  Serial.println(F("[Display] MAX7219 initialized"));
  Serial.print(F("[Display] Devices: "));
  Serial.println(NUM_DEVICES);
}

// ============================================
// CLEAR: Bersihkan seluruh display
// ============================================
void MatrixDisplay::clear() {
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.clearDisplay(i);
  }
  memset(_frame, 0, sizeof(_frame));
  _frameValid = true;
}

// ============================================
// SET PIXEL: Nyalakan/matikan satu LED
// ============================================
void MatrixDisplay::setPixel(uint8_t col, uint8_t row, bool state) {
  // Validasi batas
  if (col >= BOARD_WIDTH || row >= BOARD_HEIGHT) return;
  
  // LedControl: setLed(device, row, col, state)
  // Device 0 untuk single module
  uint8_t device = 0;
  _lc.setLed(device, row, col, state);
  
  // setLed memakai kolom hardware, jadi cache tidak lagi akurat
  _frameValid = false;
}

// ============================================
// SET ROW: Set seluruh baris sekaligus
// ============================================
void MatrixDisplay::setRow(uint8_t row, uint8_t value) {
  if (row >= BOARD_HEIGHT) return;
  
  // LedControl::setRow(device, row, value)
  // library memetakan bit 0 ke kolom paling kiri.
  // Karena board game memakai bit 7 sebagai kolom kiri,
  // kita balik urutan bit sebelum dikirim ke hardware.
  _lc.setRow(0, row, reverseBits8(value));
  _frame[row] = value;
}

// ============================================
// RENDER: Tampilkan board ke LED Matrix
// ============================================
void MatrixDisplay::render(const uint8_t* board) {
  for (uint8_t row = 0; row < BOARD_HEIGHT; row++) {
    // Setiap elemen board[] adalah bitmask satu baris.
    // Karena board game memakai bit 7 = kolom paling kiri,
    // sementara hardware MAX7219 memakai bit 0 = kolom paling kiri,
    // maka perlu di-mirror dulu sebelum dikirim ke display.
    // Lewati baris yang tidak berubah sejak render terakhir.
    if (_frameValid && _frame[row] == board[row]) continue;
    setRow(row, board[row]);
  }
  _frameValid = true;
}

// ============================================
// BLINK ALL: Animasi game over
// ============================================
void MatrixDisplay::blinkAll() {
  _blinkState = !_blinkState;
  
  if (_blinkState) {
    // Nyalakan semua LED
    for (uint8_t row = 0; row < BOARD_HEIGHT; row++) {
      setRow(row, 0xFF);
    }
  } else {
    // Matikan semua LED
    clear();
  }
}

// ============================================
// ANIMATE LINE CLEAR: Efek blink satu baris
// ============================================
void MatrixDisplay::animateLineClear(uint8_t row) {
  if (row >= BOARD_HEIGHT) return;
  
  // Efek: blink baris yang di-clear 3 kali
  for (uint8_t i = 0; i < 3; i++) {
    _lc.setRow(0, row, 0xFF);   // Nyalakan penuh
    delay(80);
    _lc.setRow(0, row, 0x00);   // Matikan
    delay(80);
  }
  
  // Baris ini sekarang mati di hardware
  _frame[row] = 0x00;
}

// ============================================
// SET BRIGHTNESS: Ubah kecerahan (0-15)
// ============================================
void MatrixDisplay::setBrightness(uint8_t level) {
  if (level > 15) level = 15;
  for (int i = 0; i < NUM_DEVICES; i++) {
    _lc.setIntensity(i, level);
  }
}
