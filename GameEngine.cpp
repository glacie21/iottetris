/*
 * ============================================
 * GameEngine.cpp - Implementasi Game Engine
 * ============================================
 * Inti logika permainan Tetris:
 * 
 * Game Loop:
 * 1. Baca input → gerakkan/rotasi piece
 * 2. Auto-drop piece berdasarkan timer
 * 3. Cek collision saat bergerak/turun
 * 4. Lock piece jika mendarat
 * 5. Hapus baris penuh dan update score
 * 6. Spawn piece baru
 * 7. Game over jika piece baru collision
 * ============================================
 */

#include "GameEngine.h"

// ============================================
// CONSTRUCTOR
// ============================================
GameEngine::GameEngine(MatrixDisplay& display)
  : _display(display),
    _state(STATE_IDLE),
    _score(0),
    _highScore(0),
    _level(1),
    _linesCleared(0),
    _hasPiece(false),
    _lastDrop(0),
    _lastBlink(0),
    _dropInterval(INITIAL_DROP_MS) {
}

// ============================================
// BEGIN: Inisialisasi game
// ============================================
void GameEngine::begin() {
  resetGame();
  _state = STATE_IDLE;  // Mulai di mode idle, tunggu tombol START
  
  Serial.println(F("[Game] Engine initialized"));
  Serial.println(F("[Game] Press START to begin!"));
}

// ============================================
// UPDATE: Main game loop logic
// ============================================
void GameEngine::update(InputHandler& input) {
  // Baca aksi dari tombol
  GameAction action = input.getAction();
  
  switch (_state) {
    // ------------------------------------------
    // STATE IDLE: Menunggu pemain menekan START
    // ------------------------------------------
    case STATE_IDLE:
      if (action == ACTION_START) {
        resetGame();
        _state = STATE_PLAYING;
        spawnPiece();
        Serial.println(F("[Game] Game started!"));
      }
      break;
    
    // ------------------------------------------
    // STATE PLAYING: Game sedang berjalan
    // ------------------------------------------
    case STATE_PLAYING:
      // Proses input pemain
      switch (action) {
        case ACTION_LEFT:   moveLeft();     break;
        case ACTION_RIGHT:  moveRight();    break;
        case ACTION_ROTATE: rotatePiece();  break;
        case ACTION_DOWN:   dropPiece();    break;
        case ACTION_START:
          // Tombol START saat bermain = restart
          resetGame();
          _state = STATE_PLAYING;
          spawnPiece();
          Serial.println(F("[Game] Game restarted!"));
          break;
        default: break;
      }
      
      // Soft drop: jika tombol DOWN ditahan, percepat
      if (input.isDownHeld()) {
        _dropInterval = MIN_DROP_MS;
      } else {
        // Hitung interval normal berdasarkan level
        _dropInterval = INITIAL_DROP_MS - ((_level - 1) * SPEED_STEP_MS);
        if (_dropInterval < MIN_DROP_MS) {
          _dropInterval = MIN_DROP_MS;
        }
      }
      
      // Auto-drop berdasarkan timer
      if (millis() - _lastDrop >= _dropInterval) {
        if (!dropPiece()) {
          // Piece tidak bisa turun lagi → lock ke board
          lockPiece();
          
          // Cek dan hapus baris penuh
          uint8_t cleared = clearLines();
          if (cleared > 0) {
            addScore(cleared);
          }
          
          // Spawn piece baru
          spawnPiece();
          
          // Cek game over: piece baru langsung collision
          if (checkCollision(_currentPiece)) {
            _state = STATE_GAMEOVER;
            
            // Update high score
            if (_score > _highScore) {
              _highScore = _score;
            }
            
            Serial.println(F("[Game] === GAME OVER ==="));
            Serial.print(F("[Game] Final Score: "));
            Serial.println(_score);
            Serial.print(F("[Game] High Score: "));
            Serial.println(_highScore);
          }
        }
        _lastDrop = millis();
      }
      break;
    
    // ------------------------------------------
    // STATE GAMEOVER: Animasi game over
    // ------------------------------------------
    case STATE_GAMEOVER:
      if (action == ACTION_START) {
        resetGame();
        _state = STATE_PLAYING;
        spawnPiece();
        Serial.println(F("[Game] New game started!"));
      }
      break;
  }
}

