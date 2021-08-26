/**************************************************************************************
This is the code of the CO2-sensor. It has been written by Julian Partanen within a project for the Gymnasium St. Mauritz.
It has been published under the AGPL-3.0 license.
**************************************************************************************/

// add libraries
#include <PubSubClient.h>       // for the MQTT connection
#include <WiFi.h>                // Wifi library of the ESP32
#include "ClosedCube_HDC1080.h"   // for the HDC1080 sensor
#include "MHZ19.h"                // for the MH-Z19B sensor
#include <SSD_13XX.h>            //for the SSD1331 display
#include "Adafruit_CCS811.h"      // for the CCS811 sensor

/***!!!start configuration area!!!***/
// general configuration
#define DATA_DELAY 5000    // pause between measuring points in millisecond, default: 5 seconds per measuring point - for the MH-Z19B this is the minimum value!
#define RECONNECT_DELAY 15 // pause between attempts to reconnect to the Wifi network and/or the MQTT server if the connection got interrupted (leads to a freeze of the display and increases network traffic, why this value shouldn't be too low. Possible solution: Using multi-core capabilities of the ESP32: https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/)
#define alarm1 1000        // limit for first alarm (yellow font)
#define alarm2 1500        // limit for second alarm (orange font)
#define alarm3 2000        // limit for third alarm (red font)
#define RectMax 2500       // maximum CO2-concentration which can be displayed inside the rectangle
#define warmup true        // the MH-Z19B sensor have to warmup for 3 minutes, give in microseconds (look at the Datasheet https://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19b-co2-ver1_0.pdf). Attention: Set this only for development purposes to false!!!
#define LED 4              // GPIO-pin of the status LED

// Offsets - for the callibration of the sensors can be determined an offset which can be set here
#define offset_mhz19b_co2 0
#define offset_mhz19b_temp 0
#define offset_hdc1080_temp 0
#define offset_hdc1080_hum 0
#define offset_ccs811_co2 0
#define offset_ccs811_tvoc 0
#define offset_ccs811_temp ccs.calculateTemperature()-(hdc1080.readTemperature()+offset_hdc1080_temp)    // only necessary, if ccs_temperature is set to true (look at the configuration of the CJMCU-8118 sensor | The temperature sensor of the CCS811 sensor isn't calibrated by default, without this adjustments the readings would be completely wrong. So the offset of the CCS811 sensor is being with the reading of the HDC1080 sensor.

// Wifi login data & configuration
const char* ssid = "Wifi name";
const char* password = "Wifi password";
const int timeout = 30; //in seconds

// MQTT server configuration
#define MQTT_SERVER "192.168.1.50"              // e.g. the local IP-address of the Raspberry on which the MQTT and Grafana servers are running (can be determined with the command 'ip addr')
#define MQTT_PORT 1883                          // default MQTT-port: 1883
#define MQTT_USER ""                            // default: empty
#define MQTT_PASSWORD ""                        // default: empty
#define MQTT_CLIENT_ID "co2Node"                // default: co2Nod
#define MQTT_DEVICE_TOPIC "iot/chemie/"         // default: iot/chemie
#define MQTT_TOPIC_STATE "iot/chemie/status"    // default: iot/chemie/status
WiFiClient co2Node;                           // initialize MQTT client
PubSubClient mqttClient(co2Node);             // initialize MQTT client

// configuration of the MH-Z19B sensor
#define auto_calibration true                            // defines if the automatic calibration of the MH-Z19B should be activated. For a manuel calibration you have to execute a calibration code once, look at the example code of the MH-Z19B library.
#define RX_PIN 16                                        // This pin is connected to the TX-pin of the MH-Z19B
#define TX_PIN 17                                        // This pin is connected to the RS-pin of the MH-Z19B
#define BAUDRATE 9600                                    // Baudrate of the MH-Z19B, don't change!
MHZ19 myMHZ19;                                           // constructor for the MH-Z19B Sensor
HardwareSerial mySerial(1);                              // Initializes UART-connection with MH-Z19B sensor

// configuration of the SSD1331 display
#define __CS    5                                          
#define __DC    26 
#define __RST   27
//define __mosi 23
//define __sclk 18
SSD_13XX display = SSD_13XX(__CS, __DC, __RST);

// configuration of the CJMCU-8118 sensor / the CCS811 and HDC1080 sensor
#define ccs_temperature false       // regulates wheter the temperature values of the CCS811 sensor are outputed with MQTT. For the CJMCU-8118 this should be set to 'false', because otherwise it will lead to errors. If the CCS811 sensor is used in standalone, this can be set to 'true'.
Adafruit_CCS811 ccs;                // Initializes CCS811 Sensor
ClosedCube_HDC1080 hdc1080;      // Initializes HDC1080 Sensor
/***!!!End configuration area!!!***/

