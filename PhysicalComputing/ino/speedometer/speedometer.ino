#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

// =====================================================
// OLED Display
// =====================================================

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64

#define DISPLAY_SDA    21
#define DISPLAY_CLOCK  22

Adafruit_SH1106G display(128, 64, &Wire, -1);

// =====================================================
// NeoPixel Ring
// =====================================================

#define STRIP_PIN       7
#define STRIP_COUNT    12
#define BRIGHTNESS     50

Adafruit_NeoPixel strip(
  STRIP_COUNT,
  STRIP_PIN,
  NEO_GRB + NEO_KHZ800
);

// =====================================================
// Reed Sensor
// =====================================================

#define REED_PIN 4

// =====================================================
// Wheel settings
// =====================================================

// Wheel circumference in meters
#define WHEEL_CIRCUMFERENCE 2.2

// LED ring full scale speed
#define MAX_RING_SPEED 50.0

// Maximum displayed speed
#define MAX_DISPLAY_SPEED 99.0

// =====================================================
// Variables
// =====================================================

unsigned long lastPulseTime = 0;
unsigned long lastDisplayUpdate = 0;

float speedKmh = 0.0;

bool lastReedState = HIGH;

// =====================================================
// Setup
// =====================================================

void setup() {

  Serial.begin(115200);

  // I2C
  Wire.begin(DISPLAY_SDA, DISPLAY_CLOCK);

  // Reed sensor
  pinMode(REED_PIN, INPUT_PULLUP);

  // OLED init
  if (!display.begin(0x3C, true)) {
    Serial.println("Display error");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.display();

  // NeoPixel init
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  Serial.println("System started");
}

// =====================================================
// Main loop
// =====================================================

void loop() {

  bool reedState = digitalRead(REED_PIN);

  // Detect wheel rotation
  if (lastReedState == HIGH && reedState == LOW) {

    unsigned long currentTime = millis();

    // Ignore first pulse
    if (lastPulseTime > 0) {

      unsigned long deltaTime = currentTime - lastPulseTime;

      if (deltaTime > 0) {

        // Speed calculation
        float timeSeconds = deltaTime / 1000.0;

        float speedMs =
          WHEEL_CIRCUMFERENCE / timeSeconds;

        speedKmh = speedMs * 3.6;

        // Limit max display speed
        if (speedKmh > MAX_DISPLAY_SPEED) {
          speedKmh = MAX_DISPLAY_SPEED;
        }

        Serial.print("Speed: ");
        Serial.print(speedKmh);
        Serial.println(" km/h");
      }
    }

    lastPulseTime = currentTime;

    // Debounce
    delay(20);
  }

  lastReedState = reedState;

  // Stop detection
  if (millis() - lastPulseTime > 3000) {
    speedKmh = 0;
  }

  // =====================================================
  // LED Ring Speedometer
  // =====================================================

  int ledsToLight = map(
    constrain(speedKmh, 0, MAX_RING_SPEED),
    0,
    MAX_RING_SPEED,
    0,
    STRIP_COUNT
  );

  strip.clear();

  for (int i = 0; i < ledsToLight; i++) {

    // Green -> Red gradient

    int red = map(
      i,
      0,
      STRIP_COUNT - 1,
      0,
      255
    );

    int green = map(
      i,
      0,
      STRIP_COUNT - 1,
      255,
      0
    );

    strip.setPixelColor(
      i,
      strip.Color(red, green, 0)
    );
  }

  strip.show();

  // =====================================================
  // Display update
  // =====================================================

  if (millis() - lastDisplayUpdate > 200) {

    display.clearDisplay();

    // Title
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Speed");

    // Big speed number
    display.setTextSize(5);
    display.setCursor(0, 18);

    if (speedKmh < 10) {
      display.print(" ");
    }

    display.print((int)speedKmh);

    // Unit
    display.setTextSize(2);
    display.setCursor(95, 45);
    display.print("km");

    display.display();

    lastDisplayUpdate = millis();
  }
}