// ============================================
// RENDER: Tampilkan ke LED Matrix
// ============================================
void GameEngine::render() {
  switch (_state) {
    case STATE_IDLE:
      // Tampilkan pattern "siap" (checkerboard)
      {
        static unsigned long lastIdle = 0;
        static bool idleState = false;
        if (millis() - lastIdle > 500) {
          idleState = !idleState;
          lastIdle = millis();
        }
        for (uint8_t r = 0; r < BOARD_HEIGHT; r++) {
          // Pattern bergantian: baris genap/ganjil berbeda
          _renderBuffer[r] = idleState ? (r % 2 == 0 ? 0xAA : 0x55)
                                       : (r % 2 == 0 ? 0x55 : 0xAA);
        }
        _display.render(_renderBuffer);
      }
      break;
    
    case STATE_PLAYING:
      // Bangun buffer: board + piece aktif
      buildRenderBuffer();
      _display.render(_renderBuffer);
      break;
    
    case STATE_GAMEOVER:
      // Animasi blink game over
      if (millis() - _lastBlink >= BLINK_MS) {
        _display.blinkAll();
        _lastBlink = millis();
      }
      break;
  }
}

// ============================================
// RESET GAME: Kembalikan ke state awal
// ============================================
void GameEngine::resetGame() {
  // Bersihkan board
  memset(_board, 0, sizeof(_board));
  memset(_renderBuffer, 0, sizeof(_renderBuffer));
  
  // Reset variabel game
  _score = 0;
  _level = 1;
  _linesCleared = 0;
  _hasPiece = false;
  _dropInterval = INITIAL_DROP_MS;
  _lastDrop = millis();
  
  // Bersihkan display
  _display.clear();
  
  Serial.println(F("[Game] Board reset"));
}

// ============================================
// SPAWN PIECE: Buat piece baru
// ============================================
void GameEngine::spawnPiece() {
  _currentPiece = _tetroMgr.spawn();
  _hasPiece = true;
  _lastDrop = millis();  // Reset timer drop
}

// ============================================
// MOVE LEFT: Geser piece ke kiri
// ============================================
void GameEngine::moveLeft() {
  if (!_hasPiece) return;
  
  Tetromino test = _currentPiece;
  test.x--;  // Coba geser kiri
  
  // Hanya geser jika tidak collision
  if (!checkCollision(test)) {
    _currentPiece.x--;
  }
}

// ============================================
// MOVE RIGHT: Geser piece ke kanan
// ============================================
void GameEngine::moveRight() {
  if (!_hasPiece) return;
  
  Tetromino test = _currentPiece;
  test.x++;  // Coba geser kanan
  
  if (!checkCollision(test)) {
    _currentPiece.x++;
  }
}

// ============================================
// ROTATE PIECE: Rotasi 90° searah jarum jam
// ============================================
void GameEngine::rotatePiece() {
  if (!_hasPiece) return;
  
  Tetromino test = _currentPiece;
  test.rotation = (test.rotation + 1) % 4;  // Rotasi 0→1→2→3→0
  
  // Coba rotasi, jika collision coba wall kick
  if (!checkCollision(test)) {
    _currentPiece.rotation = test.rotation;
    return;
  }
  
  // Wall kick: coba geser kiri/kanan jika rotasi di tepi
  test.x = _currentPiece.x - 1;  // Coba geser kiri
  if (!checkCollision(test)) {
    _currentPiece = test;
    return;
  }
  
  test.x = _currentPiece.x + 1;  // Coba geser kanan
  if (!checkCollision(test)) {
    _currentPiece = test;
    return;
  }
  
  // Rotasi tidak bisa dilakukan (piece terlalu mepet)
}

// ============================================
// DROP PIECE: Turunkan piece satu baris
// ============================================
// Return: true jika berhasil turun, false jika mendarat
// ============================================
bool GameEngine::dropPiece() {
  if (!_hasPiece) return false;
  
  Tetromino test = _currentPiece;
  test.y++;  // Coba turun satu baris
  
  if (!checkCollision(test)) {
    _currentPiece.y++;
    return true;   // Berhasil turun
  }
  
  return false;    // Tidak bisa turun (mendarat)
}

// ============================================
// LOCK PIECE: Kunci piece ke board
// ============================================
// Saat piece mendarat, sel-selnya menjadi bagian
// permanen dari board.
// ============================================
void GameEngine::lockPiece() {
  if (!_hasPiece) return;
  
  uint8_t shape[4][4];
  _tetroMgr.getShape(_currentPiece, shape);
  
  // Tulis setiap sel piece ke board
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 4; c++) {
      if (shape[r][c]) {
        int8_t boardRow = _currentPiece.y + r;
        int8_t boardCol = _currentPiece.x + c;
        
        // Pastikan dalam batas board
        if (boardRow >= 0 && boardRow < BOARD_HEIGHT &&
            boardCol >= 0 && boardCol < BOARD_WIDTH) {
          // Set bit pada posisi kolom
          // Bit 7 = kolom 0, Bit 6 = kolom 1, dst.
          _board[boardRow] |= (0x80 >> boardCol);
        }
      }
    }
  }
  
  _hasPiece = false;
}