// needed variables
int64_t now;                   // for the execution of the DELAY, the current time will be stored here
int64_t lastMsgTime = 0;       // for the execution of the DATA_DELAY, last measurement time will be stored here
int64_t lastReconnectTime = 0; // for the exectuion of the RECONNECT_DELAY, last measurement time will be stored here
int mhz19_temp;                // variable for measurement output
int mhz19_co2;                 // variable for measurement output
float hdc1080_temp;            // variable for measurement output
float hdc1080_humidity;        // variable for measurement output
int ccs811_co2;                // variable for measurement output
int ccs811_tvoc;               // variable for measurement output
float ccs811_temp;             // variable for measurement output
int color = BLUE;              // to display the mhz19_co2 value at the display. The color changes dependent on the value
int x;                         // to display the mhz19_co2 value at the display. The x-position of the rectangle changes dependent on the value
int16_t cursor_pos[1];         // for the countdown at the beginning (e.g. warmup of the MH-Z19B). The cursor position (x and y) get stored here

// definition of functions (this will only be needed in loop(), the initial setup of Wifi and MQTT happens in setup())    
void wifiReconnect() {   //with it the measuring device can indepentendly without an reboot connect to the Wifi network
  Serial.println("connecting to " + String(ssid));
  WiFi.begin(ssid, password);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("connected - IP: ");
    Serial.println(WiFi.localIP());
  } 
}

void mqttReconnect() {   //with it the measuring device can indepentendly without an reboot connect to the MQTT server
    Serial.print("connecting to " + String(MQTT_DEVICE_TOPIC) + " with MQTT...");

    // trying to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");

      // If connection attempt is successful, this will be sent
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    }else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
    }
}

void setup() {
  //start serial connection & lettering
  Serial.begin(115200);
  Serial.println("CO2- & air quality sensor");

  //start display communication & bootscreen
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
  display.setCursor(12,42);
  display.println("air quality sensor");
  delay(5000);
  display.clearScreen();
  
  //define Status LED as LED Pin
  pinMode(LED,OUTPUT);
  
  //configure MH-Z19B uart connection
  mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN); 
  myMHZ19.begin(mySerial);                                
  myMHZ19.autoCalibration(auto_calibration);

  //CJMCU-8118 (or CCS811 und HDC1080) I2C connection configuration and starting
  hdc1080.begin(0x40);
  if(!ccs.begin()){
    Serial.println("CCS811 sensor not connected!");
    display.setTextColor(RED);
    display.setCursor(0,0);
    display.println("CCS811 sensor not connected!");
    while(1);
  }
  while(!ccs.available());
  if (ccs_temperature == true) {                   //CCS811 temperature calibration
    float temp = ccs.calculateTemperature();
    ccs.setTempOffset(offset_ccs811_temp);
  }

  //WiFi initial setup - with a timeout of 30 seconds
  Serial.println("connecting to " + String(ssid) + " - timeout in: ");
  display.setTextColor(CYAN);
  display.print("connecting to " + String(ssid) + " - timeout in: ");
  WiFi.begin(ssid, password);
  display.getCursor(cursor_pos[0],cursor_pos[1]);
  for(int i=timeout; i>=0; i--) { 
    display.setTextColor(CYAN);
    Serial.println(i);
    display.println(i);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("connected - IP: ");
      Serial.println(WiFi.localIP());
      display.setTextColor(GREEN);
      display.print("connected - IP: ");
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
    Serial.println("connection failed: timeout");
    display.println("connection failed: timeout");
    WiFi.disconnect();
    delay(10000);
  }

  //MQTT initial setup - three attempts with an pause of 5 seconds
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  Serial.println("connecting to " + String(MQTT_DEVICE_TOPIC) + " with MQTT...");
  display.clearScreen();
  display.setTextColor(CYAN);
  display.setCursor(0,0);
  display.println("connecting to " + String(MQTT_DEVICE_TOPIC) + " with MQTT...");
  for(int i=1; i<=3; i++) {
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");
      display.setTextColor(GREEN);
      display.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
      delay(10000);
      break;
    }
    Serial.print("attempt " + String(i) + "/3 failed, rc=");
    Serial.print(mqttClient.state());
    display.setTextColor(RED);
    display.print("attempt " + String(i) + "/3 failed, rc=");
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
    // MH-Z19B warmup
    display.clearScreen();
    display.setCursor(0,0);
    display.setTextColor(CYAN);
    display.print("MH-Z19B is warming up - ");
    Serial.print("MH-Z19B is warming up - ");
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
      
      //maintain MQTT connection
      mqttClient.loop();
    }
  }
}

