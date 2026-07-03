/*
 * reed_sender.ino — ESP32-C6: Reed-Sensor → Geschwindigkeit, WLAN → DB + OSC zu Display-ESP & TouchDesigner
 * VELO_ID 1 = Bike A, VELO_ID 2 = Bike B (pro Fahrrad separat flashen).
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

// =====================================================
// Init Reed Sensor
// =====================================================

#define REED_PIN 4

// =====================================================
// Init WiFi / Database
// =====================================================

const char* ssid = "tinkergarden";
const char* pass = "strenggeheim";
const char* serverURL = "https://provelo-allegra.piltoverprints.ch/PhysicalComputing/api/load.php";

#define WIFI_RECONNECT_INTERVAL 5000
#define DATABASE_SEND_INTERVAL 1000
#define REED_DEBOUNCE_TIME 50

// =====================================================
// Init WiFi UDP
// =====================================================

WiFiUDP Udp;
const IPAddress tdIp(192, 168, 0, 154);      // TouchDesigner PC IP
const IPAddress displayIp(192, 168, 0, 155); // ESP32 #2 — auf tatsächliche IP setzen!
const unsigned int remotePort = 9000;
const unsigned int localPort  = 8000;

// =====================================================
// Fahrrad Einstellungen
// =====================================================

#define WHEEL_CIRCUMFERENCE 2.1
#define VELO_ID 2
#define MAX_DISPLAY_SPEED 60.0

// =====================================================
// Variablen
// =====================================================

unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastDatabaseSend = 0;
unsigned long lastPulseTime = 0;

float speedKmh = 0.0;
bool wifiConnected = false;
String displayErrorMessage = "";

// =====================================================
// ISR Variablen
// =====================================================

volatile unsigned long isrLastPulseTime = 0;
volatile unsigned long isrDeltaTime = 0;
volatile bool newSpeedReady = false;

TaskHandle_t uploadTaskHandle;

// =====================================================
// Funktionsdeklarationen
// =====================================================

void connectWiFi();
bool ensureWiFiConnected();
bool sendSpeedToDatabase(float speedValue);
void sendSpeedOSC(float speedValue);
void setDisplayError(const String& message);
void clearDisplayError();

// =====================================================
// ISR
// =====================================================

void IRAM_ATTR reedISR() {
  unsigned long currentTime = millis();
  if (currentTime - isrLastPulseTime >= REED_DEBOUNCE_TIME) {
    if (isrLastPulseTime > 0) {
      isrDeltaTime = currentTime - isrLastPulseTime;
      // Nur realistische Geschwindigkeiten akzeptieren (min ~126ms = 60 km/h)
      if (isrDeltaTime >= 126) {
        newSpeedReady = true;
      }
    }
    isrLastPulseTime = currentTime;
  }
}

// =====================================================
// Upload Task (Kern 0)
// =====================================================

void uploadTask(void * parameter) {
  for(;;) {
    ensureWiFiConnected();
    if (millis() - lastDatabaseSend >= DATABASE_SEND_INTERVAL) {
      if (speedKmh > 0) sendSpeedToDatabase(speedKmh);
      sendSpeedOSC(speedKmh);
      lastDatabaseSend = millis();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// =====================================================
// Setup
// =====================================================

void setup() {
  Serial.begin(115200);

  pinMode(REED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REED_PIN), reedISR, FALLING);

  connectWiFi();
  Udp.begin(localPort);

  xTaskCreatePinnedToCore(
    uploadTask,
    "UploadTask",
    10000,
    NULL,
    1,
    &uploadTaskHandle,
    0
  );

  Serial.println("Reed Sender gestartet");
}

// =====================================================
// Loop (Kern 1)
// =====================================================

void loop() {
  if (newSpeedReady) {
    noInterrupts();
    unsigned long currentDelta = isrDeltaTime;
    newSpeedReady = false;
    interrupts();

    float timeSeconds = currentDelta / 1000.0;
    float speedMs = WHEEL_CIRCUMFERENCE / timeSeconds;
    speedKmh = speedMs * 3.6;

    if (speedKmh > MAX_DISPLAY_SPEED) speedKmh = MAX_DISPLAY_SPEED;

    Serial.printf("Speed: %.1f km/h\n", speedKmh);
    lastPulseTime = millis();
  }

  if (millis() - lastPulseTime > 4000) {
    speedKmh = 0;
  }
}

// =====================================================
// Hilfsfunktionen
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

bool ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return true;
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
  return WiFi.status() == WL_CONNECTED;
}

bool sendSpeedToDatabase(float speedValue) {
  if (!wifiConnected) return false;

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

void sendSpeedOSC(float speedValue) {
  if (!wifiConnected) return;

  // Senden an TouchDesigner
  OSCMessage msg("/bike2/speed");
  msg.add(speedValue);
  Udp.beginPacket(tdIp, remotePort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  // Senden an Display-ESP
  OSCMessage msg2("/bike2/speed");
  msg2.add(speedValue);
  Udp.beginPacket(displayIp, remotePort);
  msg2.send(Udp);
  Udp.endPacket();
  msg2.empty();
}

void setDisplayError(const String& message) {
  displayErrorMessage = message;
}

void clearDisplayError() {
  displayErrorMessage = "";
}