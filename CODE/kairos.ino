#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- TB6612FNG ----------
#define PWMA 10
#define PWMB 11
#define AIN1 12
#define AIN2 2
#define BIN1 7
#define BIN2 3
#define STBY 8   // STANDBY pin

// ---------- LED ----------
#define LED_PIN 4

// ---------- MOTOR SPEED ----------
int motorSpeed = 120;   // 0–255 (safe zone)

void setup() {
  // Motor pins
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);   // 👈 IMPORTANT

  pinMode(LED_PIN, OUTPUT);

  // Wake up TB6612FNG
  digitalWrite(STBY, HIGH);  // 👈 STANDBY = HIGH (enabled)

  // OLED init
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1); // OLED failed — hard stop
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("LFR BOOT");
  display.println("STBY: HIGH");
  display.println("Motors: FORWARD");
  display.println("Status: OK");
  display.display();

  // Motor direction: FORWARD
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}

void loop() {
  // Run motors
  analogWrite(PWMA, motorSpeed);
  analogWrite(PWMB, motorSpeed);

  // Blink LED (heartbeat)
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  delay(300);
}
