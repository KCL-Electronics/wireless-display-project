#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// Buttons
#define BTN_L 13
#define BTN_R 14

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Tetris Constants
#define GRID_W 10
#define GRID_H 20
#define BLOCK_SIZE 3 // Smaller blocks to fit 64px height
#define OFFSET_X 48  // Center the grid on the 128px wide screen

bool grid[GRID_W][GRID_H] = {0};
int curX, curY, curType, curRot;
unsigned long lastDrop = 0;
int dropInterval = 800;
int score = 0;

// Tetromino Definitions (Simplified 4x4)
const uint16_t shapes[7][4] = {
  {0x4444, 0x0F00, 0x4444, 0x0F00}, // I
  {0x4460, 0x0E80, 0xC440, 0x2E00}, // L
  {0x44C0, 0x8E00, 0x6440, 0x0E20}, // J
  {0x0660, 0x0660, 0x0660, 0x0660}, // O
  {0x06C0, 0x8C40, 0x06C0, 0x8C40}, // S
  {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
  {0x0C60, 0x4C80, 0x0C60, 0x4C80}  // Z
};

bool checkCollision(int nx, int ny, int nr) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (bitRead(shapes[curType][nr], 15 - (i * 4 + j))) {
        int gx = nx + j;
        int gy = ny + i;
        if (gx < 0 || gx >= GRID_W || gy >= GRID_H || (gy >= 0 && grid[gx][gy])) return true;
      }
    }
  }
  return false;
}

void spawn() {
  curX = GRID_W / 2 - 2; curY = 0;
  curType = random(7); curRot = 0;
  if (checkCollision(curX, curY, curRot)) {
    memset(grid, 0, sizeof(grid)); // Game Over - Clear board
    score = 0;
  }
}

void lockPiece() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (bitRead(shapes[curType][curRot], 15 - (i * 4 + j))) {
        if (curY + i >= 0) grid[curX + j][curY + i] = 1;
      }
    }
  }
  // Clear Lines
  for (int i = GRID_H - 1; i >= 0; i--) {
    bool full = true;
    for (int j = 0; j < GRID_W; j++) if (!grid[j][i]) full = false;
    if (full) {
      score += 10;
      for (int k = i; k > 0; k--) 
        for (int j = 0; j < GRID_W; j++) grid[j][k] = grid[j][k - 1];
      i++; 
    }
  }
  spawn();
}

void setup() {
  Wire.begin(8, 9);
  pinMode(BTN_L, INPUT); pinMode(BTN_R, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  randomSeed(analogRead(0));
  spawn();
}

void loop() {
  bool l = digitalRead(BTN_L);
  bool r = digitalRead(BTN_R);

  // INPUT LOGIC
  if (l && r) { // Pull Down
    while (!checkCollision(curX, curY + 1, curRot)) curY++;
    lockPiece();
    delay(200);
  } else if (l) {
    if (!checkCollision(curX - 1, curY, curRot)) curX--;
    delay(150);
  } else if (r) {
    if (!checkCollision(curX + 1, curY, curRot)) curX++;
    delay(150);
  }

  // Auto Gravity
  if (millis() - lastDrop > dropInterval) {
    if (!checkCollision(curX, curY + 1, curRot)) curY++;
    else lockPiece();
    lastDrop = millis();
  }

  // RENDER
  display.clearDisplay();
  display.drawRect(OFFSET_X - 1, 0, (GRID_W * BLOCK_SIZE) + 2, (GRID_H * BLOCK_SIZE) + 1, SSD1306_WHITE);
  
  // Draw Locked Blocks
  for (int x = 0; x < GRID_W; x++)
    for (int y = 0; y < GRID_H; y++)
      if (grid[x][y]) display.fillRect(OFFSET_X + x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE - 1, BLOCK_SIZE - 1, SSD1306_WHITE);

  // Draw Current Piece
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      if (bitRead(shapes[curType][curRot], 15 - (i * 4 + j)))
        display.fillRect(OFFSET_X + (curX + j) * BLOCK_SIZE, (curY + i) * BLOCK_SIZE, BLOCK_SIZE - 1, BLOCK_SIZE - 1, SSD1306_WHITE);

  display.setCursor(0, 0); display.print("Score:"); display.print(score);
  display.display();
}
