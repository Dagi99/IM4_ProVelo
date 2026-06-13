#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

// Dieser Sketch misst die Anzahl der Kontaktschliessungen des Reed-Sensors,
// berechnet daraus die Geschwindigkeit und zeigt sie auf dem Display an.
// Zusätzlich werden die Geschwindigkeitswerte regelmäßig an die Datenbank geschickt.

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

//WLAN Zugangsdaten
const char* ssid = "Marks iPhone";
const char* pass = "passwort";
const char* serverURL = "https://provelo-allegra.piltoverprints.ch/PhysicalComputing/api/load.php";

// Nach wie vielen Millisekunden erneut WLAN probiert wird
#define WIFI_RECONNECT_INTERVAL 5000
// Delay wie oft die aktuelle Geschwindigkeit an den Server geschickt wird
#define DATABASE_SEND_INTERVAL 1000
// Entprellung des Reed-Sensors
#define REED_DEBOUNCE_TIME 100

// =====================================================
// Fahrrad Einstellungen
// =====================================================

// Rad Umfang in Metern (Float)
#define WHEEL_CIRCUMFERENCE 2.1

// ID vom Velo, damit in der DB mehrere Velos unterschieden werden koennen
#define VELO_ID 1

// Max Skala LED Ring
#define MAX_RING_SPEED 50.0

// Maximal darstellbare Geschwindigkeit auf dem Display
#define MAX_DISPLAY_SPEED 60.0

// =====================================================
// Variablen für Funktioun, Debouncing und Troubleshooting
// =====================================================

unsigned long lastPulseTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastDatabaseSend = 0;
unsigned long lastDebounceTime = 0;

// speedKm ist die aktuell berechnete Geschwindigkeit
float speedKmh = 0.0;

bool lastReedState = HIGH;
bool wifiConnected = false;

// Falls etwas schieflaeuft, wird diese Meldung auf dem OLED angezeigt
String displayErrorMessage = "Error";

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
// Setup/ESP Init
// =====================================================

void setup() {

  Serial.begin(115200);

  // Protokoll des Displays starten
  Wire.begin(DISPLAY_SDA, DISPLAY_CLOCK);

  // Reed-Sensor Pin starten
  pinMode(REED_PIN, INPUT_PULLUP);

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

  Serial.println("System started");
}

// =====================================================
// Loop
// =====================================================

void loop() {

  //Auslesen des Reed-Sensors
  bool reedState = digitalRead(REED_PIN);

  // Wenn der Reed-Sensor von HIGH auf LOW wechselt, zaehlen wir das als eine Radumdrehung
  if (lastReedState == HIGH && reedState == LOW) {

    unsigned long currentTime = millis();

    // Entprellung: Nur Impulse verarbeiten, die genuegend Zeit seit dem letzten haben
    if (currentTime - lastDebounceTime >= REED_DEBOUNCE_TIME) {

      // Debounce-Zimer aktualisieren
      lastDebounceTime = currentTime;

      // Beim ersten Impuls koennen wir noch keine Geschwindigkeit berechnen, weil noch kein Vergleichswert vorhanden ist
      if (lastPulseTime > 0) {

        unsigned long deltaTime = currentTime - lastPulseTime;

        if (deltaTime > 0) {

          // Zeitdifferenz in Sekunden umrechnen
          float timeSeconds = deltaTime / 1000.0;

          // Aus Weg / Zeit zuerst m/s berechnen
          float speedMs =
            WHEEL_CIRCUMFERENCE / timeSeconds;

          // Danach von m/s auf km/h umrechnen
          speedKmh = speedMs * 3.6;

          // Falls der Wert zu hoch wird, auf Anzeige-Maximum begrenzen
          if (speedKmh > MAX_DISPLAY_SPEED) {
            speedKmh = MAX_DISPLAY_SPEED;
          }

          Serial.print("Speed: ");
          Serial.print(speedKmh);
          Serial.println(" km/h");
        }
      }
      //Festhalten, wann der letzte Impuls war
      lastPulseTime = currentTime;
    }
  }

  // Speichern des aktuellen Zustands für den nächsten Durchlauf
  lastReedState = reedState;

  // Wenn länger nichts kommt, gehen wir davon aus, dass das Velo steht
  if (millis() - lastPulseTime > 4000) {
    speedKmh = 0;
  }

  // Intervall basiertes schreiben der Geschwindigkeit in die Datenbank
  if (millis() - lastDatabaseSend >= DATABASE_SEND_INTERVAL && speedKmh > 0) {
    sendSpeedToDatabase(speedKmh);
    lastDatabaseSend = millis();
  }

  // Ausgaben aktualisieren
  ensureWiFiConnected();
  updateLedRing();
  updateDisplay();
}

void connectWiFi() {
  Serial.printf("Verbinde mit WLAN %s\n", ssid);
  WiFi.begin(ssid, pass);
  int attempts = 0;

  // Mehrere Versuche machen, bevor ein Fehler angezeigt wird
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  // Erfolg melden
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    clearDisplayError();
    Serial.printf("\nWiFi verbunden. IP: %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  // Fehler melden
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

  // Nicht in jedem Loop sofort neu verbinden, sonst blockiert der Sketch zu stark
  if (millis() - lastWiFiReconnectAttempt >= WIFI_RECONNECT_INTERVAL) {
    lastWiFiReconnectAttempt = millis();
    connectWiFi();
  }
  return WiFi.status() == WL_CONNECTED;
}

// Sende die Geschwindigkeit an die Datenbank
bool sendSpeedToDatabase(float speedValue) {
  if (!ensureWiFiConnected()) {
    return false;
  }

  // JSON fuer das PHP-Backend zusammenbauen
  JSONVar dataObject;
  dataObject["velo_id"] = VELO_ID;
  dataObject["wert"] = speedValue;
  String jsonString = JSON.stringify(dataObject);

  // HTTP POST an die API schicken
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

  // LEDs laufen einmal von gruen nach rot hoch
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
  // Je schneller das Velo ist, desto mehr LEDs leuchten
  int ledsToLight = map(
    constrain(speedKmh, 0, MAX_RING_SPEED),
    0,
    MAX_RING_SPEED,
    0,
    STRIP_COUNT
  );

  strip.clear();

  for (int i = 0; i < ledsToLight; i++) {
    // Verlauf von gruen zu rot
    int red = map(i, 0, STRIP_COUNT - 1, 0, 255);
    int green = map(i, 0, STRIP_COUNT - 1, 255, 0);
    strip.setPixelColor(i, strip.Color(red, green, 0));
  }

  strip.show();
}

void updateDisplay() {
  // Display nicht in jedem Loop neu zeichnen, damit es ruhiger laeuft
  if (millis() - lastDisplayUpdate <= 200) {
    return;
  }

  display.clearDisplay();

  // Anzeige von Fehlermeldungen
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

  //  Geschwindigkeitsanzeige
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
