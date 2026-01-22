#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define BTN_LEFT 13
#define BTN_RIGHT 14
#define BTN_JUMP 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Player
#define PLAYER_W 5
#define PLAYER_H 8
int playerX = SCREEN_WIDTH / 2 - PLAYER_W/2;
int playerY = SCREEN_HEIGHT - PLAYER_H;
bool jumping = false;
int jumpY = 0;
int jumpVel = 0;

// Obstacles
#define MAX_OBS 5
struct Obstacle {
  int x, y;
  int w, h;
  bool active;
};
Obstacle obstacles[MAX_OBS];

// Game
int score = 0;
unsigned long lastFrame = 0;
const int frameDelay = 50; // ~20 FPS

void spawnObstacle() {
  for (int i = 0; i < MAX_OBS; i++) {
    if (!obstacles[i].active) {
      obstacles[i].w = 5;
      obstacles[i].h = 8;
      obstacles[i].x = random(0, SCREEN_WIDTH - obstacles[i].w);
      obstacles[i].y = -obstacles[i].h;
      obstacles[i].active = true;
      break;
    }
  }
}

void setup() {
  Wire.begin(8, 9);

  pinMode(BTN_LEFT, INPUT);
  pinMode(BTN_RIGHT, INPUT);
  pinMode(BTN_JUMP, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  randomSeed(analogRead(0));
}

void loop() {
  if (millis() - lastFrame < frameDelay) return;
  lastFrame = millis();

  // ---- INPUT ----
  if (digitalRead(BTN_LEFT) && playerX > 0) playerX -= 3;
  if (digitalRead(BTN_RIGHT) && playerX < SCREEN_WIDTH - PLAYER_W) playerX += 3;
  if (digitalRead(BTN_JUMP) && !jumping) {
    jumping = true;
    jumpVel = -6;
    jumpY = 0;
  }

  // ---- PLAYER JUMP ----
  if (jumping) {
    jumpY += jumpVel;
    jumpVel += 1; // gravity
    if (jumpY > 0) {
      jumpY = 0;
      jumping = false;
    }
  }

  // ---- OBSTACLES ----
  for (int i = 0; i < MAX_OBS; i++) {
    if (obstacles[i].active) {
      obstacles[i].y += 3;

      // Collision
      if (playerX < obstacles[i].x + obstacles[i].w &&
          playerX + PLAYER_W > obstacles[i].x &&
          playerY + jumpY < obstacles[i].y + obstacles[i].h &&
          playerY + jumpY + PLAYER_H > obstacles[i].y) {
        score = 0; // reset score on hit
        for (int j = 0; j < MAX_OBS; j++) obstacles[j].active = false;
        break;
      }

      // Passed obstacle
      if (obstacles[i].y > SCREEN_HEIGHT) {
        obstacles[i].active = false;
        score++;
      }
    }
  }

  // Spawn new obstacles randomly
  if (random(0, 10) < 3) spawnObstacle();

  // ---- RENDER ----
  display.clearDisplay();

  // Player
  display.fillRect(playerX, playerY + jumpY, PLAYER_W, PLAYER_H, SSD1306_WHITE);

  // Obstacles
  for (int i = 0; i < MAX_OBS; i++) {
    if (obstacles[i].active) {
      display.fillRect(obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h, SSD1306_WHITE);
    }
  }

  // Score
  display.setCursor(0,0);
  display.print("Score: ");
  display.print(score);

  display.display();
}
