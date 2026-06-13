/*
 * speedometer.ino — ESP32-C6: Reed-Sensor → Geschwindigkeit, OLED + LED-Ring, WLAN → load.php.
 * VELO_ID 1 = Bike A, VELO_ID 2 = Bike B (pro Fahrrad separat flashen).
 */
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
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
// Init Reed Sensor
// =====================================================

#define REED_PIN 4

// =====================================================
// Init WiFi / Database
// =====================================================

const char* ssid = "Marks iPhone";
const char* pass = "passwort";
const char* serverURL = "https://provelo-allegra.piltoverprints.ch/PhysicalComputing/api/load.php";

#define WIFI_RECONNECT_INTERVAL 5000
#define DATABASE_SEND_INTERVAL 1000
#define REED_DEBOUNCE_TIME 100

// =====================================================
// Fahrrad Einstellungen
// =====================================================

#define WHEEL_CIRCUMFERENCE 2.1
#define VELO_ID 1
#define MAX_RING_SPEED 50.0
#define MAX_DISPLAY_SPEED 60.0

// =====================================================
// Variablen für Funktionen, Debouncing und Troubleshooting
// =====================================================

unsigned long lastDisplayUpdate = 0;
unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastDatabaseSend = 0;

// Für den Stand-Check im Loop (4 Sekunden)
unsigned long lastPulseTime = 0; 

float speedKmh = 0.0;
bool wifiConnected = false;
String displayErrorMessage = "Error";

// =====================================================
// Neue Variablen für den Hardware-Interrupt (ISR)
// =====================================================

volatile unsigned long isrLastPulseTime = 0;
volatile unsigned long isrDeltaTime = 0;
volatile bool newSpeedReady = false;

// TaskHandle für den Datenbank-Upload auf Kern 0
TaskHandle_t uploadTaskHandle;

// =====================================================
// Übersicht über verwendeten Funktionen
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
// Die Interrupt-Service-Routine (ISR)
// =====================================================

void IRAM_ATTR reedISR() {
  unsigned long currentTime = millis();
  
  // Entprellung
  if (currentTime - isrLastPulseTime >= REED_DEBOUNCE_TIME) {
    if (isrLastPulseTime > 0) {
      // Berechne die Zeit für exakt eine Radumdrehung sicher im Interrupt
      isrDeltaTime = currentTime - isrLastPulseTime;
      newSpeedReady = true;
    }
    isrLastPulseTime = currentTime;
  }
}

// =====================================================
// Die Task für Kern 0 (WLAN & Datenbank)
// =====================================================

void uploadTask(void * parameter) {
  for(;;) { 
    // WLAN-Status prüfen und ggf. neu verbinden (blockiert jetzt nur Kern 0)
    ensureWiFiConnected();

    // Datenbank-Upload Interval
    if (millis() - lastDatabaseSend >= DATABASE_SEND_INTERVAL && speedKmh > 0) {
      sendSpeedToDatabase(speedKmh);
      lastDatabaseSend = millis();
    }
    
    // Dem Watchdog-Timer des ESP32 Zeit zum Atmen geben (Verhindert Abstürze)
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// =====================================================
// Setup/ESP Init
// =====================================================

void setup() {
  Serial.begin(115200);

  // Protokoll des Displays starten
  Wire.begin(DISPLAY_SDA, DISPLAY_CLOCK);

  // Display initialisieren
  if (!display.begin(0x3C, true)) {
    Serial.println("Display error");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.display();

  // LED-Ring initialisieren
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  // Startanimation abspielen
  showStartupAnimation();
  
  // Versuch WLAN Verbindung
  connectWiFi();

  // Reed-Sensor initialisieren und Interrupt aktivieren (FALLING = HIGH zu LOW)
  pinMode(REED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REED_PIN), reedISR, FALLING);

  // Startet den Upload-Task auf Kern 0
  xTaskCreatePinnedToCore(
    uploadTask,       // Die Funktion der Task
    "UploadTask",     // Name der Task
    10000,            // Stack-Größe
    NULL,             // Parameter
    1,                // Priorität
    &uploadTaskHandle,// Handle
    0                 // Läuft auf Kern 0
  );

  Serial.println("System started");
}

// =====================================================
// Loop (Läuft auf Kern 1 - Nur für Anzeige & Mathe)
// =====================================================

void loop() {

  // Prüfen, ob der Interrupt eine neue Geschwindigkeit registriert hat
  if (newSpeedReady) {
    
    // Interrupts kurz pausieren, um die Variable sicher zu lesen
    noInterrupts();
    unsigned long currentDelta = isrDeltaTime;
    newSpeedReady = false;
    interrupts();

    // Berechnung
    float timeSeconds = currentDelta / 1000.0;
    float speedMs = WHEEL_CIRCUMFERENCE / timeSeconds;
    speedKmh = speedMs * 3.6;

    if (speedKmh > MAX_DISPLAY_SPEED) {
      speedKmh = MAX_DISPLAY_SPEED;
    }

    Serial.print("Speed: ");
    Serial.print(speedKmh);
    Serial.println(" km/h");
    
    // lastPulseTime aktualisieren für den Stand-Check
    lastPulseTime = millis(); 
  }

  // Wenn länger als 4 Sekunden kein Impuls kommt, Velo steht
  if (millis() - lastPulseTime > 4000) {
    speedKmh = 0;
  }

  // Ausgaben aktualisieren (laufen jetzt absolut flüssig)
  updateLedRing();
  updateDisplay();
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
    if (displayErrorMessage == "Keine WLAN\nVerbindung") {
      clearDisplayError();
    }
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
  if (!wifiConnected) {
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
    // Auskommentiert, damit der Serial Monitor nicht überflutet wird,
    // kann zum Debuggen wieder aktiviert werden.
    // Serial.printf("HTTP Response code: %d\n", httpResponseCode);
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