
#define DECODE_NEC
#define DECODE_HASH



#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include "PinDefinitionsAndMore.h" // Define output pin for IR
#include <IRremote.hpp>

#include <Adafruit_Sensor.h>
#include <DHT.h>

 
const char *SSID = "SSID";
const char *PWD = "PWD";
#define LED 25 
#define PIN 27
#define IR_SEND_PIN 4
#define DHTTYPE    DHT11
int RELAYS[6] = {26, 19, 18, 23, 33, 21};
// Web server running on port 80
WebServer server(80);

 
// Sensor
DHT dht(PIN, DHTTYPE);
 
// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];
 
// env variable
float temperature;
float humidity;
int relay[6];
String jsnTypeName = "Rstate";
 WiFiManager wm;
void connectToWiFi() {

 //wm.resetSettings();
bool res;
 res = wm.autoConnect("AutoConnectAP","password");

if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.print("connected to ");
        //Serial.println(wm.getWiFiPass());
        Serial.println(wm.getWiFiSSID());
        //Serial.println(WiFi.localIP());
    }
 
  //Serial.println(SSID);
  
  //WiFi.begin(SSID, PWD);
  
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    
  }
 
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}
void setup_routing() {	 	 
  server.on("/temperature", getTemperature);	 	 
  server.on("/humidity", getHumidity);	 	 
  server.on("/led", HTTP_POST, handlePost);	 
  	server.on("/reset", resetWiFi);
  	 	 
  // start server	 	 
  server.begin();	 	 
}
 
void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}
 

void read_sensor_data(void * parameter) {
   for (;;) {
      temperature = dht.readTemperature();
     Serial.print("Temp: ");
     Serial.print(temperature);
     Serial.println("°C");     
     humidity = dht.readHumidity();
     Serial.println("Read sensor data");
 
     // Felatat késleltetése
     vTaskDelay(20000 / portTICK_PERIOD_MS);
   }
}
 
void getTemperature() {
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", buffer);
    
}
 
void getHumidity() {
  Serial.println("Get humidity");
  create_json("humidity", humidity, "%");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", buffer);
  
   
   
}
 
void handlePost() {
 
  String body = server.arg("plain");

  deserializeJson(jsonDocument, body);
  Serial.println(body);
  for (int i = 0; i < 6; i++) {
  jsnTypeName = jsnTypeName+String(i+1);
  relay[i] = jsonDocument[jsnTypeName];
  Serial.println(jsnTypeName + ": " + String(relay[i]));
  Serial.println(relay[i]);
  jsnTypeName = "Rstate";
  }
  int lamp = jsonDocument["lampState"];
  serializeJson(jsonDocument, Serial);
  for(int i = 0; i < 6; i++)
  {
    digitalWrite(RELAYS[i], relay[i]);
  }
  if (lamp == 1) {
  IrSender.sendNEC(0x1, 0x0, 1);
  Serial.println("Switched");
  }

  
  serializeJson(jsonDocument, buffer);
  
      server.send(200, "application/json", buffer);

    
}

void resetWiFi() {
  wm.resetSettings();
  ESP.restart();
  connectToWiFi();
}

void setup_task() {	 	 
  xTaskCreate(	 	 
  read_sensor_data, 	 	 
  "Read sensor data", 	 	 
  1000, 	 	 //Stack memory allocation
  NULL, 	 	 
  1, 	 	 //task priority
  NULL 	 	 
  );	 	 
}
void setup() {	 	 
  pinMode(LED, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(21, OUTPUT);
  digitalWrite(23, LOW);
  Serial.begin(115200);	 	 
 	 	 IrSender.begin(IR_SEND_PIN);
  // Sensor setup	 	 
  if (isnan(dht.readTemperature())) {	 	 
    Serial.println("Problem connecting to DHT11");	 	 
  }	 	 
  connectToWiFi();	 	 
  setup_task();	 	 
  setup_routing(); 	 	 
  	 	 
  	
}	 	 
  	 	 
void loop() {	 	 
  server.handleClient();	 	 
}