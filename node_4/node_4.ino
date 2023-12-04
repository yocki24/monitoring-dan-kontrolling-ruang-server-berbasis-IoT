#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>

#define dhtpin D2
#define dhttype DHT22
DHT dht(dhtpin, dhttype);
float humi, temp;
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
  res = wm.autoConnect("Node 4", "qweasdzxc");
  if(!res){
    Serial.println("Gagal terhubung");    
  } else {
    Serial.println("Koneksi berhasil");
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
      Serial.println("MQTT berhasil terhubung");
      client.publish("/kiyura/status", "node4");
    }
    else
    {
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
  dht.begin();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect(clientId.c_str(), mqtt_user, mqtt_pass);
  }
  now = millis();
  if (now - lastMeasure > 5000)
  {
    humi = dht.readHumidity();
    temp = dht.readTemperature();
    if (isnan(humi) || isnan(temp))
    {
      Serial.print("Pembancaan DHT22 gagal");
      delay(1000);
      return;
    }
    static char temperatureTemp[7];
    dtostrf(temp, 4, 2, temperatureTemp);
    Serial.println(temperatureTemp);
    client.publish("/kiyura/temp", temperatureTemp);

    static char humidityTemp[7];
    dtostrf(humi, 4, 2, humidityTemp);
    Serial.println(humidityTemp);
    client.publish("/kiyura/humi", humidityTemp);
  }
  delay(5000);
}
