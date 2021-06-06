# table of content

<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Deutsch](#deutsch)
	- [Kurzbeschreibung](#kurzbeschreibung)
	- [Aufbau und Funktionsweise](#aufbau-und-funktionsweise)
	- [Key-Features / Todo-Liste](#key-features-todo-liste)
	- [Anleitung](#anleitung)
		- [Konfiguration der Messstation](#konfiguration-der-messstation)
		- [Konfiguration des Grafana-&InfluxDB-&MQTT-Servers](#konfiguration-des-grafana-influxdb-mqtt-servers)
	- [Referenz](#referenz)
	- [Verwendete Bibliotheken](#verwendete-bibliotheken)

<!-- /TOC -->

# Deutsch
## Kurzbeschreibung
Dies ist ein CO2- und Luftqualitätssensor, den ich während der Qualifikationsphase in der Schule gebaut habe. Hintergrund ist u.a. die Covid-19 Pandemie, da wegen dieser in Schulen die Klassenräume gelüftet werden müssen. Eine hohe CO2-Konzentration in der Luft ist ein guter Hinweis für schlechte Lüftung, weswegen dieser Sensor Lüftungskonzepte überprüfen kann. Des Weiteren wollte ich meinen Chemielehrer daran hindern, eine überteuerte, proprietäre und funktionsarme Fertiglösung zu kaufen. Mein Sensor im speziellen wird in den Chemieräumen ihren Einsatz finden.

## Aufbau und Funktionsweise
Eine ESP32 Nodemcu liest in gewissen Intervallen (Standard: 5 Sekunden) die Messdaten verschiedenster Sensoren aus, zeigt diese auf einen Display an und schickt sie per MQTT zu einem Server, der diese in eine InfluxDB Datenbank speichert und per Grafana visualisiert. Alternativ können die Messdaten auch mithilfe des seriellen Plotters der Arduino IDE visualisiert werden. Die drei Anzeigemöglichkeiten (Display, Grafana, serieller Plotter) sind unabhängig voneinander möglich, man kann also entweder nur eine, zwei oder alle drei Möglichkeiten parallel nutzen. Der Grafana-Server ist also nicht Pflicht!

![Side3](https://user-images.githubusercontent.com/70963316/117326839-56398300-ae92-11eb-8ba9-c78fc04254be.jpg)

![Front3](https://user-images.githubusercontent.com/70963316/117326867-5c2f6400-ae92-11eb-9242-9dcd53b409c1.jpg)

![Back2](https://user-images.githubusercontent.com/70963316/117326895-62bddb80-ae92-11eb-9587-686052b05009.jpg)

## Key-Features / Todo-Liste
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

## Anleitung
### Konfiguration der Messstation
[Dies ist das Script für die Messstation.](https://github.com/JulianFP/CO2-Sensor/blob/main/CO2-Sensor.ino) Lade es herunter und öffne es mit der Arduino IDE. Stelle sicher, dass alle benötigten Bibliotheken installiert sind (siehe [Verwendete Bibliotheken](#verwendete-bibliotheken)) und dass als Board der ESP32 eingestellt ist.
Der Konfigurationsbereich beginnt mit "!!!Beginn Konfigurationsbereich!!!" und endet auch entsprechend. Hier können die entsprechenden Werte nach belieben verändert werden, dies sollte aufgrund der Beschreibungen im Script selbst recht selbsterklärend sein. Wichtig ist das Einstellen der Offsets sowie (falls ein Server genutzt wird) das Einstellen der WLAN-Anmeldedaten und der Verbindung mit den MQTT-Server.
Nun kann die Sensorstation per USB mit den PC verbunden und das Programm geflasht werden. Es gibt im Internet viele Anfängertutorials zum Umgang mit der Arduino IDE, die zur Hilfe hinzugezogen werden können.

### Konfiguration des Grafana-&InfluxDB-&MQTT-Servers
Dieser Server kann grundsätzlich auf jeden PC installiert werden, auf den Linux läuft. Diese Anleitung ist allerdings für einen Linux-PC ausgelegt, der auf Basis von Ubuntu läuft (was meistens der Fall ist). Besonders empfehlenswert für diese Anwendung ist ein Raspberry Pi, den es schon sehr günstig zu haben gibt und der problemlos 24/7 laufen kann (diese Anleitung funktioniert auf jeden Fall auf Raspberry OS).
1. `sudo -i` -- Anmelden als root. Achtung: Als root kann man sehr viel Unsinn anstellen, erst denken, dann tippen!
2. `apt update && apt upgrade -y` -- Aktualisieren der Paketquellen und installieren von Updates
3. `apt install python3 python3-pip mosquito mosquito-client influxdb influxdb-client grafana -y` -- Installieren aller notwendiger Pakete, falls diese nicht schon vorhanden sind
4. `nano /etc/influxdb/influxdb.conf` -- Öffnen der InfluxDB-Konfigurationsdatei
5. Scrolle mit den Pfeiltasten bis zum Bereich, der mit [http] beginnt und entferne das Hashtag vor "enabled=true". Verlasse den Texteditor mit "Strg+X", speichere die Änderungen dabei mit "Y" und dann "Enter".
6. `systemctl enable influxdb && systemctl start influxdb` -- Starten von InfluxDB und hinzufügen zum Autostart
7. `influxdb` -- Öffnen der InfluxDB-Konsole
8. `CREATE DATABASE iot`-- Erstellen einer InfluxDB-Datenbank mit den Namen "iot"
9. `CREATE USER grafana WITH PASSWORD '>Dein Passwort<'` -- Erstellen eines InfluxDB-Nutzers mit den Namen "grafana". Setze für >Dein Password< ein sicheres Passwort ein!
10. `GRANT ALL ON iot TO grafana` -- Zuweisen der Rechte auf die Datenbank zu den Nutzer
11. `exit` -- Verlassen der InfluxDB-Konsole
12. `pip3 install paho-mqtt influxdb` -- Installiere einige Python-Bibliotheken für das Bridge-Script
13. `cd` -- Sicherstellen, dass wird uns im /root Ordner befinden
14. `wget https://github.com/JulianFP/CO2-Sensor/blob/main/MQTTInfluxDBBridge.py` -- Herunterladen des Bridge-Scripts.
15. Öffne das Bridge-Script mit `nano MQTTInfluxDBBridge.py` und ersetze >Dein Password< mit deinem vorher definierten Passwort. Alles andere habe ich bereits vorkonfiguriert. Natürlich kann diese bei Bedarf mithilfe von nano noch verändert werden, z.B. wenn man mehrere Messstationen mit einen Server nutzen möchte.
16. Teste das Script mit `python3 MQTTInfluxDBBridge.py`. Falls keine Fehlermeldung erscheint, funktioniert es. Beende das Script mit "Strg+C" nun wieder.
17. `nano /etc/systemd/system/bridge.service` -- Erstellen und öffnen einer .service Datei, um das Bridge-Script ständig im Hintegrund ausführen zu können
18. Kopiere und füge mithilfe von "Strg+Shift+V" folgendes in die Datei ein. Verlasse anschließend den Texteditor mit "Strg+X", speichere die Änderungen dabei mit "Y" und dann "Enter".
```
[Unit]
Description=MySQL_to_Grafana_bridge
After=network.target
[Service]
ExecStart=/usr/bin/python3 -u MQTTInfluxDBBridge.py
WorkingDirectory=/root
StandardOutput=inherit
StandardError=inherit
Restart=always
User=root
[Install]
WantedBy=multi-user.target
```
19. `systemctl enable bridge.service && systemctl start bridge.service` -- Starten des Services und hinzufügen zum Autostart
20. `systemctl enable grafana && systemctl start grafana` -- Starten von InfluxDB und hinzufügen zum Autostart
21. Der Grafanaserver kann nun im Browser unter >ip-adresse_des_servers<:3000 aufgerufen werden. Füge als "Data Source" InfluxDB mit den Datenbanknamen "iot" und dem Nutzer "grafana" hinzu.
22. _Optional:_ Wenn man ohne Login auf die Daten auf den Grafanaserver zugreifen soll, dann öffne mit `nano /etc/grafana/grafana.ini` die Konfigdatei von Grafana und entferne das ";" vor "enabled=true" unter "Anonymous Auth" (du kannst mithilfe von "Strg+W suchen"). Definiere außerdem "org_name = Main Org." und "org_role = Viewer". Hiernach muss grafana mit `systemctl restart grafana` neugestartet werden
23. _Optional:_ Definiere in derselben Konfigdate unter "Server" "protocol" zu "https", "http_port" zu "443" und "cert_key" und "cert_file" zu den Speicherort des SSL-Zertifikats. Bei der Verwendung eines Reverse-Proxy muss nur "http_port" zu "80" und "root_url" zur verwendeten URL geändert werden. Hiernach muss grafana mit `systemctl restart grafana` neugestartet werden

## Referenz
Einige ähnliche Projekte haben mir sehr geholfen und mich inspiriert. Von Voltlog habe ich Code (hauptsächlich für die MQTT-Verbindung) und von Sasul habe ich das Gehäuse übernommen. Der dritte Link führt zu einer Anleitung zur Einrichtung des Servers, von dem ich auch das Python-Script habe. Dieses unterliegt somit natürlich nicht meiner AGPL-Lizenz!
- https://github.com/voltlog/CO2
- https://github.com/Sasul/Arduino-CO2-Meter
- https://diyi0t.com/visualize-mqtt-data-with-influxdb-and-grafana/

## Verwendete Bibliotheken
Eine Liste aller verwendeten externen Bibliotheken mit zugehörigen Links. Eine ZIP-Datei dieser Bibliotheken ist im entsprechenden Ordner hinterlegt, für den Fall dass zukünftige Versionen nicht mehr funktionieren oder diese Bibliotheken in Zukunft nicht mehr existieren sollten.
- PubSubClient: Für die MQTT-Verbindung - https://github.com/knolleary/pubsubclient
- ClosedCube_HDC1080: Für HDC1080 Sensor - https://github.com/closedcube/ClosedCube_HDC1080_Arduino
- MHZ19: Für MH-Z19B Sensor - https://github.com/WifWaf/MH-Z19
- SSD_13XX: Für SSD1331 Display - https://github.com/sumotoy/SSD_13XX
- Adafruit_CCS811: Für CCS811 Sensor - https://github.com/adafruit/Adafruit_CCS811
- Die Standard WLAN-Bibliothek des ESP32
