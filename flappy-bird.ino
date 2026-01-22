#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define BTN_JUMP 13  // button to flap

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Bird
#define BIRD_W 5
#define BIRD_H 5
int birdX = 30;
int birdY = SCREEN_HEIGHT / 2;
float birdVY = 0;
const float gravity = 0.5;
const float flap = -6;

// Pipes
#define PIPE_W 10
#define GAP_H 20
#define MAX_PIPES 3
struct Pipe {
  int x;
  int gapY;
  bool active;
};
Pipe pipes[MAX_PIPES];
const int pipeSpacing = 60; // horizontal spacing

// Game
int score = 0;
unsigned long lastFrame = 0;
const int frameDelay = 40; // ~25 FPS

void spawnPipe(int i) {
  pipes[i].x = SCREEN_WIDTH;
  pipes[i].gapY = random(10, SCREEN_HEIGHT - GAP_H - 10);
  pipes[i].active = true;
}

void setup() {
  Wire.begin(8, 9);
  pinMode(BTN_JUMP, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true);
  }

  display.setRotation(2); // flip display

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  randomSeed(analogRead(0));

  // initialize pipes
  for (int i = 0; i < MAX_PIPES; i++) {
    spawnPipe(i);
    pipes[i].x += i * pipeSpacing;
  }
}

void loop() {
  if (millis() - lastFrame < frameDelay) return;
  lastFrame = millis();

  // ---- INPUT ----
  if (digitalRead(BTN_JUMP)) {
    birdVY = flap;
  }

  // ---- UPDATE ----
  birdVY += gravity;
  birdY += birdVY;

  if (birdY < 0) birdY = 0;
  if (birdY > SCREEN_HEIGHT - BIRD_H) {
    birdY = SCREEN_HEIGHT - BIRD_H;
    birdVY = 0;
  }

  // Pipes movement
  for (int i = 0; i < MAX_PIPES; i++) {
    if (pipes[i].active) {
      pipes[i].x -= 3;

      // Check collision
      if (birdX + BIRD_W > pipes[i].x && birdX < pipes[i].x + PIPE_W) {
        if (birdY < pipes[i].gapY || birdY + BIRD_H > pipes[i].gapY + GAP_H) {
          // collision
          score = 0;
          birdY = SCREEN_HEIGHT / 2;
          birdVY = 0;
          for (int j = 0; j < MAX_PIPES; j++) spawnPipe(j);
          break;
        }
      }

      // Passed pipe
      if (pipes[i].x + PIPE_W < birdX && pipes[i].active) {
        score++;
        pipes[i].active = false;
      }

      // Respawn pipe
      if (pipes[i].x < -PIPE_W) {
        spawnPipe(i);
      }
    }
  }

  // ---- RENDER ----
  display.clearDisplay();

  // Bird
  display.fillRect(birdX, birdY, BIRD_W, BIRD_H, SSD1306_WHITE);

  // Pipes
  for (int i = 0; i < MAX_PIPES; i++) {
    if (pipes[i].active) {
      // top pipe
      display.fillRect(pipes[i].x, 0, PIPE_W, pipes[i].gapY, SSD1306_WHITE);
      // bottom pipe
      display.fillRect(pipes[i].x, pipes[i].gapY + GAP_H, PIPE_W, SCREEN_HEIGHT - (pipes[i].gapY + GAP_H), SSD1306_WHITE);
    }
  }

  // Score
  display.setCursor(SCREEN_WIDTH/2 - 10, 0);
  display.print(score);

  display.display();
}
