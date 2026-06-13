
# README: Interaktive Velo-Installation

## Kurzbeschreibung des Projekts

* **Modul:** Interaktive Medien 4 an der Fachhochschule Graubünden (FS26)  
* **Themenfeld:** Interaktive Velo-Installation (Bewegung, Wettbewerb & digitale Visualisierung)
* **Name des Projekts:** `Interaktive Velo-Installation`   
* **Team Physical Computing:** `, Mark Hamann`   Kamil Matyja`
* **Team WebApp:** `[NOTIZ: Namen ergänzen]`
 
### Problemstellung & Systemzweck
  Das Projekt löst kein klassisches Alltagsproblem, sondern ist spezifisch auf eine Installation im öffentlichen Raum angelegt. Es verbindet die physische Bewegung auf einem Fahrrad mit der digitalen Erfassung in einem digitalen Wettbewerb. Die Installation wird entwickelt für die "Polenta" und dem "Satelfest" von Pro Velo in Chur.

  Ziel ist es, eine interaktive und motivierende Erfahrung zu schaffen, bei der Besucher\*innen ihre Leistung direkt sehen können. Durch das Tracken von Echtzeitdaten (Geschwindigkeit, Distanz) und das Bereitstellen einer Live-Rangliste entsteht ein Gamification-Ansatz, der zu Bewegung und kleinen Wettkämpfen anspornt.

  Entwickelt wurde ein - theoretisch skalierbares System - welches je Fahrrad eine Microcontroller mit Peripherie benötigt. In unserem Aufbau haben wir zwei Fahrräder umgesetzt.

---

### UX & Konzeption

* **Figma:** [Link zum Figma](https://www.figma.com/design/lTifdIONmofy3zx2MQEjBC/IM-4-%E2%80%93-App-Konzeption-Velo-Race?node-id=78-325&t=3EO7N0VVxFRgnQxb-1)
* **User Flow & Screen Flow:**
  `[NOTIZ: Screenshot aus Figma hier einfügen oder verlinken]`

#### Features und Produktlogik
* **Angedachte Features:**
  * Echtzeit-Erfassung von Geschwindigkeit und Distanz über ein stationäres Fahrrad.
  * Live-Visualisierung der Fahrdaten auf einer Webapp.
  * Fahrer-Anmeldung mit einem selbst gewählten Namen vor der Fahrt.
  * Bestenliste/Rangliste mit den höchsten erreichten Geschwindigkeiten.
  * Separater, passwortgeschützter Administratorbereich zur Steuerung und Datenbereinigung.
  * Reduziertes physisches Display am Lenker für Core-Daten
* **Nicht umgesetzte Features:**
  `[NOTIZ: Welche Features wurden nicht umgesetzt und warum? Bitte hier ergänzen]`

---

## Setup

* **WebApp:** [Link zur Website](http://link.zur.website) `[NOTIZ: Echten Link einfügen]`  
* **Video-Dokumentation:** [Link zum Video auf Youtube](http://link.zum.video) `[NOTIZ: Echten Link einfügen]` 


### Installationsanleitung

`[NOTIZ: Hier kommt eine verständliche Schritt-für-Schritt-Anleitung für Aussenstehende rein.]`

1. **Infrastruktur:**
- Webserver mit **PHP 8.x** (getestet mit PHP 8.4) und **MariaDB 10.6** (oder MySQL 8.x)
- Schreibzugriff auf PHP-Sessions (Standard-Session-Handling für Login/Admin)
- Optional: **HTTPS** für den Produktivbetrieb (empfohlen für Admin-Login)
- Kein Node.js / npm / Composer erforderlich.
2. **Webserver-Installation:**
1. Repository klonen:
      ```bash
      git clone <repository-url>
      cd IM4_ProVelo
      ```
   2. Gesamten Projektordner per **SFTP** ins Webroot des Hostings hochladen
   3. Sicherstellen, dass folgende Pfade erreichbar sind:
      - `index.html` → leitet auf `leaderboard.html` weiter
      - `api/` → PHP-Backend (Login, Challenge, Rangliste, Admin)
      - `PhysicalComputing/api/load.php` → Empfängt ESP-Geschwindigkeitsdaten
   4. Keine weiteren Build-Schritte nötig (kein `npm install`, kein `composer install`).
3. **Datenbank-Import:**
   1. In **phpMyAdmin** (oder per CLI) eine leere Datenbank anlegen.
   2. SQL-Dateien **in dieser Reihenfolge** importieren (Tab „SQL“ → Inhalt einfügen → Ausführen):
      | Reihenfolge | Datei | Zweck |
      |-------------|-------|--------|
      | 1 | [`system/db.sql`](system/db.sql) | Tabelle `users` (Admin-Login) |
      | 2 | [`system/speed.sql`](system/speed.sql) | Tabelle `speed` (ESP-Telemetrie) |
      | 3 | [`resources/sql/assigned_names.sql`](resources/sql/assigned_names.sql) | Tabelle `assigned_names` (Spielernamen) |
      | 4 | [`system/highscores.sql`](system/highscores.sql) | Rangliste |
      | 5 | [`resources/sql/challenge_state.sql`](resources/sql/challenge_state.sql) | Challenge-Status |
      | 6 | [`resources/sql/challenge_presence.sql`](resources/sql/challenge_presence.sql) | Anwesenheit der Bikes |
      | 7 | [`resources/sql/challenge_results.sql`](resources/sql/challenge_results.sql) | Optional: detaillierte Ergebnisse |
   3. Admin-Benutzer anlegen über `register.html`
      ```
