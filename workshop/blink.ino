#define LED 2
#define LED2 4

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);
  digitalWrite(LED2, LOW);
  delay(500);
  digitalWrite(LED, LOW);
  digitalWrite(LED2, HIGH);
  delay(500);
}
