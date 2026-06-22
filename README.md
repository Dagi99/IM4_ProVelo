
# README: Interaktive Velo-Installation

## Kurzbeschreibung des Projekts

* **Modul:** Interaktive Medien 4 an der Fachhochschule Graubünden (FS26)  
* **Themenfeld:** Interaktive Velo-Installation (Bewegung, Wettbewerb & digitale Visualisierung)
* **Name des Projekts:** Interaktive Velo-Installation
* **Team Physical Computing:** Mark Hamann, Fadri Collet
* **Team WebApp:** Kamil Matyja, Danino Giger
* **Creative Technology:** Kamil Matyja, Danino Giger, Fadri Collet
 
### Problemstellung & Systemzweck
  Das Projekt löst kein klassisches Alltagsproblem, sondern ist spezifisch auf eine Installation im öffentlichen Raum angelegt. Es verbindet die physische Bewegung auf einem Fahrrad mit der digitalen Erfassung in einem digitalen Wettbewerb. Die Installation wird entwickelt für die "Polenta" und dem "Satelfest" von Pro Velo in Chur.

  Ziel ist es, eine interaktive und motivierende Erfahrung zu schaffen, bei der Besucher\*innen ihre Leistung direkt sehen können. Durch das Tracken von Echtzeitdaten (Geschwindigkeit, Distanz) und das Bereitstellen einer Live-Rangliste entsteht ein Gamification-Ansatz, der zu Bewegung und kleinen Wettkämpfen anspornt.

  Entwickelt wurde ein - theoretisch skalierbares System - welches je Fahrrad eine Microcontroller mit Peripherie benötigt. In unserem Aufbau haben wir zwei Fahrräder umgesetzt.

---

### UX & Konzeption

