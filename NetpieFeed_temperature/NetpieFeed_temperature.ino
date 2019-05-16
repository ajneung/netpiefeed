//json temp, humid
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include "DHT.h"

const char* ssid = "";
const char* password = "";

#define APPID ""
#define KEY ""
#define SECRET ""
#define ALIAS ""
#define FEEDID ""
#define FEEDAPI ""

#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float humid = 0;
float temp = 0;

long lastDHTRead = 0;
long lastTimeWriteFeed = 0;

WiFiClient client;
MicroGear microgear(client);

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen){
  Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  if(WiFi.begin(ssid, password)){
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");  
    } 
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  microgear.on(CONNECTED, onConnected);
  microgear.init(KEY,SECRET,ALIAS);
  microgear.connect(APPID);
}

void loop() {
  if(microgear.connected()){
    microgear.loop();
    if(millis() - lastDHTRead > 5000){
      humid = dht.readHumidity();
      temp = dht.readTemperature();
      lastDHTRead = millis();

      Serial.print("Humid : "); Serial.print(humid); Serial.print(" %, ");
      Serial.print("Temp : "); Serial.print(temp); Serial.println(" C ");

      if(isnan(humid) || isnan(temp)){
        Serial.println("Failed to read from DHT sensor!");  
      }else{
        Serial.print("publish temp -> ");
        Serial.print(temp); Serial.print("humid -> "); Serial.println(humid);
        microgear.publish("/outdoor/temp", (String)temp);
        microgear.publish("/outdoor/humid", (String)humid);
      }
    }

    if(millis() - lastTimeWriteFeed > 30000){
      lastTimeWriteFeed = millis();
      if(humid!=0 && temp!=0){
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["temp"] = temp;
        root["humid"] = humid;

        String jsonData;
        root.printTo(jsonData);

        Serial.print("Write Feed --> ");
        Serial.println(jsonData);
        microgear.writeFeed(FEEDID, jsonData);
      }  
    }
  }else{
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);  
  }
}