void loop() {
  now = esp_timer_get_time();
  
  //check, if DATA_DELAY is expired
  if((now-lastMsgTime)/1000 > DATA_DELAY) {
    lastMsgTime = now;

    //get CCS811 data
    if(ccs.available()){
      if (ccs_temperature == true) {
        ccs811_temp = ccs.calculateTemperature();  //offset has been set in the calibration area
      }
      if(!ccs.readData()){
        ccs811_co2 = ccs.geteCO2()+offset_ccs811_co2;
        ccs811_tvoc = ccs.getTVOC()+offset_ccs811_tvoc;
      }
      else{
        Serial.println("Couldn't read any data of the CCS811 sensor");
        display.clearScreen();
        display.setCursor(0,0);
        display.setTextColor(RED);
        display.print("Couldn't read any data of the CCS811 sensor");
        while(1);
      }
    }

    //get HDC1080 data
    hdc1080_temp = hdc1080.readTemperature()+offset_hdc1080_temp;
    hdc1080_humidity = hdc1080.readHumidity()+offset_hdc1080_temp;

    //get MH-Z19B data
    mhz19_co2 = myMHZ19.getCO2()+offset_mhz19b_co2;
    mhz19_temp = myMHZ19.getTemperature()+offset_mhz19b_temp;            

    //only, if a MQTT connection is established
    if(mqttClient.connected() == true) {
      
      //maintain MQTT connection
      mqttClient.loop();
      
      // Send data with MQTT
      mqttClient.publish((char*)((String)MQTT_DEVICE_TOPIC + "mhz19_co2").c_str(), (char*)((String)mhz19_co2).c_str(), true);
      mqttClient.publish((char*)((String)MQTT_DEVICE_TOPIC + "mhz19_temp").c_str(), (char*)((String)mhz19_temp).c_str(), true);
      mqttClient.publish((char*)((String)MQTT_DEVICE_TOPIC + "hdc1080_temp").c_str(), (char*)((String)hdc1080_temp).c_str(), true);
      mqttClient.publish((char*)((String)MQTT_DEVICE_TOPIC + "hdc1080_humidity").c_str(), (char*)((String)hdc1080_humidity).c_str(), true);
      mqttClient.publish((char*)((String)MQTT_DEVICE_TOPIC + "ccs811_co2").c_str(), (char*)((String)ccs811_co2).c_str(), true);
      mqttClient.publish((char*)((String)MQTT_DEVICE_TOPIC + "ccs811_tvoc").c_str(), (char*)((String)ccs811_tvoc).c_str(), true);
      if (ccs_temperature == true) {
        mqttClient.publish((char*)((String)MQTT_DEVICE_TOPIC + "ccs811_temp").c_str(), (char*)((String)ccs811_temp).c_str(), true);
      }
      Serial.println("succesfully sent measurement data with MQTT");

      //MQTT status LED
      digitalWrite(LED,LOW);
    }else {
      digitalWrite(LED,HIGH);
    }

    // serial output for measurement data for serial plotter
    Serial.println("mhz19_co2,mhz19_temp,hdc1080_temp,hdc1080_humidity,ccs811_co2,ccs811_tvoc,ccs811_temp");
    Serial.print(String(mhz19_co2) + ",");
    Serial.print(String(mhz19_temp) + ",");;
    Serial.print(String(hdc1080_temp) + ",");
    Serial.print(String(hdc1080_humidity) + ",");
    Serial.print(String(ccs811_co2) + ",");
    Serial.print(String(ccs811_tvoc) + ",");
    Serial.println(String(ccs811_temp));
    Serial.println(" ");
        
    // OLED Display output CO2 values
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
    display.drawRect(9, 39, 80, 12,WHITE);  //Start_x, Start_y, length (to the right), thickness (to  the bottom)
    display.fillRect(10, 40, x, 10, color); //Start_x, Start_y, length (to the right), thickness (to  the bottom); starts one pixel more to thhe bottom and right than the edge, in order to let the edge be still visible. Because the blue stroke with a thickness of 2 comes to this, the maximum value for x is inside the rectangle 80 (length of the rectangle) - 2 (respectively one pixel at thhe right and left) - 2 (wide of the blue stroke)
    display.drawRect(x+10, 40, 2, 10,BLUE); //Start_x, Start_y, length (to the right), thickness (to  the bottom); x+10, because x reffers to the length of the filling of the rectangle, this however starts at 10 (10 is also the offset)

    // OLED display output temperature and humidity
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

    // OLED Display output TVOC and eCO2
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
    
    //check Wifi-status, if not connected --> wifiReconnect(), if connected --> check MQTT status, if not connected --> mqttReconnect(), if connected --> nothing
    if (WiFi.status() != WL_CONNECTED) {
      wifiReconnect();
    }else {
      if (!mqttClient.connected()) {
        mqttReconnect();
      }
    }
  }
}
