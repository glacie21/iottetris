/*
 * ============================================
 * Tetromino.h - Tetromino Manager Header
 * ============================================
 * Mengelola bentuk-bentuk tetromino (7 jenis):
 * I, O, T, S, Z, L, J
 * 
 * Setiap piece memiliki 4 rotasi yang disimpan
 * sebagai 16-bit bitmask untuk hemat memory.
 * ============================================
 */

#ifndef TETROMINO_H
#define TETROMINO_H

#include <Arduino.h>
#include "Config.h"

// ============================================
// ENUM: 7 Jenis Tetromino Standard
// ============================================
enum TetrominoType : uint8_t {
  PIECE_I = 0,   // ████  (garis panjang)
  PIECE_O = 1,   // ██    (kotak)
                  // ██
  PIECE_T = 2,   //  █    (huruf T)
                  // ███
  PIECE_S = 3,   //  ██   (huruf S)
                  // ██
  PIECE_Z = 4,   // ██    (huruf Z)
                  //  ██
  PIECE_L = 5,   //   █   (huruf L)
                  // ███
  PIECE_J = 6,   // █     (huruf J)
                  // ███
  PIECE_COUNT = 7
};

// ============================================
// STRUCT: Data satu piece di board
// ============================================
struct Tetromino {
  TetrominoType type;     // Jenis piece
  int8_t x;               // Posisi kolom (bisa negatif di tepi)
  int8_t y;               // Posisi baris (0 = atas)
  uint8_t rotation;       // State rotasi (0-3)
};

// ============================================
// CLASS: TetrominoManager
// ============================================
class TetrominoManager {
public:
  TetrominoManager();

  /**
   * Spawn piece baru secara random di atas board.
   * Posisi awal: tengah atas board.
   */
  Tetromino spawn();

  /**
   * Ambil bentuk piece sebagai array 4x4.
   * shape[row][col] = 1 jika terisi, 0 jika kosong.
   */
  void getShape(const Tetromino& piece, uint8_t shape[4][4]) const;
  void getShape(TetrominoType type, uint8_t rotation, uint8_t shape[4][4]) const;

private:
  /**
   * Data bentuk disimpan di PROGMEM (flash memory).
   * Format: 16-bit per rotasi, 4 rotasi per piece.
   * Bit layout (4x4 grid):
   *   Row 0: bits 15-12
   *   Row 1: bits 11-8
   *   Row 2: bits 7-4
   *   Row 3: bits 3-0
   */
  static const uint16_t SHAPES[PIECE_COUNT][4] PROGMEM;

  // Decode 16-bit menjadi array 4x4
  void decodeShape(uint16_t encoded, uint8_t shape[4][4]) const;
};

#endif // TETROMINO_H
