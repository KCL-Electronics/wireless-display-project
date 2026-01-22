#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define BTN_UP   13
#define BTN_DOWN 14

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Paddle
#define PADDLE_W 3
#define PADDLE_H 16
int playerY = (SCREEN_HEIGHT - PADDLE_H) / 2;
int aiY = playerY;

// Ball
int ballX = SCREEN_WIDTH / 2;
int ballY = SCREEN_HEIGHT / 2;
int ballVX = -2;
int ballVY = 1;

// Score
int score = 0;

// Timing
unsigned long lastFrame = 0;
const int frameDelay = 33; // ~30 FPS

void resetBall() {
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  ballVX = (random(0, 2) == 0) ? -2 : 2;
  ballVY = random(-2, 3);
}

void setup() {
  Wire.begin(8, 9);

  pinMode(BTN_UP, INPUT);     // external pulldown
  pinMode(BTN_DOWN, INPUT);  // external pulldown

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  resetBall();
}

void loop() {
  if (millis() - lastFrame < frameDelay) return;
  lastFrame = millis();

  // ---- INPUT ----
  if (digitalRead(BTN_UP) && playerY > 0) {
    playerY -= 3;
  }
  if (digitalRead(BTN_DOWN) && playerY < SCREEN_HEIGHT - PADDLE_H) {
    playerY += 3;
  }

  // ---- AI ----
  if (ballY > aiY + PADDLE_H / 2) aiY += 2;
  if (ballY < aiY + PADDLE_H / 2) aiY -= 2;
  aiY = constrain(aiY, 0, SCREEN_HEIGHT - PADDLE_H);

  // ---- BALL UPDATE ----
  ballX += ballVX;
  ballY += ballVY;

  // Top / bottom collision
  if (ballY <= 0 || ballY >= SCREEN_HEIGHT - 2) {
    ballVY = -ballVY;
  }

  // Player paddle collision
  if (ballX <= PADDLE_W &&
      ballY >= playerY &&
      ballY <= playerY + PADDLE_H) {
    ballVX = -ballVX;
  }

  // AI paddle collision
  if (ballX >= SCREEN_WIDTH - PADDLE_W - 2 &&
      ballY >= aiY &&
      ballY <= aiY + PADDLE_H) {
    ballVX = -ballVX;
  }

  // Missed ball
  if (ballX < 0) {
    score = 0;
    resetBall();
  }

  if (ballX > SCREEN_WIDTH) {
    score++;
    resetBall();
  }

  // ---- RENDER ----
  display.clearDisplay();

  // Center dashed line
  for (int y = 0; y < SCREEN_HEIGHT; y += 4) {
    display.drawPixel(SCREEN_WIDTH / 2, y, SSD1306_WHITE);
  }

  // Paddles
  display.fillRect(0, playerY, PADDLE_W, PADDLE_H, SSD1306_WHITE);
  display.fillRect(SCREEN_WIDTH - PADDLE_W, aiY, PADDLE_W, PADDLE_H, SSD1306_WHITE);

  // Ball
  display.fillRect(ballX, ballY, 2, 2, SSD1306_WHITE);

  // Score
  display.setCursor(56, 0);
  display.print(score);

  display.display();
}