4. **Datenbank-Credentials:**
- Zugangsdaten in [`system/config.php`](system/config.php) eintragen:
     ```php
     $host = 'localhost';      // z. B. db-Host des Providers
     $db   = 'deine_datenbank';
     $user = 'dein_db_user';
     $pass = 'dein_db_passwort';
     ```
   - Es gibt **keine** `.env`-Datei; alle PHP-APIs binden diese Datei per `require_once` ein.
   - Nach dem Upload: Seite `leaderboard.html` öffnen (Rangliste) und `login.html` (Admin). Bei Verbindungsfehlern erscheint eine Meldung aus `config.php`.
5. **Inbetriebnahme des physischen Artefakts:** Der ESP32 wird wie definert verkabelt. Anschliessend wird der Microcontroller per USB-Verbindung mit dem Computer verbunden. Zum Beispiel mit der Software *Arduino IDE* kann das Programm *speedometer.ino* hochgeladen werden. Zuvor muss in der IDE im Programmcode in Zeile 69 die *Velo-ID* festlegen (1 für Velo A, 2 für Velo B). In Zeile 50 bis 52 müssen die WLAN-Credentials und die Server-URL wie im Muster eingetragen werden.

---

### Bauanleitung Physical Computing

#### Komponenten & Bauteile
Die Installation besteht aus folgenden Komponenten:
* Stationäres, aufgebocktes Fahrrad. Je Fahrrad:
* **ESP32-C6**  Zentrale Steuereinheit
* **Reed-Kontakt und zugehöriger Magnet an Speiche** (Erfassung der Rad- bzw. Pedalumdrehungen)
* **OLED-Display** (Direkte Anzeige der Geschwindigkeit am Rad)
* **WS128b 12px LED Ring** (Visuelle Darstellung der Geschwindigkeit)
* **3D Druck Bauteile** (als Gehäuse und Montage)

#### Kommunikationsprozess der Komponenten
1. **Pedalbewegung:** Beim Tretten bewegt sich ein Magnet am Reed-Kontakt vorbei und schliesst den Schalter-Kontakt
2. **Erfassung:** Der Reed-Kontakt (eingestellt im *Input-Pullup-Modus*) registriert das Signal an einem GPIO-Pin des ESP32-C6.
3. **Verarbeitung:** Der ESP32-C6 berechnet aus den Impulsen in Echtzeit Geschwindigkeit und Distanz.
4. **Lokale Anzeige:** Die aktuelle Geschwindigkeit wird auf dem OLED-Display und LED-Ring ausgegeben.
5. **Übertragung:** Die Daten werden alle Sekunde per WLAN an einen Server gesendet und in der Datenbank gespeichert.
6. **Webapp-Darstellung:** Die Webapp greift auf die Datenbank zu und visualisiert Daten und Ranglisten live.


