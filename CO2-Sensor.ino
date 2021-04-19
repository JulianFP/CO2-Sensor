//Bibliotheken hinzufügen
#include <PubSubClient.h>
#include <WiFi.h>
#include "ClosedCube_HDC1080.h"
#include "MHZ19.h"
#include <SPI.h>
#include <SSD_13XX.h> //https://github.com/sumotoy/SSD_13XX
#include "Adafruit_CCS811.h"

//Allgemeine Konfiguration
#define DATA_DELAY 5000    // Pause zwischen Messpunkten in Millisekunden, Standard: 5 Sekunden pro Messpunkt - Für den MHZ-19B Sensor ist dies der Minimalwert!
#define RECONNECT_DELAY 15 // Pause zwischen Versuchen, sich erneut mit dem WLAN-Netzwerk oder dem MQTT-Server zu verbinden, falls die Verbindung unterbrochen wurde (führt zu einem Einfrieren des Displays und erhöhten Netzwerkverkehr, deswegen sollte dieser Wert nicht zu niedrig gesetzt werden. Mögliche Lösung: Nutzung beider Kerne des ESP32: https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/)
#define alarm1 1000        // Grenze für ersten Alarm (gelbe Schrift)
#define alarm2 1500        // Grenze für ersten Alarm (orangene Schrift)     
#define alarm3 2000        // Grenze für ersten Alarm (rote Schrift)
#define RectMax 2500       // maximale CO2-Konzentration, die innerhalb des Balkens angezeigt wird
#define warmup true        // MH-Z19B muss 3 min. aufwärmen, hier angegeben in Mikrosekunden (siehe Datasheet https://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19b-co2-ver1_0.pdf). Achtung: Nur für Entwicklungszwecke auf false stellen!!!
#define LED 4              // GPIO-Pin der Status-LED

// WLAN Anmeldedaten && Konfiguration
const char* ssid = "WLAN-Name";
const char* password = "WLAN-Passwort";
const int timeout = 30; //in Sekunden

// MQTT Server Konfiguration
#define MQTT_SERVER "192.168.1.50"              //z.B. lokale IP des Raspberrys
#define MQTT_PORT 1883                          //Standard MQTT-Port: 1883
#define MQTT_USER ""                            //Standard: leer
#define MQTT_PASSWORD ""                        //Standard: leer
#define MQTT_CLIENT_ID "co2Node"                //Standard: co2Node
#define MQTT_DEVICE_TOPIC "iot/chemie/"         //Standard:iot/chemie
#define MQTT_TOPIC_STATE "iot/chemie/status"    //Standard: iot/chemie/status

// MQTT Client initialisieren
WiFiClient co2Node;
PubSubClient mqttClient(co2Node);

// Konfiguration des MH-Z19B Sensors
#define RX_PIN 16                                         // An diesen Pin ist der TX-Pin des MH-Z19B Sensors angeschlossen
#define TX_PIN 17                                          // An diesen Pin ist der RX-Pin des MH-Z19B Sensors angeschlossen
#define BAUDRATE 9600                                      // Baudrate des MH-Z19B Sensor, nicht ändern
MHZ19 myMHZ19;                                             // Constructor for MH-Z19 class
HardwareSerial mySerial(1);                               // Init UART for MHZ19

// Konfiguration des SSD1331 Displays
#define __CS    5                                          
#define __DC    26 
#define __RST   27
//define __mosi 23
//define __sclk 18
SSD_13XX display = SSD_13XX(__CS, __DC, __RST);

//Initialisiere CCS811 Sensor
Adafruit_CCS811 ccs;

//Initialisiere HDC1080 Sensor
ClosedCube_HDC1080 hdc1080;

