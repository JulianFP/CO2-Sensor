# CO2-Sensor
## Deutsch
Dies ist ein CO2- und Luftqualitätssensor, den ich während der Qualifikationsphase in der Schule gebaut habe. Hintergrund ist u.a. die Covid-19 Pandemie, da wegen dieser in Schulen die Klassenräume gelüftet werden müssen. Eine hohe CO2-Konzentration in der Luft ist ein guter Hinweis für schlechte Lüftung, weswegen dieser Sensor Lüftungskonzepte überprüfen kann. Des Weiteren wollte ich meinen Chemielehrer daran hindern, eine überteuerte, proprietäre und funktionsarme Fertiglösung zu kaufen. Mein Sensor im speziellen wird in den Chemieräumen ihren Einsatz finden.

### Beschreibung
Eine ESP32 Nodemcu liest in gewissen Intervallen (Standard: 5 Sekunden) die Messdaten verschiedenster Sensoren aus, zeigt diese auf einen Display an und schickt sie per MQTT zu einem Grafana Server.

![Side3](https://user-images.githubusercontent.com/70963316/117326839-56398300-ae92-11eb-8ba9-c78fc04254be.jpg)

![Front3](https://user-images.githubusercontent.com/70963316/117326867-5c2f6400-ae92-11eb-9242-9dcd53b409c1.jpg)

![Back2](https://user-images.githubusercontent.com/70963316/117326895-62bddb80-ae92-11eb-9587-686052b05009.jpg)

### Key-Features / Todo-Liste
- [x] Einbindung des MH-Z19B Sensors (CO2, Temperatur)
- [x] Einbindung des CCS811 Sensors (eCO2, TVOC, Temperatur)
- [x] Einbindung des HDC1080 Sensors (Temperatur, Luftfeuchtigkeit)
- [x] Senden der Messdaten über MQTT und InfluxDB zu einem Grafana-Server (gehostet auf einen Raspberry Pi)
- [x] Anzeigen der (relevanten) Messdaten auf einem SSD1331 OLED Display
- [x] Debug beim Startprozess sowohl über seriellen Monitor als auch über den SSD1331 OLED Display + Countdown für Wartezeiten
- [x] Automatischer Standalone-Betrieb ohne WLAN- oder MQTT-Verbindung inklusive Status-LED für MQTT-Verbindung
- [x] Automatische Reconnects in bestimmten Intervalls im Standalone-Betreib
- [x] SSD1331 Bootscreen
- [x] Anzeigen der Messdaten mithilfe des seriellen Plotters der Arduino IDE
- [x] Gehäuse
- [x] Möglichkeit zum Kalibrieren/Einstellen von Offsets

### Referenz
Einige ähnliche Projekte haben mir sehr geholfen und mich inspiriert. Von Voltlog habe ich Code (hauptsächlich für die MQTT-Verbindung) und von Sasul habe ich das Gehäuse übernommen.
- https://github.com/voltlog/CO2
- https://github.com/Sasul/Arduino-CO2-Meter

### Verwendete Bibliotheken
Eine Liste aller verwendeten externen Bibliotheken mit zugehörigen Links. Eine ZIP-Datei dieser Bibliotheken ist im entsprechenden Ordner hinterlegt, für den Fall dass zukünftige Versionen nicht mehr funktionieren oder diese Bibliotheken in Zukunft nicht mehr existieren sollten.
- PubSubClient: Für die MQTT-Verbindung - https://github.com/knolleary/pubsubclient
- ClosedCube_HDC1080: Für HDC1080 Sensor - https://github.com/closedcube/ClosedCube_HDC1080_Arduino
- MHZ19: Für MH-Z19B Sensor - https://github.com/WifWaf/MH-Z19
- SSD_13XX: Für SSD1331 Display - https://github.com/sumotoy/SSD_13XX
- Adafruit_CCS811: Für CCS811 Sensor - https://github.com/adafruit/Adafruit_CCS811
- Die Standard WLAN-Bibliothek des ESP32
