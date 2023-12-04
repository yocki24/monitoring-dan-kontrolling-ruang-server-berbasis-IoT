#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <WiFiManager.h>

#define rainpin A0

String clientId;

//const char *ssid = "[]";
//const char *pass = "satuduatiga";

const char *mqtt_server = "138.3.218.179";
const char *mqtt_user = "kiyura";
const char *mqtt_pass = "1234";

WiFiClient espClient;
PubSubClient client(espClient);

long now = millis();
long lastMeasure = 0;

void setup_wifi()
{
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("Node 2", "qweasdzxc");
  if(!res){
    Serial.println("Gagal terhubung");    
  } else {
    Serial.println("Koneksi berhasil");
  }
}

void reconnect(){
  while (!client.connected()){
    Serial.println("Attempting MQTT connection...");
    clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)){
      Serial.println("MQTT Berhasil Terhubung");
      client.publish("/kiyura/status", "node2");
    }
    else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("MQTT Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(rainpin, INPUT);
}

void loop()
{
  if (!client.connected()){
    reconnect();
  }
  if (!client.loop()){
    client.connect(clientId.c_str(), mqtt_user, mqtt_pass);
  }
  now = millis();
  if (now-lastMeasure > 5000)
  {
    int maxval = 1024;
    int rainValue = maxval - analogRead(rainpin);
    
    static char rain[7];
    sprintf(rain,"%d",rainValue);
    Serial.println(rain);
    client.publish("/kiyura/raindrop", rain);
  }
  delay(5000);
}
