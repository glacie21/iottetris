/*
 * ============================================
 * InputHandler.cpp - Implementasi Button Handler
 * ============================================
 * Membaca 5 push button dengan teknik debounce.
 * 
 * Cara kerja debounce:
 * - Tombol dibaca setiap loop cycle
 * - Perubahan state baru diterima jika stabil
 *   selama DEBOUNCE_MS milidetik
 * - Mencegah false trigger dari bouncing mekanis
 * ============================================
 */

#include "InputHandler.h"

// ============================================
// CONSTRUCTOR
// ============================================
InputHandler::InputHandler() {
  // Inisialisasi array tombol dengan pin dan aksi masing-masing
  _buttons[0] = { PIN_BTN_LEFT,   ACTION_LEFT,   true, true, false, 0 };
  _buttons[1] = { PIN_BTN_RIGHT,  ACTION_RIGHT,  true, true, false, 0 };
  _buttons[2] = { PIN_BTN_ROTATE, ACTION_ROTATE, true, true, false, 0 };
  _buttons[3] = { PIN_BTN_DOWN,   ACTION_DOWN,   true, true, false, 0 };
  _buttons[4] = { PIN_BTN_START,  ACTION_START,  true, true, false, 0 };
}

// ============================================
// BEGIN: Setup pin mode
// ============================================
void InputHandler::begin() {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    // D0 (GPIO16) tidak mendukung INPUT_PULLUP
    // Gunakan resistor pull-up eksternal untuk D0
    if (_buttons[i].pin == D0) {
      pinMode(_buttons[i].pin, INPUT);
    } else {
      pinMode(_buttons[i].pin, INPUT_PULLUP);
    }
  }
  
  Serial.println(F("[Input] 5 buttons initialized"));
}

// ============================================
// UPDATE: Baca semua tombol dengan debounce
// ============================================
void InputHandler::update() {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    debounce(_buttons[i]);
  }
}

// ============================================
// GET ACTION: Ambil aksi yang tertrigger
// ============================================
GameAction InputHandler::getAction() {
  // Cek setiap tombol, return aksi pertama yang ditemukan
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    if (_buttons[i].pressed) {
      _buttons[i].pressed = false;  // Reset flag setelah dibaca
      return _buttons[i].action;
    }
  }
  return ACTION_NONE;
}

// ============================================
// IS DOWN HELD: Cek apakah tombol DOWN ditahan
// ============================================
bool InputHandler::isDownHeld() const {
  // currentState = false berarti tombol ditekan (active LOW)
  return !_buttons[3].currentState;
}

// ============================================
// DEBOUNCE: Proses debounce satu tombol
// ============================================
// Algoritma:
// 1. Baca state pin saat ini
// 2. Jika berbeda dari pembacaan terakhir, reset timer
// 3. Jika stabil selama DEBOUNCE_MS, terima perubahan
// 4. Jika berubah dari HIGH ke LOW, set flag pressed
// ============================================
void InputHandler::debounce(ButtonState& btn) {
  bool reading = digitalRead(btn.pin);
  
  // Jika pembacaan berubah, reset timer debounce
  if (reading != btn.lastReading) {
    btn.lastChange = millis();
  }
  btn.lastReading = reading;
  
  // Cek apakah sudah stabil cukup lama
  if ((millis() - btn.lastChange) >= DEBOUNCE_MS) {
    // Jika state berubah setelah debounce
    if (reading != btn.currentState) {
      bool oldState = btn.currentState;
      btn.currentState = reading;
      
      // Deteksi transisi HIGH → LOW (tombol ditekan)
      // Tombol active LOW karena menggunakan pull-up
      if (oldState == true && reading == false) {
        btn.pressed = true;
      }
    }
  }
}
