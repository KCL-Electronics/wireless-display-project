#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Hardware ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define BTN_L 13  // left  – turn counterclockwise
#define BTN_R 14  // right – turn clockwise

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Grid ----------
#define CELL 4          // pixels per cell
#define GRID_W (SCREEN_WIDTH  / CELL)  // 32
#define GRID_H (SCREEN_HEIGHT / CELL)  // 16

// ---------- Snake ----------
#define MAX_LEN (GRID_W * GRID_H)  // 512 – absolute max
int segX[MAX_LEN];
int segY[MAX_LEN];
int snakeLen;
int headIdx;  // circular-buffer index of the head

// Direction: 0=up  1=right  2=down  3=left
int dir;
const int dx[] = { 0, 1, 0, -1};
const int dy[] = {-1, 0, 1,  0};

// ---------- Food ----------
int foodX, foodY;

// ---------- Game state ----------
enum State { PLAYING, GAME_OVER };
State state;
int score;

// ---------- Timing ----------
int tickInterval = 120;       // ms per move (starts moderate, speeds up)
unsigned long lastTick = 0;

// ---------- Button edge detection ----------
bool prevL = false;
bool prevR = false;

// ---------- Helpers ----------

// Return the circular-buffer index of segment i (0 = tail, snakeLen-1 = head)
int segIdx(int i) {
  return (headIdx - snakeLen + 1 + i + MAX_LEN) % MAX_LEN;
}

// Check whether grid cell (x,y) is occupied by any snake segment
bool onSnake(int x, int y) {
  for (int i = 0; i < snakeLen; i++) {
    int idx = segIdx(i);
    if (segX[idx] == x && segY[idx] == y) return true;
  }
  return false;
}

void spawnFood() {
  // Pick a random empty cell
  do {
    foodX = random(0, GRID_W);
    foodY = random(0, GRID_H);
  } while (onSnake(foodX, foodY));
}

void resetGame() {
  // Place snake (length 3) roughly in the centre, heading right
  snakeLen = 3;
  headIdx = snakeLen - 1;
  int startX = GRID_W / 2 - 1;
  int startY = GRID_H / 2;
  for (int i = 0; i < snakeLen; i++) {
    segX[i] = startX - (snakeLen - 1 - i);  // tail on the left
    segY[i] = startY;
  }

  dir = 1;  // right
  score = 0;
  tickInterval = 120;
  spawnFood();
  state = PLAYING;
}

// ---------- Arduino entry points ----------

void setup() {
  Wire.begin(8, 9);
  pinMode(BTN_L, INPUT);
  pinMode(BTN_R, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);  // halt if display not found
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  randomSeed(analogRead(0));

  resetGame();
}

void loop() {
  // ---------- GAME OVER state ----------
  if (state == GAME_OVER) {
    // Render game-over screen once, then wait for a press
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print(F("GAME"));
    display.setCursor(10, 30);
    display.print(F("OVER"));
    display.setTextSize(1);
    display.setCursor(80, 14);
    display.print(F("Score"));
    display.setCursor(80, 26);
    display.print(score);
    display.setCursor(70, 50);
    display.print(F("Press any"));
    display.display();

    // Wait until both buttons are released (debounce)
    while (digitalRead(BTN_L) || digitalRead(BTN_R)) { delay(10); }
    // Wait for a new press
    while (!digitalRead(BTN_L) && !digitalRead(BTN_R)) { delay(10); }
    delay(150);  // small debounce

    resetGame();
    return;
  }

  // ---------- INPUT (edge detection – every loop iteration) ----------
  bool curL = digitalRead(BTN_L);
  bool curR = digitalRead(BTN_R);

  // Detect rising edges (transition from not-pressed to pressed)
  bool pressedL = (curL && !prevL);
  bool pressedR = (curR && !prevR);
  prevL = curL;
  prevR = curR;

  // Queue the turn so short taps between ticks aren't lost
  static int pendingTurn = 0;  // -1 = left, +1 = right, 0 = none
  if (pressedL && !pressedR)      pendingTurn = -1;
  else if (pressedR && !pressedL) pendingTurn =  1;

  // ---------- Tick gate ----------
  if (millis() - lastTick < (unsigned long)tickInterval) return;
  lastTick = millis();

  // Apply queued turn
  if (pendingTurn == -1)      dir = (dir + 3) % 4;  // turn left  (CCW)
  else if (pendingTurn == 1)  dir = (dir + 1) % 4;  // turn right (CW)
  pendingTurn = 0;

  // ---------- MOVE ----------
  int hx = segX[headIdx] + dx[dir];
  int hy = segY[headIdx] + dy[dir];

  // Wall collision
  if (hx < 0 || hx >= GRID_W || hy < 0 || hy >= GRID_H) {
    state = GAME_OVER;
    return;
  }

  bool ate = (hx == foodX && hy == foodY);

  if (!ate) {
    // Remove tail (advance past it in the circular buffer → just decrease len logically)
    // Actually we keep headIdx moving forward; tail is implicitly dropped by not growing
  }

  // Advance head in circular buffer
  headIdx = (headIdx + 1) % MAX_LEN;
  segX[headIdx] = hx;
  segY[headIdx] = hy;

  if (ate) {
    snakeLen++;
    score++;
    // Speed up slightly every 5 points, floor at 60 ms
    if (score % 5 == 0 && tickInterval > 60) {
      tickInterval -= 10;
    }
    spawnFood();
  }
  // (if not ate, snakeLen stays the same → oldest segment is effectively dropped)

  // Self collision (check new head against body, excluding head itself)
  for (int i = 0; i < snakeLen - 1; i++) {
    int idx = segIdx(i);
    if (segX[idx] == hx && segY[idx] == hy) {
      state = GAME_OVER;
      return;
    }
  }

  // ---------- RENDER ----------
  display.clearDisplay();

  // Border
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

  // Snake body
  for (int i = 0; i < snakeLen; i++) {
    int idx = segIdx(i);
    display.fillRect(segX[idx] * CELL, segY[idx] * CELL,
                     CELL - 1, CELL - 1, SSD1306_WHITE);
  }

  // Food (blinking-style: small filled rect)
  display.fillRect(foodX * CELL + 1, foodY * CELL + 1,
                   CELL - 2, CELL - 2, SSD1306_WHITE);

  // Score (top-right, inside border)
  display.setCursor(SCREEN_WIDTH - 24, 2);
  display.print(score);

  display.display();
}