![Not-accurate diagram of the data flow in the project](/documentation/ressources/Flowdiagramm.png)

#### Komponentenplan & Steckplan
* **Komponentenplan:** `[NOTIZ: Schaubild einfügen/verlinken, das Komponenten, Sensoren, Aktoren, Dateinamen der Programme und Kommunikationswege zeigt]`
![Placeholder Picture](/documentation/ressources/Komponentenplan.png)

* **Steckplan:** 
![Schematic plan of the breadboard with ESP32-C6, Button as Reed-Kontakt, OLED-Display und WS128b 12px LED Ring](/documentation/ressources/Steckplan.png)

* Der Microcontroller ist zentrales Element des Systems und bedient alle anderen Komponenten. Alle untergeordneten Komponenten (OLED-Display, WS128b 12px LED Ring) beziehen Ground, VCC, Daten und ggf. Clock vom Microcontroller. Der Reed-Kontakt wird per Input Pullup-Modus an GPIO erfasst.
* OLED-Display wird mit **SDA an GPIO 21, mit SCL an GPIO 22** angeschlossen.
* Der Reed-Kontakt wird **zwischen 5V und GPIO 4** angeschlossen.
* Der Datenkontakt des WS128b 12px LED Ring wird an **GPIO 7** angeschlossen.

---

## Technische Details

### Projektstruktur / Code-Struktur
`[NOTIZ: Hier die Verzeichnisstruktur einfügen (z.B. als Baumdiagramm). Wichtig: Jede Datei muss im Kopfbereich eine kurze Zusammenfassung enthalten.]`

### Datenschnittstelle
* Der ESP verpackt ca. einmal pro Sekunde die aktuell berechnete Geschwindigkeit in einem JSON-Objekt und sendet es als HTTP Post an die API des Server, welcher diese per PHP in der Datenbank speichert. 

### ERM (Entity-Relationship-Modell)
`[NOTIZ: Erklärung der Tabellenstrukturen (z.B. Users, Rides, Admins) sowie das grafische ERM-Schaubild hier einfügen]`

### Authentifizierung
`[NOTIZ: Erklärung einfügen, wie die Authentifizierung für den Administratorbereich und das Session-Handling der User gelöst wurde]`

---
## 3D-Modelle
* Um die Bauteile Produktionsgerecht zu gestalten wurden 3D-Modelle erstellt, welche zu den Komponenten passen und es ermöglichen diese am Fahrrad zu befestigen.

* **Displayhalterung:** Für das Display ist eine Halterung vorgesehen, welcher per Kabelbinder am Lenker des Fahrrades montiert werden kann. Das Gehäuse hat Löcher für Kabelbinder (kein Metalldraht!) um am Lenker fixiert zu werden.

* ![3D Animation of a display mounting case, rotating](/documentation/ressources/IM4Velo_Display.gif)

* **LED-Ring:** Für den LED-Ring, welcher die Geschwindigkeit wie ein Tacho anzeigen soll, wurde ein Teller-artiges Modell modeliert, in welches der Ring eingelassen werden kann. Auch hier sind Löcher in der Rückseite berücksichtig, um das Kreisförmige Teil fixieren zu können.

* ![3D Animation of a LED ring mounting case, rotating](/documentation/ressources/IM4Velo_LED-Ring.gif)

* **Microcontroller-Box:** Für den Microcontroller wurde eine Box modelliert, welche die Elektronik schützt und Kabeldurchlässe berücksichtigt.

* ![3D Animation of a microcontroller case, rotating](/documentation/ressources/IM4Velo_Box.gif)


