# Übersicht: Intro, Projekt, Background & Ziel

## Intro

Im Rahmen von Polenta sowie später auch des Satelfests von Pro Velo erstellen wir eine interaktive Velo-Installation, die Bewegung, Wettbewerb und digitale Visualisierung miteinander verbindet. Besucher*innen können auf einem stationären Fahrrad fahren, während Geschwindigkeit, Distanz und weitere Bewegungsdaten in Echtzeit erfasst und verarbeitet werden. Die gesammelten Daten werden anschliessend live auf einer Webapp visualisiert.

## Background

Das Projekt verbindet Physical Computing mit Webtechnologien und schafft dadurch ein interaktives Erlebnis. Ziel ist es, die körperliche Aktivität der Teilnehmenden sichtbar und vergleichbar zu machen. Durch die Verbindung von Fahrrad, Sensorik und Webapp entsteht eine Installation, die direkt auf die Aktionen der Nutzenden reagiert.

Vor der Nutzung können sich Teilnehmende mit einem selbst gewählten Namen anmelden. Während der Fahrt werden ihre Werte erfasst und mit anderen verglichen. Die Webapp enthält zusätzlich eine Rangliste, auf der die höchsten erreichten Geschwindigkeiten angezeigt werden. Dadurch entsteht ein Wettbewerb, der Besucher*innen motiviert.

Neben dem öffentlichen Bereich verfügt die Webapp auch über einen separaten Administratorbereich mit Login. Dort können Verwaltungsfunktionen, Datenübersichten oder Einstellungen für die Installation zentral gesteuert werden.

## Ziel

Das Ziel des Projekts ist es, eine interaktive und motivierende Erfahrung zu schaffen, bei der Besucher*innen ihre Leistung direkt erleben können. Die Webapp ermöglicht es, persönliche Werte sowie Ranglisten live einzusehen und sich mit anderen Teilnehmenden zu vergleichen.

Zusätzlich soll das Projekt zeigen, wie physische Bewegung mit digitalen Medien, Echtzeitdaten und Sensorik verbunden werden kann. Der Administratorbereich ermöglicht dabei eine einfache Verwaltung der Installation und der erfassten Daten während des Events.

Die Installation dient somit sowohl als spielerische Attraktion als auch als praktisches Beispiel für interaktive Mediengestaltung im öffentlichen Raum.




Produktlogig (Basic Funktionsweise, Bedienung)
Entwickler: Namen

Inhaltsverzeichnis

Webapp:
- UX (Designentscheidung, Benutzerführung)
- Visualisierung des Screenflow (Flow Diagramm)




## Physical Computing

### Komponenten/Bauteile

Die Installation besteht aus einem stationären Fahrrad, einem Reed-Kontakt mit Magnet, einem ESP32-C6 sowie einem OLED-Display. Der Reed-Kontakt erkennt die Pedalumdrehungen und sendet die Signale an den ESP32-C6, welcher daraus Geschwindigkeit und Distanz berechnet.

Die Daten werden direkt auf dem Display angezeigt und zusätzlich per WLAN an eine Webapp übertragen. Dort können sich Nutzer*innen mit einem Namen anmelden, ihre Werte live verfolgen und die Rangliste der höchsten Geschwindigkeiten einsehen. Zusätzlich existiert ein Administratorbereich mit Login zur Verwaltung der Installation.

### Steckplan

Der ESP32-C6 bildet die zentrale Verbindung aller Komponenten. Der Reed-Kontakt ist mit einem GPIO-Pin verbunden und arbeitet im Input-Pullup-Modus. Jede erkannte Umdrehung erzeugt ein Signal, welches vom ESP32 verarbeitet wird.

Das OLED-Display ist über die I²C-Schnittstelle mit den SDA- und SCL-Pins angeschlossen und dient zur Anzeige der aktuellen Werte.

### Kommunikationsprozess der Komponenten

Beim Pedalieren bewegt sich ein Magnet am Reed-Kontakt vorbei und erzeugt elektrische Impulse. Der ESP32-C6 verarbeitet diese Signale und berechnet daraus Geschwindigkeit und Distanz.

Die Daten werden anschliessend per WLAN an einen Server übertragen und in einer Datenbank gespeichert. Die Webapp greift auf diese Datenbank zu und visualisiert die aktuellen Werte sowie die Rangliste in Echtzeit.

Der Ablauf erfolgt in folgenden Schritten:

1. Pedalbewegung
2. Erfassung durch Reed-Kontakt
3. Verarbeitung durch den ESP32-C6
4. Anzeige auf dem OLED-Display
5. Übertragung an die Datenbank
6. Darstellung in der Webapp

### Design Entscheidung Display

Die Anzeige wurde bewusst einfach und übersichtlich gestaltet. Das OLED-Display direkt an der Installation zeigt die wichtigsten Informationen wie die aktuelle Geschwindigkeit in km/h an. Zusätzlich können dort Status- und Fehlermeldungen dargestellt werden, beispielsweise bei Verbindungsproblemen oder wenn keine Sensordaten erkannt werden.

Für detailliertere Informationen wird zusätzlich eine Webapp verwendet, welche per QR-Code mit dem Smartphone geöffnet werden kann. Nutzer*innen können dadurch ihr eigenes Handy als erweitertes Display verwenden und dort zusätzliche Daten wie Distanz, persönliche Werte oder die Rangliste einsehen.

Diese Aufteilung reduziert die Komplexität der physischen Installation und ermöglicht gleichzeitig eine flexiblere Darstellung auf mobilen Geräten. Zudem bleibt die Installation übersichtlich und einfach bedienbar.





Reproduzierbarkeit (Wie kann das Projekt nachgebaut werden?)
Datei-Komponentenplan (Wo kommuniziert welche Datei mit welchem Protokoll mit welchen Client/Server/Datenbank?)
Video des Produkts in Aktion

Umsetzungsprozess (Entwicklung, Entscheidungen, Fehler und Probleme)
Bekannte Probleme mit dem Produkt

Lernerfolg

KI-Einsatz erwähnen