* **Figma:** [Link zum Figma](https://www.figma.com/design/lTifdIONmofy3zx2MQEjBC/IM-4-%E2%80%93-App-Konzeption-Velo-Race?node-id=78-325&t=3EO7N0VVxFRgnQxb-1)

#### Features und Produktlogik
* **Angedachte Features:**
  * Echtzeit-Erfassung von Geschwindigkeit und Distanz über ein stationäres Fahrrad.
  * Live-Visualisierung der Fahrdaten auf einer Webapp.
  * Fahrer-Anmeldung mit einem selbst gewählten Namen vor der Fahrt.
  * Bestenliste/Rangliste mit den höchsten erreichten Geschwindigkeiten.
  * Separater, passwortgeschützter Administratorbereich zur Steuerung und Datenbereinigung.
  * Reduziertes physisches Display am Lenker für Core-Daten
* **Nicht umgesetzte Features:**
  * Benutzerdefinierte Namen wurden nicht implementiert. Nutzer\*innen bekommen einen zufälligen Namen zugeteilt.

---

## Setup

* **WebApp:** [Link zur Website](https://provelo-allegra.piltoverprints.ch/leaderboard.html)
* **Video-Dokumentation:** [Link zum Video auf Youtube](https://youtu.be/euy03_3Im-A)


### Installationsanleitung

1. **Infrastruktur:**
- Webserver mit **PHP 8.x** (getestet mit PHP 8.4) und **MariaDB 10.6** (oder MySQL 8.x)
- Schreibzugriff auf PHP-Sessions (Standard-Session-Handling für Login/Admin)
- Optional: **HTTPS** für den Produktivbetrieb (empfohlen für Admin-Login)
- Kein Node.js / npm / Composer erforderlich.
2. **Webserver-Installation:**
1. Repository klonen:
      ```bash
      git clone <https://github.com/Dagi99/IM4_ProVelo.git>
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
5. **Inbetriebnahme des physischen Artefakts:** Der ESP32 wird wie angegeben verkabelt. Anschliessend wird der Microcontroller per USB-Verbindung mit dem Computer verbunden. Zum Beispiel mit der Software *Arduino IDE* kann das Programm *speedometer.ino* hochgeladen werden.
Zuvor muss
- in der IDE im Programmcode in Zeile 69 die *Velo-ID* festlegen (1 für Velo A, 2 für Velo B).
- in der IDE im Programmcode in Zeile 50 bis 52 müssen die *WLAN-Credentials* und die *Server-URL* wie im Muster eingetragen werden.

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

##### Zentrale Creative-Technology-Installation
   * **TouchDesigner** zur Echtzeitverarbeitung der Renndaten
   * **DMX-Steckdose** als Schnittstelle zur Licht- und Haptiksteuerung
   * **LED-Streifen** zur Visualisierung des aktuellen Rennverlaufs
   * **Vibrationsmotor** für haptisches Feedback am Rennende

#### Kommunikationsprozess der Komponenten

1. Pedalbewegung:
Beim Treten bewegt sich ein Magnet am Reed-Kontakt vorbei und löst ein elektrisches Signal aus.
2. Erfassung der Radumdrehungen
Der Reed-Kontakt registriert jede Radumdrehung und übermittelt die Impulse an den ESP32-C6.
3. Berechnung der Fahrdaten
Der ESP32-C6 berechnet aus den erfassten Impulsen in Echtzeit die aktuelle Geschwindigkeit und die zurückgelegte Distanz.
4. Lokale Anzeige am Fahrrad
Die berechnete Geschwindigkeit wird direkt auf dem OLED-Display angezeigt. Zusätzlich visualisiert ein LED-Ring die Geschwindigkeit durch verschiedene Farbeffekte.
5. Übertragung an den Server
Der ESP32-C6 sendet die Messdaten im JSON-Format per WLAN an den Server. Dort werden sie in einer Datenbank gespeichert und für weitere Anwendungen bereitgestellt.
6. Verarbeitung in TouchDesigner
TouchDesigner liest die aktuellen Renndaten aus der Datenbank aus und verarbeitet sie in Echtzeit, um die externen Ausgabegeräte anzusteuern.
7. Visuelles Feedback über den LED-Streifen
Für die Lichtsteuerung werden die Daten über das Netzwerk an einen Pixel-LED-Controller übertragen. Dieser steuert den LED-Streifen über ein vorgeschaltetes LED Constant Voltage Control Device an. Während des Rennens visualisiert der LED-Streifen den aktuellen Führenden. Bike A wird durch die Farbe Blau und Bike B durch Orange dargestellt. Je nach Vorsprung verändert sich der Farbverlauf dynamisch zwischen beiden Farben.
8. Haptisches Feedback über den Vibrationsmotor
Für die Ansteuerung des Vibrationsmotors sendet TouchDesigner die Steuerdaten per ArtNet an einen ArtNet-zu-DMX-Konverter. Dieser erzeugt ein DMX512-Signal, das an einen DMX-Dimmer weitergeleitet wird. Der DMX-Dimmer steuert schliesslich den Vibrationsmotor an. Fünf Sekunden vor Rennende wird der Motor aktiviert und bleibt bis zum Ablauf des Countdowns eingeschaltet. Nach Erreichen von null Sekunden wird er automatisch deaktiviert.
9. Aktualisierung der Webanwendung
Parallel dazu liest die Webanwendung die gespeicherten Daten aus der Datenbank aus und aktualisiert fortlaufend die Rangliste sowie die Live-Anzeigen des Rennens.

#### Komponentenplan & Steckplan
* **Komponentenplan:** 
![Schematische Darstellung der Komponenten-Kommunikation](/documentation/ressources/Komponentenplan.png)

* **Steckplan:** 
![Schematische Darstellung des Aufbaus auf dem Breadbord mit ESP32-C6, Taster als Reed-Kontakt, OLED-Display und WS128b 12px LED Ring](/documentation/ressources/Steckplan.png)

* Der Microcontroller ist zentrales Element des Systems und bedient alle anderen Komponenten. Alle untergeordneten Komponenten (OLED-Display, WS128b 12px LED Ring) beziehen Ground, VCC, Daten und ggf. Clock vom Microcontroller. Der Reed-Kontakt wird per Input Pullup-Modus an GPIO erfasst.
* OLED-Display wird mit **SDA an GPIO 21, mit SCL an GPIO 22** angeschlossen.
* Der Reed-Kontakt wird **zwischen 5V und GPIO 4** angeschlossen.
* Der Datenkontakt des WS128b 12px LED Ring wird an **GPIO 7** angeschlossen.

---

## Technische Details

### Projektstruktur / Code-Struktur

```
IM4_ProVelo/
├── index.html              → Weiterleitung zur Rangliste
├── leaderboard.html        → Öffentliche Rangliste
├── infoBikeA.html / infoBikeB.html
├── challengeName.html      → Spielername zuweisen (?velo=1|2)
├── race.html               → 90s-Duell
├── login.html / register.html / admin.html
├── api/                    → PHP-Backend
│   ├── challenge-status.php, challenge-heartbeat.php, assign-name.php
│   ├── leaderboard.php
│   ├── login.php, logout.php, protected.php, register.php
│   └── admin/delete-highscore.php
├── js/                     → Frontend-Logik (race.js, leaderboard.js, admin.js, …)
├── css/                    → Styles (style.css, race.css, leaderboard.css, …)
├── system/                 → config.php, db.sql, speed.sql, highscores.sql
├── resources/sql/          → Challenge-Tabellen, assigned_names.sql
└── PhysicalComputing/
    ├── ino/speedometer/speedometer.ino
    └── api/load.php        → ESP-Empfang
```

Systemübersicht: [Komponentenplan](/documentation/ressources/Komponentenplan.png) · [Datenfluss ESP](/documentation/ressources/Flowdiagramm.png)

### Datenschnittstelle
* Der ESP verpackt ca. einmal pro Sekunde die aktuell berechnete Geschwindigkeit in einem JSON-Objekt und sendet es als HTTP Post an die API des Server, welcher diese per PHP in der Datenbank speichert. 

### ERM (Entity-Relationship-Modell)

Die Datenbank besteht aus **7 Tabellen** in einer MariaDB-Instanz. Es gibt keine separate Tabelle für Fahrräder: **Bike A** und **Bike B** werden über `velo_id` (1 bzw. 2) in mehreren Tabellen unterschieden.

**Entitäten**

| Tabelle | Zweck |
|---------|--------|
| `users` | Admin-Benutzer (Login für Highscore-Verwaltung) |
| `speed` | Rohe Geschwindigkeits-Telemetrie vom ESP (kurzlebig, ~15 Min.) |
| `assigned_names` | Pool zufälliger Spielernamen vor dem Rennen |
| `highscores` | Rangliste: Distanz-Ergebnisse pro Spieler und Bike |
| `challenge_state` | Globaler Challenge-Status (genau eine Zeile, id=1) |
| `challenge_presence` | Heartbeat pro Bike (Anwesenheit am Rennen) |
| `challenge_results` | Optional: Detail-Ergebnis pro Challenge-Runde |

**Logische Beziehungen**

![ERM Velo-Installation](/documentation/ressources/ERM.png)

- **`velo_id`** verknüpft `speed`, `challenge_presence`, `highscores`, `challenge_results` und (bei Zuweisung) `assigned_names` mit demselben physischen Fahrrad (1 = A, 2 = B).
- **`assigned_names` → `highscores`:** Beim Rennende wird der Spielername als Text in `highscores.player_name` gespeichert (Kopie, kein FK).
- **`challenge_state` → `challenge_results`:** Ergebnisse gehören zur Challenge-Runde über denselben `started_at`-Zeitstempel.
- **`users`:** Steht isoliert; nur für Admin-Login, nicht für Besucher am Fahrrad.

**Hinweis:** `users` kann in der Produktion zusätzlich `firstname`/`lastname` enthalten (`profile.php`); in `system/db.sql` sind nur `id`, `email`, `password` definiert.

### Authentifizierung

Das System unterscheidet zwei getrennte Session-Konzepte: **öffentlicher Challenge-Betrieb** (Besucher am Fahrrad) und **geschützter Admin-Bereich** (Highscores verwalten).

#### Admin-Bereich (Login)

Der Admin-Zugang schützt ausschliesslich die Verwaltung der Rangliste — nicht die öffentlichen Seiten wie Rangliste, Rennen oder Namens-Zuweisung.

**Registrierung und Login**
- Neue Admins registrieren sich über `register.html` → `api/register.php`.
- Das Passwort wird serverseitig mit `password_hash()` gehasht und in der Tabelle `users` gespeichert (`system/db.sql`).
- Der Login über `login.html` → `api/login.php` prüft E-Mail und Passwort mit `password_verify()`.
- Bei Erfolg startet PHP eine **Session** und speichert `user_id` und `email` in `$_SESSION`.
- Zur Absicherung wird die Session-ID nach Login mit `session_regenerate_id()` erneuert; das Session-Cookie ist `secure`.

**Geschützte Seiten und APIs**
- `admin.html` lädt `js/auth.js`, das beim Seitenaufruf `api/protected.php` abfragt (`credentials: "include"`).
- Ist keine gültige Session vorhanden, antwortet `protected.php` mit **HTTP 401** — der Browser wird zu `login.html` weitergeleitet.
- Auch serverseitig geschützt: z. B. `api/admin/delete-highscore.php` prüft `isset($_SESSION['user_id'])` und lehnt unautorisierte Requests ab.
- **Logout:** `logout.js` ruft `api/logout.php` auf, leert und zerstört die Session, danach Redirect zu `login.html`.

**Rollenmodell**
- Es gibt **keine separate Admin-Rolle** in der Datenbank: **jeder eingeloggte Benutzer** aus `users` gilt als Admin und darf Highscores löschen.

#### Challenge-Spieler (ohne Login)

Besucher am Fahrrad müssen sich **nicht** anmelden.

- Auf `challengeName.html` vergibt `api/assign-name.php` einen **Zufallsnamen** aus `assigned_names` und speichert ihn in der **PHP-Session** pro Bike (`velo_id` 1 oder 2).
- Der Name bleibt für diese Browser-Session erhalten (`$_SESSION['players']`, `player_name`).
- Challenge-Logik und Rangliste-Speicherung laufen über `api/challenge-status.php` serverseitig; die öffentliche Rangliste (`leaderboard.html`) ist ohne Login erreichbar.

#### Sicherheitshinweise

- Echte Autorisierung passiert **immer serverseitig** (PHP prüft die Session); Frontend-Redirects in `auth.js` sind nur UX.
- Admin-APIs dürfen sensible Aktionen nicht allein im Frontend verstecken — Löschen ist nur mit gültiger Session möglich.
- Für Produktion wird **HTTPS** empfohlen (`session.cookie_secure` in `login.php` ist vorbereitet, aber auskommentiert).
- ESP-Daten (`PhysicalComputing/api/load.php`) und öffentliche Challenge-APIs sind **nicht** admin-geschützt (Installation im öffentlichen Raum).

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

* Wenn das Gerät an einen neuen Ort bewegt wird, kann der Microcontroller keine Verbindung zu einem neuen Netzwerk herstellen ohne das Programm erneut in der IDE anzupassen und zu flashen.
* Der Umfang vom Rad als Berechnungsgrundlage für die Geschwindigkeit lässt sich nicht Benutzerseitig verändern.
* Das Flashen des Microcontrollers hat seine Tücken da der Code zwei Prozesse im ESP32 implementiert. Es ist wichtig, dass das korrekte Board in der IDE ausgewählt wird.
* Wichtig ist eine stabile Anbringung am Rad, damit die Sensoren korrekt funktionieren und die Komponenten sich nicht lösen.
* Laptop stürzt beim öffnen bestimmter touchdesigner sachen ab.
---

## Umsetzungsprozess

### Reflexion / Erfahrung / Lernfortschritt

Im Verlauf des Projekts konnten zahlreiche praktische Erfahrungen gesammelt und vorhandenes Wissen erweitert werden. Besonders wertvoll war die Verbindung zwischen Webentwicklung und Physical Computing, wodurch die Zusammenarbeit zwischen unterschiedlichen technischen Bereichen erprobt werden konnte. Die Umsetzung eines anspruchsvollen Projekts brachte viele neue Erkenntnisse, insbesondere im Bereich der Integration verschiedener Systeme, der Problemlösung und der iterativen Entwicklung.

Ein wichtiger Lernaspekt war die Erkenntnis, dass die Entwicklung eines vollständigen und ausgereiften Prototyps innerhalb des zeitlichen Rahmens eines Kurses nur begrenzt realistisch ist. Rückblickend hätte eine stärkere Fokussierung auf einen qualitativen Proof-of-Concept möglicherweise einen höheren Lernerfolg ermöglicht. Gleichzeitig zeigte das Projekt, wie wichtig eine realistische Planung, klare Priorisierung und ein effizientes Zeitmanagement bei komplexen Entwicklungsprojekten sind.

### Herausforderungen & Lösungen

Während der Umsetzung traten verschiedene technische und organisatorische Herausforderungen auf. Die Kombination aus Webapplikation, Physical Computing und Creative Technology führte zu einem erhöhten Aufwand bei der Integration und Fehlersuche der einzelnen Komponenten. Viele Probleme konnten durch systematisches Testen, wiederholte Anpassungen und eine iterative Vorgehensweise gelöst werden.

Auch die praktische Umsetzung erwies sich als anspruchsvoll. Die Elektronik musste platzsparend in einer Transportkiste untergebracht und die Verkabelung möglichst sauber und unsichtbar verlegt werden. Dadurch waren mehrere Anpassungen am Aufbau und an der Positionierung der Komponenten notwendig.

Zudem waren die Anforderungen an das Endprodukt sowie die Bedingungen der späteren Einsatzumgebung zu Beginn noch nicht vollständig definiert. Dies führte während der Entwicklung zu Umplanungen und verdeutlichte die Bedeutung einer guten Kommunikation sowie einer flexiblen Projektplanung.

### KI-Einsatz

In der Umsetzung dieses Projekts wurde KI sowohl zur Unterstützung der Code-Produktion als auch beim Troubleshooting und der Fehlersuche eingesetzt. Sie diente als Hilfsmittel zur Analyse von Problemen, zur Erarbeitung von Lösungsansätzen und zur Beschleunigung wiederkehrender Entwicklungsaufgaben.

### Fazit

Das Projekt war insgesamt eine sehr lehrreiche Erfahrung und bot die Möglichkeit, die Zusammenarbeit im Team sowie die praktische Umsetzung eines komplexen technischen Systems zu vertiefen. Besonders die Verbindung von Webentwicklung, Physical Computing und Creative Technology zeigte, wie wichtig eine gute Planung und die Abstimmung zwischen den einzelnen Teilbereichen sind.

Trotz verschiedener technischer und organisatorischer Herausforderungen konnte eine funktionierende Installation realisiert werden. Insgesamt ist das Ergebnis zufriedenstellend und der Projektverlauf ermöglichte sowohl fachliche als auch methodische Weiterentwicklungen.