// Benötigte Variablen
int64_t now;                   //zur Ausführung der DELAYs, jetziger Zeitpunkt wird hier gespeichert
int64_t lastMsgTime = 0;       //zur Ausführung des DATA_DELAYs, letzter Messzeitpunkt wird hier gespeichert
int64_t lastReconnectTime = 0; //zur Ausführung des RECONNECT_DELAYs, letzter Messzeitpunkt wird hier gespeichert
int mhz19_temp;                //Variable für Messausgabe
int mhz19_co2;                 //Variable für Messausgabe
float hdc1080_temp;            //Variable für Messausgabe
float hdc1080_humidity;        //Variable für Messausgabe
int ccs811_co2;                //Variable für Messausgabe
int ccs811_tvoc;               //Variable für Messausgabe
float ccs811_temp;             //Variable für Messausgabe
int color = BLUE;              //zur Anzeige vom mhz19_co2 auf den Display. Farbe ändert sich je nach CO2-Gehalt
int x;                         //zur Anzeige vom mhz19_co2 auf den Display. Die x-Position des Balkens ändert sich je nach CO2-Gehalt
int16_t cursor_pos[1];         //für die Countdowns zu Beginn (z.B. aufwärmen des MHZ-19B Sensors). Die Cursor-Position (x und y Wert) wird hier gespeichert

// Definition von Funktionen (diese werden nur in loop() benötigt, die Ersteinrichtung von WiFi und MQTT passiert in setup())    
void wifiReconnect() {                              //damit das Messgerät selbstständig ohne neuzustarten sich mit den WLAN-Netzwerk verbinden kann
  Serial.println("Verbinde mit " + String(ssid));
  WiFi.begin(ssid, password);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("verbunden - IP: ");
    Serial.println(WiFi.localIP());
  } 
}

void mqttReconnect() {                              //damit das Messgerät selbstständig ohne neuzustarten sich mit den MQTT-Server verbinden kann
    Serial.print("Verbinde mit " + String(MQTT_DEVICE_TOPIC) + " per MQTT...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("verbunden");

      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    }else {
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(mqttClient.state());
    }
}
void mqttPublish(char *topic, char *payload) {      //für das Senden von Informationen (z.B. Messdaten) zum MQTT-Server
  Serial.print("Publishing to MQTT: topic:");
  Serial.print(topic);
  Serial.print(" payload: ");
  Serial.println(payload);

  mqttClient.publish(topic, payload, true);
}

