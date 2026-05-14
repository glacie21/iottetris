/*
 * ============================================
 * InputHandler.h - Button Input Handler Header
 * ============================================
 * Mengelola 5 push button dengan debounce.
 * Mendeteksi press event (bukan hold).
 * ============================================
 */

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "Config.h"

// ============================================
// ENUM: Aksi yang tersedia dari tombol
// ============================================
enum GameAction : uint8_t {
  ACTION_NONE    = 0,
  ACTION_LEFT    = 1,   // Gerak kiri
  ACTION_RIGHT   = 2,   // Gerak kanan
  ACTION_ROTATE  = 3,   // Rotasi piece
  ACTION_DOWN    = 4,   // Soft drop
  ACTION_START   = 5    // Start / Reset game
};

// ============================================
// CLASS: InputHandler
// ============================================
class InputHandler {
public:
  InputHandler();

  /** Inisialisasi pin tombol dengan pull-up */
  void begin();

  /**
   * Update state tombol (panggil di setiap loop).
   * Membaca semua tombol dengan debounce.
   */
  void update();

  /**
   * Cek apakah ada aksi yang tertrigger.
   * @return GameAction yang terdeteksi, atau ACTION_NONE
   */
  GameAction getAction();

  /**
   * Cek apakah tombol DOWN sedang ditahan.
   * Digunakan untuk soft drop continuous.
   */
  bool isDownHeld() const;

private:
  // Jumlah tombol yang dikelola
  static const uint8_t NUM_BUTTONS = 5;

  // Data per tombol
  struct ButtonState {
    uint8_t pin;              // GPIO pin
    GameAction action;        // Aksi yang di-map
    bool lastReading;         // Pembacaan terakhir
    bool currentState;        // State setelah debounce
    bool pressed;             // Flag: baru saja ditekan
    unsigned long lastChange; // Waktu perubahan terakhir
  };

  ButtonState _buttons[NUM_BUTTONS];

  // Proses debounce untuk satu tombol
  void debounce(ButtonState& btn);
};

#endif // INPUT_HANDLER_H