---

## Known Bugs (Bekannte Probleme)

* Wenn das Gerät an einen neuen Ort bewegt wird, kann der Microcontroller keine Verbindung zu einem neuen Netzwerk herstellen ohne das Programm erneut und angepasst zu flashen.
* Der Umfang vom Rad als Berechnungsgrundlage für die Geschwindigkeit lässt sich nicht Benutzerseitig verändern.
* `[NOTIZ: Was funktioniert noch nicht einwandfrei?]`
* `[NOTIZ: Was ist während der Entwicklung aufgefallen?]`
* `[NOTIZ: Welche Optimierungen könnten in einer Version 2.0 vorgenommen werden?]`

---

## Umsetzungsprozess

### Reflexion / Erfahrung / Lernfortschritt
`[NOTIZ: Was wurde gelernt? Würdet ihr es wieder so machen? Was lief gut/schlecht?]`

### Herausforderungen & Lösungen
`[NOTIZ: Welche Fehler traten auf, welche Ansätze wurden verworfen, wie sahen die Umplanungen aus?]`

### KI-Einsatz
In der Umsetzung dieses Projektes wurde KI in Code-Produktion und Code-Troubleshooting verwendet.

### Fazit
`[NOTIZ: Abschliessendes Fazit zum Projektfortschritt und dem Endergebnis bei den Events.]`


Notizen für den finanlen Reflexionstext:

* Mark: Vor allem die Konzentration auf den Designaspekte des Prototyp war und ist für mich gewöhnungsbedürftig. Die Zeit und der Umfang des Kurses reicht nicht wirklich aus, um einen Prototypen vollständig zu entwickeln. Ein qualitativer Proof-of-Konzept hätte für den Lernerfolg deutlich größeres Potenzial.

Kamil: Wir haben uns für ein anspruchsvolles Projekt entschieden und dabei viel gelernt. Besonders spannend war die Zusammenarbeit zwischen dem Webapp- und dem Physical-Computing-Team. Das Projekt erforderte viel Zeit und Aufwand, da zahlreiche Herausforderungen und unerwartete Fehler gelöst werden mussten.

Fadri: Physical Computing in der Praxis anzuwenden, war eine sehr gute Ergänzung zum bisherigen Stoff. Dass aus der ursprünglichen Idee ein umfangreicheres Projekt entstanden ist, machte die Arbeit zwar aufwendiger, aber auch deutlich spannender. Beim nächsten Mal würde ich allerdings mein persönliches Zeitmanagement noch etwas verbessern.

Notizen für den finalen Fazit-Text:
* Mark: Das Projekt war eine gute Gelegenheit, die Arbeit im Team zu üben. Grade Kommunikation kann ein Knackpunkt sein. Im großen und ganzen bin ich aber zufrieden mit dem Ergebnis. Die Rahmenbedingungen waren z.T. eher ungünstig, weil Informationen über die Anforderungen an das Produkt und die Einsatzumgebung nicht gesammelt zur Verfügung standen.

Kamil: Insgesamt war das Projekt sehr lehrreich. Trotz des hohen Zeitaufwands und der zahlreichen Herausforderungen konnten wir viele praktische Erfahrungen sammeln und unser Wissen erweitern. Die Arbeit an einem komplexen Projekt hat uns gezeigt, wie wichtig eine gute Planung, Ausdauer und systematische Fehlersuche sind. Rückblickend sind wir mit dem Ergebnis zufrieden und konnten sowohl fachlich als auch persönlich viel dazulernen.

Fadri:Das Projekt hat gezeigt, dass die Kommunikation bei einem solchen Vorhaben schwierig sein kann. Auch die Planung hätte im Vorfeld besser sein können, um den Aufwand gezielter abzufangen. Trotz dieser organisatorischen Herausforderungen und einiger technischer Hürden haben wir am Ende eine funktionierende Installation umgesetzt. Insgesamt bin ich mit dem Resultat zufrieden.