void setup() {
  //Start serielle Verbindung & Schriftzug
  Serial.begin(115200);
  Serial.println("CO2- & Luftqualitätssensor");

  //Start display-Kommunikation & Bootscreen
  display.begin();
  display.setTextScale(2);
  display.setTextColor(RED);
  display.setCursor(16,12);
  display.print("C");
  display.setTextColor(YELLOW);
  display.print("O");
  display.setTextColor(GREEN);
  display.setTextScale(1);
  display.setCursor(41,26);
  display.print("2");
  display.setTextColor(ORANGE);
  display.setTextScale(2);
  display.setCursor(51,12);
  display.print("-");
  display.setTextColor(GREEN);
  display.print(" &");
  display.setTextColor(BLUE);
  display.setTextScale(1);
  display.setCursor(3,42);
  display.println("Luftqualitaetssensor");
  delay(5000);
  display.clearScreen();
  
  //GPIO 2 als LED-Pin definieren
  pinMode(LED,OUTPUT);
  
  //MH-Z19 UART Verbindung einrichten
  mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN); 
  myMHZ19.begin(mySerial);                                
  myMHZ19.autoCalibration();    //MHZ-19B Automatische Kalibrierung ist aktiviert, siehe Examples der Library für manuelle Kalibrierung                        

  //Überprüfen, ob der CCS811 Sensor angeschlossen ist und kommuniziert
  if(!ccs.begin()){
    Serial.println("CCS811 Sensor nicht angeschlossen!");
    display.setTextColor(RED);
    display.setCursor(0,0);
    display.println("CCS811 Sensor nicht angeschlossen!");
    while(1);
  }

  //CCS811 Temperatur Kalibrierung
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

  //Überprüfen, ob der CCS811 Sensor angeschlossen ist und kommuniziert
  hdc1080.begin(0x40);

  //WiFi Ersteinrichtung - mit Timeout von 30 Sekunden
  Serial.println("Verbinde mit " + String(ssid) + " - Timeout in: ");
  display.setTextColor(CYAN);
  display.print("Verbinde mit " + String(ssid) + " - Timeout in: ");
  WiFi.begin(ssid, password);
  display.getCursor(cursor_pos[0],cursor_pos[1]);
  for(int i=timeout; i>=0; i--) { 
    display.setTextColor(CYAN);
    Serial.println(i);
    display.println(i);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("verbunden - IP: ");
      Serial.println(WiFi.localIP());
      display.setTextColor(GREEN);
      display.print("verbunden - IP: ");
      display.println(WiFi.localIP());
      delay(5000);
      break;
    }
    delay(1000);
    display.setCursor(cursor_pos[0],cursor_pos[1]);
    display.setTextColor(BLACK);
    display.print(i);
    display.setCursor(cursor_pos[0],cursor_pos[1]);   
  }
  if (WiFi.status() != WL_CONNECTED) {;
    display.setTextColor(RED);
    Serial.println("Verbindung fehlgeschlagen: Timeout");
    display.println("Verbindung fehlgeschlagen: Timeout");
    WiFi.disconnect();
    delay(10000);
  }

  //MQTT Ersteinrichtung - drei Versuche mit je 5 Sekunden Abstand
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  Serial.println("Verbinde mit " + String(MQTT_DEVICE_TOPIC) + " per MQTT...");
  display.clearScreen();
  display.setTextColor(CYAN);
  display.setCursor(0,0);
  display.println("Verbinde mit " + String(MQTT_DEVICE_TOPIC) + " per MQTT...");
  for(int i=1; i<=3; i++) {
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("verbunden");
      display.setTextColor(GREEN);
      display.println("verbunden");
      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
      delay(10000);
      break;
    }
    Serial.print("Versuch " + String(i) + "/3 fehlgeschlagen, rc=");
    Serial.print(mqttClient.state());
    display.setTextColor(RED);
    display.print("Versuch " + String(i) + "/3 fehlgeschlagen, rc=");
    display.print(mqttClient.state());
    if(i<=2) {
      display.getCursor(cursor_pos[0],cursor_pos[1]);
      for(int j=5; j>=0; j--) {
        Serial.println(" | " + String(j) + "s");
        display.setTextColor(CYAN);
        display.setCursor(cursor_pos[0],cursor_pos[1]);
        display.println(" | " + String(j) + "s");
        delay(1000);
        display.setTextColor(BLACK);
        display.setCursor(cursor_pos[0],cursor_pos[1]);
        display.println(" | " + String(j) + "s");
      }
    }else {
      delay(10000); 
    }
  }
  if(warmup==true) {
    // MH-Z19B Aufwärmen
    display.clearScreen();
    display.setCursor(0,0);
    display.setTextColor(CYAN);
    display.print("MH-Z19B waermt auf - ");
    Serial.print("MH-Z19B waermt auf - ");
    display.getCursor(cursor_pos[0],cursor_pos[1]);
    for(int64_t i=(180000000-esp_timer_get_time())/1000000; i>=0; i--) {
      display.setTextColor(CYAN);
      Serial.println(i);
      display.print(i);
      delay(1000);
      display.setCursor(cursor_pos[0],cursor_pos[1]);
      display.setTextColor(BLACK);
      display.print(i);
      display.setCursor(cursor_pos[0],cursor_pos[1]);
    }
  }
}

