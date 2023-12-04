#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Panasonic.h>

#define dhtpin 33
#define dhttype DHT22
DHT dht(dhtpin, dhttype);
float humi, temp;

int mq2 = 34;
int irRemote = 32;
int temp = 16;
IRPanasonicAc ac(irRemote);  
IRsend irsend(irRemote);


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
  res = wm.autoConnect("Node 1", "qweasdzxc");
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

  if (messageTemp == "1" ||messageTemp == "nyala") {
    ac.on();
    ac.setSwingVertical(true);
    ac.setSwingHorizontal(false);
    ac.setTemp(temp);
    #if SEND_PANASONIC_AC
      ac.send();
    #endif 
    delay(2000);
    client.publish("/kiyura/rac", "nyala");
    
  } else if (messageTemp == "0" ||messageTemp == "mati"){
    ac.off();
    #if SEND_PANASONIC_AC
      ac.send();
    #endif 
    delay(2000);   
    client.publish("/kiyura/rac", "mati");
  }
}
void reconnect(){
  while (!client.connected()){
    Serial.print("Attempting MQTT connection...");
    clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)){
      Serial.println("MQTT berhasil terhubung");
      client.publish("/kiyura/status", "node1"); 
      client.subscribe("/kiyura/ac");
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
  ac.begin();
  irsend.begin();
  Serial.begin(115200);
  Serial.println("MQTT Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
}

void loop()
{
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  if (!client.loop()){
    client.connect(clientId.c_str(), mqtt_user, mqtt_pass);
  }
  
  now = millis();
  if (now-lastMeasure > 5000)
  {
    humi = dht.readHumidity();
    temp = dht.readTemperature();
    
    int mq2_value = analogRead(mq2);

    static char temperatureTemp[7];
    dtostrf(temp, 4, 2, temperatureTemp);
    Serial.println(temperatureTemp);
    client.publish("/kiyura/temp2", temperatureTemp);

    static char humidityTemp[7];
    dtostrf(humi, 4, 2, humidityTemp);
    Serial.println(humidityTemp);
    client.publish("/kiyura/humi2", humidityTemp);

    static char asap1[7];
    sprintf(asap1,"%d",mq2_value);
    //dtostrf(mq2_value, 4, 2, asap1Temp);
    Serial.println(asap1);
    client.publish("/kiyura/asap1", asap1);

  }
  delay(5000);
}
