/*
 * ============================================
 * Tetromino.cpp - Implementasi Tetromino Manager
 * ============================================
 * Berisi data bentuk semua tetromino dan
 * logika untuk spawn dan decode bentuk.
 * ============================================
 */

#include "Tetromino.h"

// ============================================
// DATA BENTUK TETROMINO (disimpan di Flash/PROGMEM)
// ============================================
// Setiap piece punya 4 rotasi.
// Setiap rotasi di-encode sebagai 16-bit:
//   Bit 15-12 = Row 0 (atas)
//   Bit 11-8  = Row 1
//   Bit 7-4   = Row 2
//   Bit 3-0   = Row 3 (bawah)
//
// Contoh I-piece rotasi 0:
//   0000 = 0x0
//   1111 = 0xF
//   0000 = 0x0
//   0000 = 0x0
//   → 0x0F00
// ============================================

const uint16_t TetrominoManager::SHAPES[PIECE_COUNT][4] PROGMEM = {
  // PIECE_I: Garis panjang 4 sel
  // R0:  ....   R1:  .X..   R2:  ....   R3:  ..X.
  //      XXXX        .X..        ....        ..X.
  //      ....        .X..        XXXX        ..X.
  //      ....        .X..        ....        ..X.
  { 0x0F00, 0x4444, 0x00F0, 0x2222 },

  // PIECE_O: Kotak 2x2
  // Semua rotasi sama
  //      ....        
  //      .XX.        
  //      .XX.        
  //      ....        
  { 0x0660, 0x0660, 0x0660, 0x0660 },

  // PIECE_T: Bentuk huruf T
  // R0:  ....   R1:  .X..   R2:  ....   R3:  .X..
  //      .X..        .XX.        XXX.        XX..
  //      XXX.        .X..        .X..        .X..
  //      ....        ....        ....        ....
  { 0x04E0, 0x4640, 0x0E40, 0x4C40 },

  // PIECE_S: Bentuk huruf S
  // R0:  ....   R1:  .X..   R2:  ....   R3:  X...
  //      .XX.        .XX.        .XX.        XX..
  //      XX..        ..X.        XX..        .X..
  //      ....        ....        ....        ....
  { 0x06C0, 0x4620, 0x06C0, 0x8C40 },

  // PIECE_Z: Bentuk huruf Z
  // R0:  ....   R1:  ..X.   R2:  ....   R3:  .X..
  //      XX..        .XX.        XX..        XX..
  //      .XX.        .X..        .XX.        X...
  //      ....        ....        ....        ....
  { 0x0C60, 0x2640, 0x0C60, 0x4C80 },

  // PIECE_L: Bentuk huruf L
  // R0:  ....   R1:  .X..   R2:  ....   R3:  XX..
  //      ..X.        .X..        XXX.        .X..
  //      XXX.        .XX.        X...        .X..
  //      ....        ....        ....        ....
  { 0x02E0, 0x4460, 0x0E80, 0xC440 },

  // PIECE_J: Bentuk huruf J
  // R0:  ....   R1:  .XX.   R2:  ....   R3:  .X..
  //      X...        .X..        XXX.        .X..
  //      XXX.        .X..        ..X.        XX..
  //      ....        ....        ....        ....
  { 0x08E0, 0x6440, 0x0E20, 0x44C0 }
};

// ============================================
// CONSTRUCTOR
// ============================================
TetrominoManager::TetrominoManager() {
  // Inisialisasi random seed dari noise analog
  randomSeed(analogRead(A0));
}

// ============================================
// SPAWN: Buat piece baru secara random
// ============================================
Tetromino TetrominoManager::spawn() {
  Tetromino piece;
  
  // Pilih jenis piece secara random
  piece.type = (TetrominoType)random(0, PIECE_COUNT);
  
  // Posisi awal: tengah atas board
  // x = 2 agar piece 4-wide terpusat di board 8-wide
  piece.x = (BOARD_WIDTH / 2) - 2;
  piece.y = -1;  // Mulai sedikit di atas board (muncul dari atas)
  piece.rotation = 0;
  
  return piece;
}

// ============================================
// GET SHAPE: Ambil bentuk piece dari struct
// ============================================
void TetrominoManager::getShape(const Tetromino& piece, uint8_t shape[4][4]) const {
  getShape(piece.type, piece.rotation, shape);
}

// ============================================
// GET SHAPE: Ambil bentuk berdasarkan type & rotation
// ============================================
void TetrominoManager::getShape(TetrominoType type, uint8_t rotation, uint8_t shape[4][4]) const {
  // Baca data dari PROGMEM (flash memory)
  uint16_t encoded = pgm_read_word(&SHAPES[type][rotation % 4]);
  decodeShape(encoded, shape);
}

// ============================================
// DECODE: Konversi 16-bit menjadi array 4x4
// ============================================
void TetrominoManager::decodeShape(uint16_t encoded, uint8_t shape[4][4]) const {
  // Decode setiap bit dari 16-bit value
  // Bit 15 = [0][0], Bit 14 = [0][1], ... Bit 0 = [3][3]
  for (uint8_t row = 0; row < 4; row++) {
    for (uint8_t col = 0; col < 4; col++) {
      // Shift dan mask untuk mendapatkan setiap bit
      uint8_t bitPos = 15 - (row * 4 + col);
      shape[row][col] = (encoded >> bitPos) & 1;
    }
  }
}
