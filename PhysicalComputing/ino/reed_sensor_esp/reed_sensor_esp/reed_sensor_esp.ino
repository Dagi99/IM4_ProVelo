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
 #define REED_DEBOUNCE_TIME 180

 // ======================================================
 // Init Wifi Udp
 // =======================================================

 WiFiUDP Udp;        // TouchDesigner (Core 0)
 WiFiUDP UdpDisplay; // Display ESP (Core 1)

 const IPAddress remoteIp(192, 168, 0, 154);  // TouchDesigner PC IP
 const IPAddress displayIp(192, 168, 0, 35);  // Display ESP IP
 const unsigned int remotePort = 9000;
 const unsigned int localPort  = 8000;

 // =====================================================
 // Fahrrad Einstellungen
 // =====================================================

 #define WHEEL_CIRCUMFERENCE 2.1
 #define VELO_ID 2
 #define MAX_DISPLAY_SPEED 60.0

 // =====================================================
 // Variablen für Funktionen, Debouncing und Troubleshooting
 // =====================================================

 unsigned long lastWiFiReconnectAttempt = 0;
 unsigned long lastDatabaseSend = 0;

 // Für den Stand-Check im Loop (4 Sekunden)
 unsigned long lastPulseTime = 0;

 float speedKmh = 0.0;
 bool wifiConnected = false;
 String displayErrorMessage = "";

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
 void sendSpeedOSC_TD(float speedValue);
 void sendSpeedOSC_Display(float speedValue);
 void setDisplayError(const String& message);
 void clearDisplayError();

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
     ensureWiFiConnected();

     if (millis() - lastDatabaseSend >= DATABASE_SEND_INTERVAL) {
       if (speedKmh > 0) sendSpeedToDatabase(speedKmh);
       sendSpeedOSC_TD(speedKmh);  // always send, even 0 so TD knows bike stopped
       lastDatabaseSend = millis();
     }

     vTaskDelay(10 / portTICK_PERIOD_MS);
   }
 }

 // =====================================================
 // Sendet OSC an TouchDesigner (Core 0)
 // =====================================================

 void sendSpeedOSC_TD(float speedValue) {
   if (!wifiConnected) return;

   OSCMessage msg("/bike2/speed");
   msg.add(speedValue);
   Udp.beginPacket(remoteIp, remotePort);
   msg.send(Udp);
   Udp.endPacket();
   msg.empty();
 }

 // =====================================================
 // Sendet OSC an Display ESP (Core 1)
 // =====================================================

 void sendSpeedOSC_Display(float speedValue) {
   if (!wifiConnected) return;

   OSCMessage msg("/bike2/speed");
   msg.add(speedValue);
   UdpDisplay.beginPacket(displayIp, remotePort);
   msg.send(UdpDisplay);
   UdpDisplay.endPacket();
   msg.empty();
 }

 // =====================================================
 // Setup/ESP Init
 // =====================================================

 void setup() {
   Serial.begin(115200);

   // Versuch WLAN Verbindung
   connectWiFi();

   // Reed-Sensor initialisieren und Interrupt aktivieren (FALLING = HIGH zu LOW)
   pinMode(REED_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(REED_PIN), reedISR, FALLING);

   // Startet den Upload-Task auf Kern 0
   xTaskCreatePinnedToCore(
     uploadTask,        // Die Funktion der Task
     "UploadTask",      // Name der Task
     10000,             // Stack-Größe
     NULL,              // Parameter
     1,                 // Priorität
     &uploadTaskHandle, // Handle
     0                  // Läuft auf Kern 0
   );

   // Started UDP
   Udp.begin(localPort);
   UdpDisplay.begin(localPort + 1); // port 8001
   Serial.println("UDP gestartet, Port: " + String(localPort));

   Serial.println("System started");
 }

 // =====================================================
 // Loop (Läuft auf Kern 1 - Nur für Mathe & Display OSC)
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

     // Sofort an Display senden
     sendSpeedOSC_Display(speedKmh);
   }

   // Wenn länger als 4 Sekunden kein Impuls kommt, Velo steht
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