# Inhaltsverzeichnis | table of content

<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Inhaltsverzeichnis | table of content](#inhaltsverzeichnis-table-of-content)
- [Deutsch](#deutsch)
	- [Kurzbeschreibung](#kurzbeschreibung)
	- [Aufbau und Funktionsweise](#aufbau-und-funktionsweise)
	- [Key-Features / Todo-Liste](#key-features-todo-liste)
	- [Anleitung](#anleitung)
		- [Einrichtung der Arduino IDE](#einrichtung-der-arduino-ide)
		- [Konfiguration der Messstation](#konfiguration-der-messstation)
		- [Serielle Verbindung und Plotter](#serielle-verbindung-un-plotter)
		- [Konfiguration des Grafana-&InfluxDB-&MQTT-Servers](#konfiguration-des-grafana-influxdb-mqtt-servers)
	- [Referenz](#referenz)
	- [Verwendete Bibliotheken](#verwendete-bibliotheken)
- [English](#english)
	- [brief description](#brief-description)
	- [structure and functionalities](#structure-and-functionalities)
	- [key features / to-do list](#key-features-to-do-liste)
	- [guide](#guide)
		- [setup of the Arduino IDE](#setup-of-the-arduino-ide)
		- [configuration of the measuring station](#configuration-of-the-measuring-station)
		- [serial connection and plotter](#serial-connection-and-plotter)
		- [configuration of the Grafana-&InfluxDB-&MQTT-servers](#configuration-of-the-grafana-influxdb-mqtt-servers)
	- [reference](#reference)
	- [used libraries](#used-libraries)

<!-- /TOC -->

# Deutsch
## Kurzbeschreibung
Dies ist ein CO2- und Luftqualitätssensor, den ich während der Qualifikationsphase in der Schule gebaut habe. Hintergrund ist u.a. die Covid-19 Pandemie, da wegen dieser in Schulen die Klassenräume gelüftet werden müssen. Eine hohe CO2-Konzentration in der Luft ist ein guter Hinweis für schlechte Lüftung, weswegen dieser Sensor Lüftungskonzepte überprüfen kann. Des Weiteren wollte ich meinen Chemielehrer daran hindern, eine überteuerte, proprietäre und funktionsarme Fertiglösung zu kaufen. Mein Sensor im speziellen wird in den Chemieräumen ihren Einsatz finden.

## Aufbau und Funktionsweise
Eine ESP32 Nodemcu liest in gewissen Intervallen (Standard: 5 Sekunden) die Messdaten verschiedenster Sensoren aus, zeigt diese auf einen Display an und schickt sie per MQTT zu einem Server, der diese in eine InfluxDB Datenbank speichert und per Grafana visualisiert. Alternativ können die Messdaten auch mithilfe des seriellen Plotters der Arduino IDE visualisiert werden. Die drei Anzeigemöglichkeiten (Display, Grafana, serieller Plotter) sind unabhängig voneinander möglich, man kann also entweder nur eine, zwei oder alle drei Möglichkeiten parallel nutzen. Der Grafana-Server ist also nicht Pflicht! Weitere Infomartionen zum Schaltkreis ist dieser im Ordner 'Schematics' hinterlegt.

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
### Einrichtung der Arduino IDE
Dies ist eine Anleitung zur Einrichtung und Modifizierung des Sensors an sich. Für die Anleitung zur Einrichtung der Servers siehe [hier](#konfiguration-des-grafana-influxdb-mqtt-servers).
1. Installiere die Arduino IDE [(Windows|Mac|Linux)](https://www.arduino.cc/en/software). *Hinweis Linux:* Installation per Package-Manager sinnvoll (z.B. apt bei Debian/Ubuntu)
2. *Nur Linux:* Installiere die Bibliothek 'esptool' mit pip: `sudo pip3 install pyserial esptool` (bzw. ohne die 3, wenn nur Python 2.x installiert ist)
3. *Nur Linux:* Hinzufügen des Nutzers zur USB-Usergruppe mit folgenden Terminal-Befehl: `sudo usermod -a -G dialout $USER` (Debian/Ubuntu) oder `sudo usermod -a -G uucp $USER` (Arch)
4. Öffne die Arduino IDE. Klicke auf 'Datei' --> 'Voreinstellungen' und füge folgenden Link im Feld 'Zusätzliche Boardverwalter-URLs' ein und klicke "OK": `https://dl.espressif.com/dl/package_esp32_index.json`
5. Gehe auf 'Werkzeuge' --> 'Board: "..."' --> 'Boardverwalter...', suche nach 'esp32' und lade das entsprechende Board herunter
6. Gehe auf 'Werkzeuge' --> 'Bibliotheken verwalten...' und installiere folgende Bibliotheken (in die Suche eingeben und das richtige Ergebnis installieren): PubSubClient von Nick O'Leary, ClosedCube HDC1080 von ClosedCube, MH-Z19 von Jonathan Dempsey, Adafruit CCS811 Library von Adafruit.
7. Lade über [diesen Link](https://github.com/sumotoy/SSD_13XX/archive/refs/tags/1.0.zip) die SSD_13XX Bibliothek herunter. Gehe in der Arduino IDE dann auf 'Sketch' --> 'Bibliothek einbinden' --> '.ZIP-Bibliothek hinzufügen...', wähle die soeben heruntergeladene Datei aus und drücke 'OK' bzw. 'Öffnen'. Mehr Informationen siehe [Verwendete Bibliotheken](#verwendete-bibliotheken).
8. Verbinde den Sensor per USB mit den Computer und gehe dann auf 'Werkzeuge', wähle als 'Board' den 'ESP32 Dev Module' aus, als 'Upload Speed' '115200' und als 'Port' den entsprechenden USB-Port, an dem der Sensor angeschlossen ist. Die Arduino IDE ist nun fertig installiert, im folgenden kann nun das Script geöffnet, modifiziert, kompiliert und hochgeladen werden.

### Konfiguration der Messstation
1. Gehe auf 'Datei' --> 'Öffnen...' und wähle [dieses Script für die Messstation aus.](https://github.com/JulianFP/CO2-Sensor/blob/main/CO2-Sensor.ino).
2. Konfiguriere das Script nach eigenen Belieben. Der entsprechende Bereich ist mit /***!!!Beginn Konfigurationsbereich!!!***/ markiert und die Einstellungen sind alle erklärt. Besonders interessant sind die Einstellungen für den Offset, die WLAN-Anmeldedaten sowie die Einstellungen, um eine Verbindung zum MQTT-Server aufzubauen. Hierfür muss dieser evtl. [vorher eingerichtet sein](#konfiguration-des-grafana-influxdb-mqtt-servers)
3. Wenn das Script fertig konfiguriert ist, kann es mit den Häkchen links oben kompiliert, oder mit den Pfeil rechts daneben kompiliert und auf den Sensor hochgeladen werden.

### Serielle Verbindung und Plotter
- *Für Probleme oder mehr Informationen zum Status des Programms:* Gehe rechts oben auf das Symbol 'Serieller Monitor' und wähle die korrekte Baudrate von '115200' aus. Nun werden hier alle Statusmeldungen des Sensors und eventuelle Fehler angezeigt (vieles hiervon ist auch auf den Display zu sehen, für den Fall das irgendwas nicht funktioniert, kann jedoch hier nachgesehen werden). Wenn man alle Meldungen vom Start an sehen möchte, dann steckt man den Sensor noch einmal raus und wieder rein.
- *Serieller Plotter:* Wenn die Daten über den seriellen Plotter der Arduino IDE visualisiert werden sollen, gehe auf 'Werkzeuge' --> 'Serieller Plotter' und wähle wieder die Baudrate von '115200' aus. Der Sensor muss natürlich angeschlossen sein.

### Konfiguration des Grafana-&InfluxDB-&MQTT-Servers
Dieser Server kann grundsätzlich auf jeden PC installiert werden, auf den Linux läuft. Diese Anleitung ist allerdings für einen Linux-PC ausgelegt, der auf Basis von Ubuntu läuft (was meistens der Fall ist). Besonders empfehlenswert für diese Anwendung ist ein Raspberry Pi, den es schon sehr günstig zu haben gibt und der problemlos 24/7 laufen kann (diese Anleitung funktioniert auf jeden Fall auf Raspberry OS).
1. `sudo -i` -- Anmelden als root. Achtung: Als root kann man sehr viel Unsinn anstellen, erst denken, dann tippen!
2. `apt update && apt upgrade -y` -- Aktualisieren der Paketquellen und installieren von Updates
3. `apt install python3 python3-pip mosquitto influxdb influxdb-client apt-transport-https software-properties-common wget -y` -- Installieren aller notwendiger Pakete, falls diese nicht schon vorhanden sind
4. `nano /etc/influxdb/influxdb.conf` -- Öffnen der InfluxDB-Konfigurationsdatei
5. Scrolle mit den Pfeiltasten bis zum Bereich, der mit [http] beginnt und entferne das Hashtag vor "enabled=true". Verlasse den Texteditor mit "Strg+X", speichere die Änderungen dabei mit "Y" und dann "Enter".
6. `systemctl enable influxdb && systemctl start influxdb` -- Starten von InfluxDB und hinzufügen zum Autostart
7. `influx` -- Öffnen der InfluxDB-Konsole
8. `CREATE DATABASE iot`-- Erstellen einer InfluxDB-Datenbank mit den Namen "iot"
9. `CREATE USER grafana WITH PASSWORD '>Dein Passwort<'` -- Erstellen eines InfluxDB-Nutzers mit den Namen "grafana". Setze für >Dein Password< ein sicheres Passwort ein!
10. `GRANT ALL ON iot TO grafana` -- Zuweisen der Rechte auf die Datenbank zu den Nutzer
11. `exit` -- Verlassen der InfluxDB-Konsole
12. `pip3 install paho-mqtt influxdb` -- Installiere einige Python-Bibliotheken für das Bridge-Script
13. `cd` -- Sicherstellen, dass wird uns im /root Ordner befinden
14. `wget https://raw.githubusercontent.com/JulianFP/CO2-Sensor/main/MQTTInfluxDBBridge.py` -- Herunterladen des Bridge-Scripts.
15. Öffne das Bridge-Script mit `nano MQTTInfluxDBBridge.py` und ersetze >Dein Password< mit deinem vorher definierten Passwort. Alles andere habe ich bereits vorkonfiguriert. Natürlich kann diese bei Bedarf mithilfe von nano noch verändert werden, z.B. wenn man mehrere Messstationen mit einen Server nutzen möchte. Verlasse anschließend den Texteditor mit "Strg+X", speichere die Änderungen dabei mit "Y" und dann "Enter".
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
20. `wget -q -O - https://packages.grafana.com/gpg.key | apt-key add -` -- Hinzufügen des Public-Keys des Grafana-Repositorys
21. `echo "deb https://packages.grafana.com/oss/deb stable main" | tee -a /etc/apt/sources.list.d/grafana.list` -- Hinzufügen des Grafana-Repositorys
22. `apt update && apt install grafana` -- Installation von Grafana
23. `systemctl enable grafana-server && systemctl start grafana-server` -- Starten von Grafana und hinzufügen zum Autostart
24. Der Grafanaserver kann nun im Browser unter >ip-adresse_des_servers<:3000 aufgerufen werden (die IP-Adresse des Servers kann mithilfe des Befehls `ip addr` herausgefunden werden). Der Standard-Login ist "admin" für sowohl Nutzername als auch Passwort. Nachdem du das Passwort geändert hast, gehe links in der Leiste auf "Configuration" (das Zahnrad) und klicke unter den Reiter "Data Sources" auf "Add data source". Wähle hier "InfluxDB", gebe folgende Informationen ein (ersetze >Dein Passwort< wieder durch dein Passwort) und drücke auf "Save&Test".
```
URL -- localhost:8086
Database -- iot
User -- grafana
Password -- >Dein Passwort<
```
25. Um nun die Daten zu visualisieren, kannst du links ein neues Dashboard erstellen und hier ein neues Panel erstellen. Bei "FROM" kannst du die entsprechende Datenreihe auswählen. Diese wird allerdings erst angezeigt, wenn die Messstation bereits Daten zum Server gesendet hat.
26. _Optional:_ Wenn man ohne Login auf die Daten auf den Grafanaserver zugreifen soll, dann öffne mit `nano /etc/grafana/grafana.ini` die Konfigdatei von Grafana und entferne das ";" vor "enabled=true" unter "Anonymous Auth" (du kannst mithilfe von "Strg+W suchen"). Definiere außerdem "org_name = Main Org." und "org_role = Viewer". Hiernach muss grafana mit `systemctl restart grafana-server` neugestartet werden
27. _Optional:_ Definiere in derselben Konfigdate unter "Server" "protocol" zu "https", "http_port" zu "443" und "cert_key" und "cert_file" zu den Speicherort des SSL-Zertifikats. Bei der Verwendung eines Reverse-Proxy muss nur "http_port" zu "80" und "root_url" zur verwendeten URL geändert werden. Hiernach muss grafana mit `systemctl restart grafana-server` neugestartet werden

## Referenz
Einige ähnliche Projekte haben mir sehr geholfen und mich inspiriert. Von Voltlog habe ich Code (hauptsächlich für die MQTT-Verbindung) und von Sasul habe ich das Gehäuse übernommen. Der dritte Link führt zu einer Anleitung zur Einrichtung des Servers, von dem ich auch das Python-Script habe. Dieses unterliegt somit natürlich nicht meiner AGPL-Lizenz!
- https://github.com/voltlog/CO2
- https://github.com/Sasul/Arduino-CO2-Meter
- https://diyi0t.com/visualize-mqtt-data-with-influxdb-and-grafana/

## Verwendete Bibliotheken
Eine Liste aller verwendeten externen Bibliotheken mit zugehörigen Links. Eine ZIP-Datei dieser Bibliotheken ist im entsprechenden Ordner hinterlegt, für den Fall dass zukünftige Versionen nicht mehr funktionieren oder diese Bibliotheken in Zukunft nicht mehr existieren sollten. Diese Bibliotheken unterliegen somit auch nicht meiner AGPL-Lizenz!
- PubSubClient: Für die MQTT-Verbindung - https://github.com/knolleary/pubsubclient
- ClosedCube_HDC1080: Für HDC1080 Sensor - https://github.com/closedcube/ClosedCube_HDC1080_Arduino
- MHZ19: Für MH-Z19B Sensor - https://github.com/WifWaf/MH-Z19
- SSD_13XX: Für SSD1331 Display - https://github.com/sumotoy/SSD_13XX
- Adafruit_CCS811: Für CCS811 Sensor - https://github.com/adafruit/Adafruit_CCS811
- Die Standard WLAN-Bibliothek des ESP32

# English
## brief description
This is a CO2- and air-quality-sensor which I built during upper school. One possible application of this project is fighting the COVID-19 pandemic at schools: THe CO2-concentration is a good indicator for the general air quality in a room and can therefore be used to determine if the room should be ventilad. Additionally I wanted to prevent my chemistry teacher to buy an overcharged, proprietary und featurepoor ready-to-use solution. My sensor will get used at the chemistry rooms of my school.

## structure and functionalities
An ESP32 Nodemcu reads in a certain intervall (default: 5 seconds) the readings of several sensors and shows them on a display and sends them over MQTT to an server which stores them in an InfluxDB database and visualizes them with Grafana. Alternatively the readings can be visualized with the serial plotter of the Arduino IDE. The three display options (display, Grafana, serial plotter) are independet from each other, so you can use one, two or three possibilities in parallel. The Grafana server is therefore optional! For more information about the schematics look into the folder 'Schematics'.

![Side3](https://user-images.githubusercontent.com/70963316/117326839-56398300-ae92-11eb-8ba9-c78fc04254be.jpg)

![Front3](https://user-images.githubusercontent.com/70963316/117326867-5c2f6400-ae92-11eb-9242-9dcd53b409c1.jpg)

![Back2](https://user-images.githubusercontent.com/70963316/117326895-62bddb80-ae92-11eb-9587-686052b05009.jpg)

## key features / to-do list
- [x] Integration of the MH-Z19B sensor (CO2, temperature)
- [x] Integration of the CCS811 sensor (eCO2, TVOC, temperature)
- [x] Integration of the HDC1080 sensor (temperature, air humidity)
- [x] Sending of the readings over MQTT and InfluxDB to a Grafana server (hostet on a Raspberry Pi)
- [x] Displaying the (relevant) readings on a SSD1331 OLED display
- [x] Displaying debug information over the serial monitor as well as the SSD1331 OLED display + countdown for waiting/loading times
- [x] Automatic standalone mode without Wifi- or MWTT-connection and status LED for MQTT-connection
- [x] Automatic reconnects in certain intervalls in standalone mode
- [x] SSD1331 bootscreen
- [x] Displaying the readings with the serial plotter of the Arduino IDE
- [x] case
- [x] possibility to calibrate/set a offset

## guide
### setup of the Arduino IDE
This is a guide for the setup and modification of the measuring station. You find the guide for the setup of the server [here](#konfiguration-des-grafana-influxdb-mqtt-servers).
1. Install the Arduino IDE [(Windows|Mac|Linux)](https://www.arduino.cc/en/software). *hint for Linux users:* Installation over the package manager makes sense (e.g. apt at Debian/Ubuntu bases systems) 
2. *Linux only:* Install the library 'esptool' with pip: `sudo pip3 install pyserial esptool` (or without the 3 if you are still using Python 2.x)
3. *Linux only:* Add your user to the USB-usergroup with the following command: `sudo usermod -a -G dialout $USER` (Debian/Ubuntu) or `sudo usermod -a -G uucp $USER` (Arch)
4. Open the Arduino IDE. Click on 'File' --> 'Preferences' and add the following link to the field labeled 'Additional Boards Manager URLs' and click 'OK': `https://dl.espressif.com/dl/package_esp32_index.json`
5. Go to 'Tools' --> 'Board: "..."' --> 'Boards Manager...', search for 'esp32' and install this board.
6. Go to 'Tools' --> 'Manage Libraries...' and install the following libraries (enter the names in the search fields and install the correct results): PubSubClient by Nick O'Leary, ClosedCube HDC1080 by ClosedCube, MH-Z19 by Jonathan Dempsey, Adafruit CCS811 Library by Adafruit.
7. Download over [this link](https://github.com/sumotoy/SSD_13XX/archive/refs/tags/1.0.zip) the SSD_13XX library. Go in the Arduino IDE to 'Sketch' --> 'Include library' --> 'Add .ZIP Library...', choose the just downloaded file and press 'OK' or 'Open'. For more information got to [Used libraries](#used-libraries).
8. Connect the measuring station per USB with the computer and go to 'Tools', choose the 'ESP32 Dev Module' as 'Board', '115200' as 'Upload Speed' and the appropriate USB-Port as 'Port'. The Arduino IDE is now ready to go, in the following you can open the Script, modify, compile and upload it.

### configuration of the measuring station
1. Go to 'File' --> 'Open...' and choose [this Script for the measuring station.](https://github.com/JulianFP/CO2-Sensor/blob/main/CO2-Sensor_english-version.ino).
2. Configure the script as you desire. The appropriate section begins with /***!!!start configuration area!!!***/ and all settings are explained. Especially interesting is the setting for the offsets, the Wifi-information as well as the settings necessary for the connection to the MQTT-server. Therefore a [completed setup of this server](#configuration-of-the-grafana-influxdb-mqtt-servers) may be necessary.
3. If the configuration of the script is finished, it can be compiled by clicking on the checkmark at the left top or it can be compiled and uploaded to the measuring station by clicking on the arrow next to the checkmark.

### serial connection and plotter
- *for issues or more information about the status of the program:* Click on the symbol at the right top labeled 'serial monitor' and choose the correct baudrate of '115200'. All status notifications of the sensors and possible issues are displayed here (many of them are also visible on the built-in display, but if anything isn't working, this is the place to debug the program) If you want to see all notifications since the begin of the program, plug out and plug in the sensor again.
- *serial plotter:* If you want to visualize the readings with the serial plotter of the Arduino IDE, go to 'Tools' --> 'serial plotter' and choose once again the correct baudrate of '115200'. The sensor has to be plugged in.

### configuration of the Grafana-&InfluxDB-&MQTT-servers
This server can be installed basically on every Linux PC. However, this guide is made for an Ubuntu-based operating system (which is the case most of the time). Especially I recommend for this a Raspberry Pi which is quiet cheap and can run 24/7 without any problems (this guide works certainly on Raspberry OS).
1. `sudo -i` -- login as root. Warning: You can break many things as root, think first, type next!
2. `apt update && apt upgrade -y` -- Update the package sources and install all updates
3. `apt install python3 python3-pip mosquitto influxdb influxdb-client apt-transport-https software-properties-common wget -y` -- Install all necessary packages, if they aren't installed yet
4. `nano /etc/influxdb/influxdb.conf` -- Open the InfluxDB configuration file
5. Scroll down with the arrow keys to the area which begins with [http] and remove the hashtag before 'enabled=true'. Leave the text editor with 'Ctrl+X', store the changes by pressing 'Y' and then 'Enter'.
6. `systemctl enable influxdb && systemctl start influxdb` -- Start InfluxDB and add it to autostart
7. `influx` -- Open the InfluxDB console
8. `CREATE DATABASE iot`-- Create an InfluxDB database named 'iot'
9. `CREATE USER grafana WITH PASSWORD '>your password<'` -- Create an InfluxDB user nammed 'grafana'. Replace >your password< with a save password!
10. `GRANT ALL ON iot TO grafana` -- grant all rights of the database 'iot' to the user 'grafana'
11. `exit` -- exit the InfluxDB console
12. `pip3 install paho-mqtt influxdb` -- install several python libaries which are necessary for the bridge script
13. `cd` -- make sure, that you are in the /root folder
14. `wget https://raw.githubusercontent.com/JulianFP/CO2-Sensor/main/MQTTInfluxDBBridge.py` -- Download the Bridge script
15. Open the Bridge script with `nano MQTTInfluxDBBridge.py` and replace >your password< with your previously defined password. Everything else is preconfigured by me. Of course you can change this anyway, e.g. if you want to use multiple measuring stations with the same server. After that exit the texteditor with 'Ctrl+X', store the changes with 'Y' and then 'Enter'.
16. Try out the script with `python3 MQTTInfluxDBBridge.py`. If there isn't any error message, it's working. End the script then with 'Ctrl+C'.
17. `nano /etc/systemd/system/bridge.service` -- Create and open a .service file which executes the bridge script in the background.
18. Copy and paste the following by pressing 'Ctrl+Shift+V' in this file. After that leave the texteditor again by pressing 'Ctrl+X' and store the chagnes by pressing 'Y' and then 'Enter'.
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
19. `systemctl enable bridge.service && systemctl start bridge.service` -- Start the service and add it to autostart
20. `wget -q -O - https://packages.grafana.com/gpg.key | apt-key add -` -- Add the Public-Keys of the Grafana-repository
21. `echo "deb https://packages.grafana.com/oss/deb stable main" | tee -a /etc/apt/sources.list.d/grafana.list` -- Add the Grafana-repository
22. `apt update && apt install grafana` -- Install Grafana
23. `systemctl enable grafana-server && systemctl start grafana-server` -- Start Grafana and add it to autostart
24. The Grafana server is now accessable over the Browser under the address >ip-adresse_des_servers<:3000 (You can find out the ip address of your server with the command `ip addr`). The default login is 'admin' for both username and password. After you changed the password, click on 'configuration' (the gear) at the panel at the left and click under 'data sources' at 'add data source'. Choose 'InfluxDB', enter the following information (by replacing >Your password> with your password again) and press 'Save&Test'.
```
URL -- localhost:8086
Database -- iot
User -- grafana
Password -- >Dein Passwort<
```
25. In order to visualize the data, you can create a new dashboard at the left and create there a new panel. At 'FROM' you can choose the desired data series. However, this will be first visible if the measuring station sent already data to the server.
26. _Optional:_ If you want to access the data at the grafana server without a logi, then open the grafana configuration file with `nano /etc/grafana/grafana.ini` and remove the ';' before 'enabled=true' under 'Anonymous Auth' (you can search for it with 'Ctrl+W'). Also define 'org_name = Main Org.' and 'org_role = Viewer'. After storing the changes you have to restart grafana with `systemctl restart grafana-server`.
27. _Optional:_ Define in the same configuration file unter 'server' 'protocol' to 'https', 'http_port' to '443' and 'cert_key_ and _cert_file_ to the storage location of your SSL-certificate. If you sue a reverse-proxy, you jsut have to change 'http_port' to '80' and 'root_url' to the used URL. AF
27. _Optional:_ Definiere in derselben Konfigdate unter "Server" "protocol" zu "https", "http_port" zu "443" und "cert_key" und "cert_file" zu den Speicherort des SSL-Zertifikats. Bei der Verwendung eines Reverse-Proxy muss nur "http_port" zu "80" und "root_url" zur verwendeten URL geändert werden. After storing the changes you have to restart grafana with `systemctl restart grafana-server`.

## reference
Some similar projects have helped and inspired me a lot. From Voltlog I used a bit of code (mainly for the mqtt-connection) and from Sasul I forked the case. The third link leads to a guide for the setup of the servers, from which I also used the python script. This thus isn't subject of my AGPL-license!
- https://github.com/voltlog/CO2
- https://github.com/Sasul/Arduino-CO2-Meter
- https://diyi0t.com/visualize-mqtt-data-with-influxdb-and-grafana/

## used libraries
A list of all used external libraries with the associated links. A ZIP-file of this libraries is in my folder at this github branch deposited, for the case if the future versions of this libraries aren't working or if they aren't existing anymore. Thus they are also not subject of my AGPL-license!
- PubSubClient: Für die MQTT-Verbindung - https://github.com/knolleary/pubsubclient
- ClosedCube_HDC1080: Für HDC1080 Sensor - https://github.com/closedcube/ClosedCube_HDC1080_Arduino
- MHZ19: Für MH-Z19B Sensor - https://github.com/WifWaf/MH-Z19
- SSD_13XX: Für SSD1331 Display - https://github.com/sumotoy/SSD_13XX
- Adafruit_CCS811: Für CCS811 Sensor - https://github.com/adafruit/Adafruit_CCS811
- Die Standard WLAN-Bibliothek des ESP32
