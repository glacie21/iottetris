/*
 * ============================================
 * GameEngine.h - Game Engine Header
 * ============================================
 * Inti logika game Tetris:
 * - Game state management
 * - Collision detection
 * - Line clearing
 * - Scoring system
 * - Game loop timing
 * ============================================
 */

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <Arduino.h>
#include "Config.h"
#include "Tetromino.h"
#include "MatrixDisplay.h"
#include "InputHandler.h"

// ============================================
// ENUM: State permainan
// ============================================
enum GameState : uint8_t {
  STATE_IDLE,       // Menunggu start
  STATE_PLAYING,    // Sedang bermain
  STATE_GAMEOVER    // Game over, menunggu restart
};

// ============================================
// CLASS: GameEngine
// ============================================
class GameEngine {
public:
  /**
   * Constructor: terima referensi display.
   * @param display Referensi ke MatrixDisplay
   */
  GameEngine(MatrixDisplay& display);

  /** Inisialisasi game engine */
  void begin();

  /**
   * Update logika game berdasarkan input.
   * @param input Referensi ke InputHandler
   */
  void update(InputHandler& input);

  /** Render state game ke display */
  void render();

  /**
   * Minta restart game dari luar (mis. REST API).
   * Diproses pada pemanggilan update() berikutnya.
   */
  void requestRestart() { _restartRequested = true; }

  // === Getter untuk data game (digunakan oleh NetManager) ===
  GameState getState() const     { return _state; }
  uint32_t  getScore() const     { return _score; }
  uint32_t  getHighScore() const { return _highScore; }
  uint8_t   getLevel() const     { return _level; }
  uint16_t  getLines() const     { return _linesCleared; }

private:
  // === Referensi ke modul lain ===
  MatrixDisplay& _display;
  TetrominoManager _tetroMgr;

  // === State game ===
  GameState _state;
  uint32_t  _score;
  uint32_t  _highScore;
  uint8_t   _level;
  uint16_t  _linesCleared;     // Total baris yang dihapus

  // === Board: array bitmask per baris ===
  // Bit 7 = kolom 0 (kiri), Bit 0 = kolom 7 (kanan)
  uint8_t _board[BOARD_HEIGHT];

  // === Piece aktif ===
  Tetromino _currentPiece;
  bool _hasPiece;              // Ada piece aktif?
  bool _restartRequested;      // Restart diminta via API?

  // === Timing ===
  unsigned long _lastDrop;     // Waktu drop terakhir
  unsigned long _lastBlink;    // Waktu blink terakhir (game over)
  uint16_t _dropInterval;      // Interval drop saat ini (ms)

  // === Render buffer ===
  // Gabungan board + piece aktif untuk ditampilkan
  uint8_t _renderBuffer[BOARD_HEIGHT];

  // === Metode internal ===

  /** Reset game ke state awal */
  void resetGame();

  /** Reset board lalu langsung mulai bermain */
  void startNewGame();

  /** Pindah ke state game over dan update high score */
  void endGame();

  /** Tambah poin dengan saturasi (tidak overflow) */
  void addPoints(uint32_t points);

  /** Hitung interval drop normal berdasarkan level */
  uint16_t levelDropInterval() const;

  /** Spawn piece baru di atas board */
  void spawnPiece();

  /** Gerakkan piece ke kiri */
  void moveLeft();

  /** Gerakkan piece ke kanan */
  void moveRight();

  /** Rotasi piece searah jarum jam */
  void rotatePiece();

  /** Turunkan piece satu baris (gravity/manual) */
  bool dropPiece();

  /**
   * Kunci piece ke board (saat mendarat).
   * @return false jika ada sel piece di atas board (top-out)
   */
  bool lockPiece();

  /**
   * Cek collision untuk piece pada posisi tertentu.
   * @param piece Piece yang dicek
   * @return true jika terjadi collision
   */
  bool checkCollision(const Tetromino& piece) const;

  /**
   * Cek dan hapus baris penuh.
   * @return Jumlah baris yang dihapus
   */
  uint8_t clearLines();

  /**
   * Hitung score berdasarkan jumlah baris.
   * @param lines Jumlah baris yang dihapus
   */
  void addScore(uint8_t lines);

  /** Bangun render buffer (board + piece aktif) */
  void buildRenderBuffer();
};

#endif // GAME_ENGINE_H
