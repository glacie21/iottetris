/*
 * ============================================
 * MatrixDisplay.h - MAX7219 Display Header
 * ============================================
 * Mengontrol LED Matrix MAX7219 8x8.
 * Menggunakan library LedControl.
 * ============================================
 */

#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include <Arduino.h>
#include <LedControl.h>
#include "Config.h"

class MatrixDisplay {
public:
  MatrixDisplay();

  /** Inisialisasi display MAX7219 */
  void begin();

  /** Bersihkan seluruh display */
  void clear();

  /**
   * Set satu pixel pada posisi (col, row).
   * @param col  Kolom (0-7, kiri ke kanan)
   * @param row  Baris (0-7, atas ke bawah)
   * @param state true = nyala, false = mati
   */
  void setPixel(uint8_t col, uint8_t row, bool state);

  /**
   * Set seluruh baris sekaligus menggunakan bitmask.
   * @param row   Baris (0-7)
   * @param value Bitmask 8-bit (bit 7 = kolom 0, bit 0 = kolom 7)
   */
  void setRow(uint8_t row, uint8_t value);

  /**
   * Render buffer board ke display.
   * @param board Array bitmask per baris [BOARD_HEIGHT]
   */
  void render(const uint8_t* board);

  /** Animasi blink untuk game over */
  void blinkAll();

  /** Animasi line clear: blink baris tertentu */
  void animateLineClear(uint8_t row);

  /** Set kecerahan display (0-15) */
  void setBrightness(uint8_t level);

private:
  LedControl _lc;        // Instance library LedControl
  bool _blinkState;      // State untuk animasi blink
};

#endif // MATRIX_DISPLAY_H
