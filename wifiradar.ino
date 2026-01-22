#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Radar settings
const int centerX = SCREEN_WIDTH/2;
const int centerY = SCREEN_HEIGHT/2;
const int radarRadius = 30;
float sweepAngle = 0;

// WiFi scan interval
unsigned long lastScan = 0;
const int scanInterval = 5000; // scan every 5s

struct Blip {
  int x;
  int y;
  int rssi;
};
#define MAX_BLIPS 20
Blip blips[MAX_BLIPS];
int blipCount = 0;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) while(true);
  display.clearDisplay();
  display.setRotation(2); // flip screen if needed
}

void scanWiFi() {
  blipCount = 0;
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && blipCount < MAX_BLIPS; i++) {
    int rssi = WiFi.RSSI(i);
    // Map RSSI (-90 to -30 dBm) to distance (radarRadius to 0)
    int dist = map(constrain(rssi, -90, -30), -90, -30, radarRadius, 0);

    // Random angle around radar for now
    float angle = random(0, 360) * PI / 180.0;

    blips[blipCount].x = centerX + dist * cos(angle);
    blips[blipCount].y = centerY + dist * sin(angle);
    blips[blipCount].rssi = rssi;
    blipCount++;
  }
  Serial.println(blipCount);
}

void loop() {
  unsigned long now = millis();
  if (now - lastScan > scanInterval) {
    lastScan = now;
    scanWiFi();
  }

  // Clear display
  display.clearDisplay();

  // Draw radar circle
  display.drawCircle(centerX, centerY, radarRadius, SSD1306_WHITE);
  display.drawCircle(centerX, centerY, radarRadius/2, SSD1306_WHITE);
  display.drawLine(centerX - radarRadius, centerY, centerX + radarRadius, centerY, SSD1306_WHITE);
  display.drawLine(centerX, centerY - radarRadius, centerX, centerY + radarRadius, SSD1306_WHITE);

  // Sweep line
  int sweepX = centerX + radarRadius * cos(sweepAngle);
  int sweepY = centerY + radarRadius * sin(sweepAngle);
  display.drawLine(centerX, centerY, sweepX, sweepY, SSD1306_WHITE);
  sweepAngle += 0.1; // rotate sweep
  if (sweepAngle > 2*PI) sweepAngle = 0;

  // Draw blips
  for (int i = 0; i < blipCount; i++) {
    display.fillCircle(blips[i].x, blips[i].y, 2, SSD1306_WHITE);
  }

  // Update display
  display.display();
  delay(50); // ~20 FPS
}
