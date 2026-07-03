/*
 * display_receiver.ino — ESP32-C6: Empfängt Geschwindigkeit via OSC → OLED Display + LED-Ring
 * Empfängt von reed_sender.ino via OSC über WiFi
 */

#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

// =====================================================
// Init OLED Display
// =====================================================

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define DISPLAY_SDA    21
#define DISPLAY_CLOCK  22

Adafruit_SH1106G display(128, 64, &Wire, -1);

// =====================================================
// Init NeoPixel Ring
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
// Init WiFi / UDP
// =====================================================

const char* ssid = "tinkergarden";
const char* pass = "strenggeheim";

WiFiUDP Udp;
const unsigned int localPort = 9000;

// =====================================================
// Fahrrad Einstellungen
// =====================================================

#define VELO_ID 2
#define MAX_RING_SPEED   50.0
#define MAX_DISPLAY_SPEED 60.0

// =====================================================
// Variablen
// =====================================================

float speedKmh = 0.0;
bool wifiConnected = false;
String displayErrorMessage = "";

unsigned long lastDisplayUpdate = 0;
unsigned long lastOSCReceived = 0;
unsigned long lastWiFiReconnectAttempt = 0;

#define WIFI_RECONNECT_INTERVAL 5000

// =====================================================
// Funktionsdeklarationen
// =====================================================

void connectWiFi();
void ensureWiFiConnected();
void receiveOSC();
void updateLedRing();
void updateDisplay();
void showStartupAnimation();
void setDisplayError(const String& message);
void clearDisplayError();

// =====================================================
// Setup
// =====================================================

void setup() {
  Serial.begin(115200);

  // Display starten
  Wire.begin(DISPLAY_SDA, DISPLAY_CLOCK);
  if (!display.begin(0x3C, true)) {
    Serial.println("Display error");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.display();

  // LED-Ring starten
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  // Startanimation
  showStartupAnimation();

  // WiFi verbinden
  connectWiFi();

  // UDP starten
  Udp.begin(localPort);
  Serial.println("UDP gestartet, Port: " + String(localPort));

  Serial.println("Display Receiver gestartet");
  Serial.printf("Eigene IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.println(">>> Diese IP in reed_sender.ino als displayIp eintragen! <<<");
}

// =====================================================
// Loop
// =====================================================

void loop() {
  ensureWiFiConnected();
  receiveOSC();

  // Wenn länger als 4 Sekunden kein OSC empfangen → Velo steht
  if (millis() - lastOSCReceived > 4000) {
    speedKmh = 0;
  }

  updateLedRing();
  updateDisplay();
}

// =====================================================
// OSC Empfang
// =====================================================

void receiveOSC() {
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) msg.fill(Udp.read());
    if (!msg.hasError()) {
      msg.dispatch("/bike2/speed", [](OSCMessage &m) {
        speedKmh = m.getFloat(0);
        if (speedKmh > MAX_DISPLAY_SPEED) speedKmh = MAX_DISPLAY_SPEED;
        lastOSCReceived = millis();
        Serial.printf("OSC empfangen: %.1f km/h\n", speedKmh);
      });
    }
  }
}

// =====================================================
// WiFi
// =====================================================

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

void ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    if (displayErrorMessage == "Keine WLAN\nVerbindung") {
      clearDisplayError();
    }
    return;
  }

  if (wifiConnected) {
    Serial.println("WLAN Verbindung verloren");
  }

  wifiConnected = false;
  setDisplayError("Keine WLAN\nVerbindung");

  if (millis() - lastWiFiReconnectAttempt >= WIFI_RECONNECT_INTERVAL) {
    lastWiFiReconnectAttempt = millis();
    connectWiFi();
  }
}

// =====================================================
// LED Ring
// =====================================================

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

// =====================================================
// Display
// =====================================================

void updateDisplay() {
  if (millis() - lastDisplayUpdate <= 200) return;

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
  display.setCursor(12, 0);
  display.println("Speed");

  display.setTextSize(5);
  display.setCursor(12, 18);
  if (speedKmh < 10) display.print(" ");
  display.print((int)speedKmh);

  display.setTextSize(2);
  display.setCursor(80, 45);
  display.print("km/h");

  display.display();
  lastDisplayUpdate = millis();
}

// =====================================================
// Startanimation
// =====================================================

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

// =====================================================
// Hilfsfunktionen
// =====================================================

void setDisplayError(const String& message) {
  displayErrorMessage = message;
}

void clearDisplayError() {
  displayErrorMessage = "";
}