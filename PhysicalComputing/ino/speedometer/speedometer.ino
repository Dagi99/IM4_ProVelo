#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
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
// WiFi / Database
// =====================================================

const char* ssid = "tinkergarden";
const char* pass = "strenggeheim";
const char* serverURL = "https://provelo-allegra.piltoverprints.ch/PhysicalComputing/api/load.php";

#define WIFI_RECONNECT_INTERVAL 5000
#define DATABASE_SEND_INTERVAL 2000
#define REED_DEBOUNCE_TIME 100

// =====================================================
// Wheel settings
// =====================================================

// Wheel circumference in meters
#define WHEEL_CIRCUMFERENCE 2.1

// Unique bike identifier for database writes
#define VELO_ID 2

// LED ring full scale speed
#define MAX_RING_SPEED 50.0

// Maximum speed on display
#define MAX_DISPLAY_SPEED 60.0

// =====================================================
// Variables
// =====================================================

unsigned long lastPulseTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastDatabaseSend = 0;
unsigned long lastDebounceTime = 0;

float speedKmh = 0.0;

bool lastReedState = HIGH;
bool wifiConnected = false;

String displayErrorMessage = "";

// =====================================================
// Function declarations
// =====================================================

void connectWiFi();
bool ensureWiFiConnected();
bool sendSpeedToDatabase(float speedValue);
void setDisplayError(const String& message);
void clearDisplayError();
void showStartupAnimation();
void updateLedRing();
void updateDisplay();

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

  showStartupAnimation();
  connectWiFi();

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

  // Send to database at fixed intervals
  if (millis() - lastDatabaseSend >= DATABASE_SEND_INTERVAL && speedKmh > 0) {
    sendSpeedToDatabase(speedKmh);
    lastDatabaseSend = millis();
  }

  ensureWiFiConnected();
  updateLedRing();
  updateDisplay();
}

void connectWiFi() {
  Serial.printf("Verbinde mit WLAN %s\n", ssid);
  WiFi.begin(ssid, pass);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    clearDisplayError();
    Serial.printf("\nWiFi verbunden. IP: %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  wifiConnected = false;
  setDisplayError("Keine WLAN\nVerbindung");
  Serial.println("\nWiFi Verbindung fehlgeschlagen");
}

bool ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    if (displayErrorMessage == "Keine WLAN\nVerbindung") {
      clearDisplayError();
    }
    return true;
  }

  if (wifiConnected) {
    Serial.println("WiFi-Verbindung verloren");
  }

  wifiConnected = false;
  setDisplayError("Keine WLAN\nVerbindung");
  if (millis() - lastWiFiReconnectAttempt >= WIFI_RECONNECT_INTERVAL) {
    lastWiFiReconnectAttempt = millis();
    connectWiFi();
  }
  return WiFi.status() == WL_CONNECTED;
}

bool sendSpeedToDatabase(float speedValue) {
  if (!ensureWiFiConnected()) {
    return false;
  }

  JSONVar dataObject;
  dataObject["velo_id"] = VELO_ID;
  dataObject["wert"] = speedValue;
  String jsonString = JSON.stringify(dataObject);

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0 && httpResponseCode < 400) {
    if (displayErrorMessage == "Kein Zugriff\nauf Datenbank") {
      clearDisplayError();
    }
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    String response = http.getString();
    if (response.length() > 0) {
      Serial.println("Response: " + response);
    }
    http.end();
    return true;
  }

  setDisplayError("Kein Zugriff\nauf Datenbank");

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Fehler: %d\n", httpResponseCode);
  } else {
    Serial.printf("POST Fehler: %d\n", httpResponseCode);
  }

  http.end();
  return false;
}

void setDisplayError(const String& message) {
  displayErrorMessage = message;
}

void clearDisplayError() {
  displayErrorMessage = "";
}

void showStartupAnimation() {
  strip.clear();

  for (int i = 0; i < STRIP_COUNT; i++) {
    int red = map(i, 0, STRIP_COUNT - 1, 0, 255);
    int green = map(i, 0, STRIP_COUNT - 1, 255, 0);
    strip.setPixelColor(i, strip.Color(red, green, 0));
    strip.show();
    delay(80);
  }

  delay(250);
  strip.clear();
  strip.show();
}

void updateLedRing() {
  int ledsToLight = map(
    constrain(speedKmh, 0, MAX_RING_SPEED),
    0,
    MAX_RING_SPEED,
    0,
    STRIP_COUNT
  );

  strip.clear();

  for (int i = 0; i < ledsToLight; i++) {
    int red = map(i, 0, STRIP_COUNT - 1, 0, 255);
    int green = map(i, 0, STRIP_COUNT - 1, 255, 0);
    strip.setPixelColor(i, strip.Color(red, green, 0));
  }

  strip.show();
}

void updateDisplay() {
  if (millis() - lastDisplayUpdate <= 200) {
    return;
  }

  display.clearDisplay();

  if (displayErrorMessage.length() > 0) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Fehler");
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println(displayErrorMessage);
    display.display();
    lastDisplayUpdate = millis();
    return;
  }

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Speed");

  display.setTextSize(5);
  display.setCursor(0, 18);

  if (speedKmh < 10) {
    display.print(" ");
  }

  display.print((int)speedKmh);

  display.setTextSize(2);
  display.setCursor(95, 45);
  display.print("km");

  display.display();
  lastDisplayUpdate = millis();
}