// ============================================
// CHECK COLLISION: Deteksi tabrakan
// ============================================
// Cek apakah piece pada posisi tertentu bertabrakan
// dengan dinding, lantai, atau blok lain di board.
//
// Return: true jika collision terdeteksi
// ============================================
bool GameEngine::checkCollision(const Tetromino& piece) const {
  uint8_t shape[4][4];
  _tetroMgr.getShape(piece, shape);
  
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 4; c++) {
      if (!shape[r][c]) continue;  // Sel kosong, skip
      
      int8_t boardRow = piece.y + r;
      int8_t boardCol = piece.x + c;
      
      // Cek batas kiri/kanan (dinding)
      if (boardCol < 0 || boardCol >= BOARD_WIDTH) {
        return true;  // Collision dengan dinding
      }
      
      // Cek batas bawah (lantai)
      if (boardRow >= BOARD_HEIGHT) {
        return true;  // Collision dengan lantai
      }
      
      // Sel di atas board (belum muncul) = tidak collision
      if (boardRow < 0) continue;
      
      // Cek collision dengan blok lain di board
      if (_board[boardRow] & (0x80 >> boardCol)) {
        return true;  // Collision dengan blok existing
      }
    }
  }
  
  return false;  // Tidak ada collision
}

// ============================================
// CLEAR LINES: Hapus baris penuh
// ============================================
// Scan dari bawah ke atas, hapus baris yang penuh
// (semua 8 bit = 1, yaitu 0xFF).
// Baris di atas digeser turun.
//
// Return: jumlah baris yang dihapus
// ============================================
uint8_t GameEngine::clearLines() {
  uint8_t cleared = 0;
  
  // Scan dari baris paling bawah ke atas
  for (int8_t row = BOARD_HEIGHT - 1; row >= 0; row--) {
    if (_board[row] == 0xFF) {
      // Baris penuh! Tampilkan animasi
      _display.animateLineClear(row);
      
      // Geser semua baris di atas turun satu
      for (int8_t r = row; r > 0; r--) {
        _board[r] = _board[r - 1];
      }
      _board[0] = 0x00;  // Baris paling atas menjadi kosong
      
      cleared++;
      row++;  // Cek baris yang sama lagi (karena sudah digeser)
    }
  }
  
  return cleared;
}

// ============================================
// ADD SCORE: Tambahkan score berdasarkan lines
// ============================================
// Scoring mengikuti sistem Nintendo:
// 1 line  = 100 poin
// 2 lines = 300 poin
// 3 lines = 500 poin
// 4 lines = 800 poin (Tetris!)
// ============================================
void GameEngine::addScore(uint8_t lines) {
  // Tabel score berdasarkan jumlah baris
  switch (lines) {
    case 1: _score += SCORE_1_LINE;  break;
    case 2: _score += SCORE_2_LINES; break;
    case 3: _score += SCORE_3_LINES; break;
    case 4: _score += SCORE_4_LINES; break;
    default: break;
  }
  
  // Bonus level multiplier
  _score += (lines * _level * 10);
  
  // Update total baris yang dihapus
  _linesCleared += lines;
  
  // Naik level setiap LINES_PER_LEVEL baris
  _level = (_linesCleared / LINES_PER_LEVEL) + 1;
  
  // Log score
  Serial.print(F("[Game] Lines cleared: "));
  Serial.print(lines);
  Serial.print(F(" | Score: "));
  Serial.print(_score);
  Serial.print(F(" | Level: "));
  Serial.println(_level);
}

// ============================================
// BUILD RENDER BUFFER: Gabung board + piece
// ============================================
// Buat buffer tampilan yang menggabungkan:
// 1. Board (blok yang sudah mendarat)
// 2. Piece aktif yang sedang jatuh
// ============================================
void GameEngine::buildRenderBuffer() {
  // Mulai dari board yang ada
  memcpy(_renderBuffer, _board, BOARD_HEIGHT);
  
  // Overlay piece aktif ke buffer
  if (_hasPiece) {
    uint8_t shape[4][4];
    _tetroMgr.getShape(_currentPiece, shape);
    
    for (uint8_t r = 0; r < 4; r++) {
      for (uint8_t c = 0; c < 4; c++) {
        if (!shape[r][c]) continue;
        
        int8_t boardRow = _currentPiece.y + r;
        int8_t boardCol = _currentPiece.x + c;
        
        // Hanya render jika dalam batas board
        if (boardRow >= 0 && boardRow < BOARD_HEIGHT &&
            boardCol >= 0 && boardCol < BOARD_WIDTH) {
          _renderBuffer[boardRow] |= (0x80 >> boardCol);
        }
      }
    }
  }
}
