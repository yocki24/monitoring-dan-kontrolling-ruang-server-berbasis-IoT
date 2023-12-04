#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

#define pinrelay D2

//#include "DHT.h"
//#define DHTPIN 2     
//#define DHTTYPE DHT22 
//DHT dht(DHTPIN, DHTTYPE);

//KONFIGURASI JARINGAN WIFI
//const char* ssid = "[]";
//const char* pass = "satuduatiga";

//MQTT BROKER
const char* mqtt_user = "kiyura";
const char* mqtt_pass = "1234";
const char* mqtt_server = "138.3.218.179";


WiFiClient espClient;
PubSubClient client(espClient);

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

unsigned long previousMillis = 0;
const long interval = 2000;

//float humi, temp;

void setupwifi(){
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("Node 3", "qweasdzxc");
  if(!res){
    Serial.println("Gagal terhubung");    
  } else {
    Serial.println("Koneksi berhasil");
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if (messageTemp == "1" ||messageTemp == "nyala") {
    digitalWrite(pinrelay, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    client.publish("/kiyura/rstate", "nyala");
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else if (messageTemp == "0" ||messageTemp == "mati"){
    digitalWrite(pinrelay, LOW);  // Turn the LED off by making the voltage HIGH
    client.publish("/kiyura/rstate", "mati");
  }
}
void reconnect(){
  while(!client.connected()){
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if(client.connect(clientId.c_str(), mqtt_user, mqtt_pass)){
      Serial.println("MQTT Berhasil Terhubung");
      client.publish("/kiyura/status", "node3");
      client.subscribe("/kiyura/relay");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupwifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(pinrelay, OUTPUT);
  delay(3000);
}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
      
      //KONFIGURASI MQTT
      if(!client.connected()){
        reconnect();
      }
      client.loop();   
  }

}