void loop() {
  now = esp_timer_get_time();
  
  //Überprüfen, ob DATA_DELAY abgelaufen ist
  if((now-lastMsgTime)/1000 > DATA_DELAY) {
    lastMsgTime = now;

    //CCS811 Daten abfragen
    if(ccs.available()){
      ccs811_temp = ccs.calculateTemperature();
      if(!ccs.readData()){
        ccs811_co2 = ccs.geteCO2();
        ccs811_tvoc = ccs.getTVOC();
      }
      else{
        Serial.println("ERROR, could not read CCS811!");
        while(1);
      }
    }

    //HDC1080 Daten abfragen
    hdc1080_temp = hdc1080.readTemperature();
    hdc1080_humidity = hdc1080.readHumidity();

    //MH-Z19 Daten abfragen
    mhz19_co2 = myMHZ19.getCO2();                             
    mhz19_temp = myMHZ19.getTemperature();                    

    //nur, wenn eine MQTT Verbindung besteht
    if(mqttClient.connected() == true) {
      
      //MQTT Verbindung aufrechterhalten
      mqttClient.loop();
      
      // MQTT Ausgabe
      mqttPublish((char*)((String)MQTT_DEVICE_TOPIC + "mhz19_co2").c_str(), (char*)((String)mhz19_co2).c_str());
      mqttPublish((char*)((String)MQTT_DEVICE_TOPIC + "mhz19_temp").c_str(), (char*)((String)mhz19_temp).c_str());
      mqttPublish((char*)((String)MQTT_DEVICE_TOPIC + "hdc1080_temp").c_str(), (char*)((String)hdc1080_temp).c_str());
      mqttPublish((char*)((String)MQTT_DEVICE_TOPIC + "hdc1080_humidity").c_str(), (char*)((String)hdc1080_humidity).c_str());
      mqttPublish((char*)((String)MQTT_DEVICE_TOPIC + "ccs811_co2").c_str(), (char*)((String)ccs811_co2).c_str());
      mqttPublish((char*)((String)MQTT_DEVICE_TOPIC + "ccs811_tvoc").c_str(), (char*)((String)ccs811_tvoc).c_str());
      mqttPublish((char*)((String)MQTT_DEVICE_TOPIC + "ccs811_temp").c_str(), (char*)((String)ccs811_temp).c_str());

      //MQTT Status-LED
      digitalWrite(LED,HIGH);
    }else {
      digitalWrite(LED,LOW);
    }
        
    // Display Ausgabe CO2
    x = mhz19_co2/(RectMax/76);
    color = GREEN;
    if (mhz19_co2>alarm1) {
      color = YELLOW;
    }
    if (mhz19_co2>alarm2) {
      color = ORANGE;
    }
    if (mhz19_co2>alarm3) {
      color = RED;
    }
    display.clearScreen();
    display.setTextColor(color);
    display.setTextScale(2);
    display.setCursor(20, 15);
    display.print(mhz19_co2);
    display.setTextScale(1);
    display.print("ppm");
    display.drawRect(9, 39, 80, 12,WHITE);  //Start_x, Start_y, Länge (nach rechts), Dicke (nach unten)
    display.fillRect(10, 40, x, 10, color); //Start_x, Start_y, Länge (nach rechts), Dicke (nach unten); startet eins weiter unten und rechts als der Rand, damit dieser noch zu sehen ist. Da noch der blaue Strich mit der Dicke 2 dazukommt, ist der maximale Wert für x innerhalb des Balkens 80 (Länge Balken) - 2 (jeweils ein Pixel Rand links und rechts) - 2 (Breite des blauen Striches)
    display.drawRect(x+10, 40, 2, 10,BLUE); //Start_x, Start_y, Länge (nach rechts), Dicke (nach unten); x+10, da x sich auf die Länge der Balkenfüllung bezieht, dieser jedoch bei 10 beginnt (10 ist also der Offset)

    // Display Ausgabe Temperatur und Luftfeuchtigkeit
    display.setTextColor(BLUE);
    display.setCursor(0, 0);
    display.setTextScale(1);
    display.print(hdc1080_temp);
    display.print(" °C");
    
    display.setTextColor(BLUE);
    display.setCursor(57, 0);
    display.setTextScale(1);
    display.print(hdc1080_humidity);
    display.print(" %");

    // Display Ausgabe TVOC und eCO2
    display.setTextColor(BLUE);
    display.setCursor(0, 56);
    display.setTextScale(1);
    display.print(ccs811_co2);
    display.print(" ppm");
    
    display.setTextColor(BLUE);
    display.setCursor(57, 56);
    display.setTextScale(1);
    display.print(ccs811_tvoc);
    display.print(" ppb");
  }
  if((now-lastReconnectTime)/60000000 > RECONNECT_DELAY) {
    lastReconnectTime = now;
    
    //WiFi-Status überprüfen, falls nicht verbunden --> wifiReconnect(), falls verbunden --> MQTT-Status überprüfen, falls nicht verbunden --> mqttReconnect(), falls verbunden --> nichts
    if (WiFi.status() != WL_CONNECTED) {
      wifiReconnect();
    }else {
      if (!mqttClient.connected()) {
        mqttReconnect();
      }
    }
  }
}
