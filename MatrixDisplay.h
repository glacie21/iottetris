/*
 * ============================================
 * MatrixDisplay.h - MAX7219 Display Header
 * ============================================
 * Mengontrol LED Matrix MAX7219 8x8
 * menggunakan library LedControl.
 * ============================================
 */

#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include <Arduino.h>
#include <LedControl.h>

#include "Config.h"

class MatrixDisplay {
public:
    /* =========================================
     * Constructor
     * ========================================= */
    MatrixDisplay();

    /* =========================================
     * Inisialisasi display MAX7219
     * ========================================= */
    void begin();

    /* =========================================
     * Bersihkan seluruh display
     * ========================================= */
    void clear();

    /* =========================================
     * Set satu pixel pada posisi tertentu
     *
     * @param col   Kolom (0-7, kiri ke kanan)
     * @param row   Baris (0-7, atas ke bawah)
     * @param state true = nyala, false = mati
     * ========================================= */
    void setPixel(uint8_t col, uint8_t row, bool state);

    /* =========================================
     * Set seluruh baris menggunakan bitmask
     *
     * @param row   Baris (0-7)
     * @param value Bitmask 8-bit
     *              (bit 7 = kolom 0,
     *               bit 0 = kolom 7)
     * ========================================= */
    void setRow(uint8_t row, uint8_t value);

    /* =========================================
     * Render buffer board ke display
     *
     * @param board Array bitmask per baris
     *              [BOARD_HEIGHT]
     * ========================================= */
    void render(const uint8_t* board);

    /* =========================================
     * Animasi blink untuk game over
     * ========================================= */
    void blinkAll();

    /* =========================================
     * Animasi line clear
     *
     * @param row Baris yang akan diblink
     * ========================================= */
    void animateLineClear(uint8_t row);

    /* =========================================
     * Set tingkat kecerahan display
     *
     * @param level Brightness (0-15)
     * ========================================= */
    void setBrightness(uint8_t level);

private:
    /* =========================================
     * Attributes
     * ========================================= */
    LedControl _lc;      // Instance LedControl
    bool _blinkState;    // State animasi blink
};

#endif // MATRIX_DISPLAY_H
