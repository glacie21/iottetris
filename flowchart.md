# 🔄 Flowchart Game Loop - IoT Tetris

## Main Program Flow

```
                    ┌─────────────┐
                    │    START    │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │   setup()   │
                    │             │
                    │ • Serial    │
                    │ • Display   │
                    │ • Buttons   │
                    │ • Game      │
                    │ • WiFi      │
                    └──────┬──────┘
                           │
              ┌────────────▼────────────┐
              │        loop()           │◄──────────────┐
              │                         │               │
              │  1. net.update()        │               │
              │  2. input.update()      │               │
              │  3. game.update(input)  │               │
              │  4. game.render()       │               │
              │  5. net.reportScore()   │               │
              │  6. yield()             │               │
              └────────────┬────────────┘               │
                           │                            │
                           └────────────────────────────┘
```

## Game State Machine

```
                        ┌───────────────────┐
                        │                   │
            ┌───────────▼──────────┐        │
            │    STATE_IDLE        │        │
            │                      │        │
            │  Tampilan:           │        │
            │  Checkerboard blink  │        │
            │                      │        │
            └───────────┬──────────┘        │
                        │                   │
                  [BTN START]               │
                        │                   │
            ┌───────────▼──────────┐        │
            │   STATE_PLAYING      │        │
            │                      │        │
            │  • Process input     │        │
            │  • Auto-drop piece   │        │
            │  • Check collision   │        │
            │  • Lock & clear lines│        │
            │  • Spawn new piece   │        │
            │                      │        │
            └───┬───────────┬──────┘        │
                │           │               │
          [New piece    [BTN START]         │
           collides]        │               │
                │           └───────────────┘
                │                   (restart)
            ┌───▼──────────────────┐
            │   STATE_GAMEOVER     │
            │                      │
            │  Tampilan:           │
            │  All LEDs blink      │
            │  Update high score   │
            │                      │
            └───────────┬──────────┘
                        │
                  [BTN START]
                        │
                        └───────────────────┘
                                (restart)
```

## Game Update Detail (STATE_PLAYING)

```
┌─────────────────────────────────────────────────────────┐
│                  game.update(input)                      │
└──────────────────────┬──────────────────────────────────┘
                       │
                ┌──────▼──────┐
                │ Read Action │
                └──────┬──────┘
                       │
           ┌───────────┼───────────┐───────────┐
           │           │           │           │
     ┌─────▼────┐ ┌────▼────┐ ┌───▼────┐ ┌───▼────┐
     │  LEFT    │ │ RIGHT   │ │ ROTATE │ │ DOWN   │
     │          │ │         │ │        │ │        │
     │ piece.x--│ │piece.x++│ │rot++ + │ │piece.y++│
     │ if no    │ │if no    │ │wallkick│ │if no    │
     │ collision│ │collision│ │        │ │collision│
     └─────┬────┘ └────┬────┘ └───┬────┘ └───┬────┘
           └───────────┼──────────┘───────────┘
                       │
                ┌──────▼──────────┐
                │  Timer Check    │
                │  millis() -     │
                │  lastDrop >=    │
                │  dropInterval?  │
                └──┬──────────┬───┘
                   │          │
                  YES         NO
                   │          │
            ┌──────▼──────┐   └──── (skip drop)
            │  Try Drop   │
            │  piece.y++  │
            └──┬──────┬───┘
               │      │
          No Collision  Collision!
               │      │
        ┌──────▼──┐ ┌─▼──────────────┐
        │ Piece   │ │ Lock piece     │
        │ moves   │ │ to board       │
        │ down    │ └──────┬─────────┘
        └─────────┘        │
                    ┌──────▼─────────┐
                    │ Check full     │
                    │ lines (0xFF)   │
                    └──┬─────────┬───┘
                       │         │
                    Lines found  No lines
                       │         │
                ┌──────▼──────┐  │
                │ Animate &   │  │
                │ Clear lines │  │
                │ Add score   │  │
                │ Level up?   │  │
                └──────┬──────┘  │
                       │         │
                       └────┬────┘
                            │
                    ┌───────▼────────┐
                    │ Spawn new      │
                    │ piece          │
                    └───┬────────┬───┘
                        │        │
                   No collision  Collision!
                        │        │
                   ┌────▼───┐ ┌──▼──────────┐
                   │Continue│ │ GAME OVER   │
                   │playing │ │ Update high │
                   └────────┘ │ score       │
                              └─────────────┘
```

## Collision Detection Flow

```
  checkCollision(piece)
         │
         ▼
  ┌──────────────────┐
  │ For each cell in │
  │ 4x4 shape grid   │──── cell empty? ──── skip
  └────────┬─────────┘
           │ cell filled
           ▼
  ┌──────────────────┐
  │ Calculate board  │
  │ position:        │
  │ row = piece.y + r│
  │ col = piece.x + c│
  └────────┬─────────┘
           │
     ┌─────┼─────────┐──────────┐
     │     │         │          │
     ▼     ▼         ▼          ▼
  col<0  col>=8   row>=8    board[row]
  or     (right   (floor)   has block
  (left   wall)             at col?
  wall)
     │     │         │          │
     ▼     ▼         ▼          ▼
  COLLISION!      COLLISION!  COLLISION!
  
  Semua check passed → NO COLLISION ✓
```

## Network (WiFi) Flow

```
  ┌─────────────┐
  │  net.begin  │
  └──────┬──────┘
         │
  ┌──────▼──────────┐
  │ WiFi.begin()    │
  │ Wait max 15s    │
  └──┬──────────┬───┘
     │          │
  Connected   Failed
     │          │
     ▼          ▼
  Start      Run offline
  Web Server  (Serial only)
     │
     ▼
  ┌──────────────────┐
  │  Routes:         │
  │  GET /           │──── Info page
  │  GET /api/status │──── JSON game data
  │  POST /api/restart│──── Restart command
  └──────────────────┘
         │
         ▼ (setiap loop)
  ┌──────────────────┐
  │ server.handle    │
  │ Client()         │
  └──────────────────┘
```
