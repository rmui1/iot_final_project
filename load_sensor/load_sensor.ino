#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
 
#include "HX711.h"
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/info"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/req"

// based on https://how2electronics.com/connecting-esp32-to-amazon-aws-iot-core-using-mqtt/

const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;

float w;
HX711 scale;

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connect_to_aws()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
 
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(on_message_receive);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void send_weight()
{
  StaticJsonDocument<200> doc;
  doc["weight"] = w;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void on_message_receive(char* topic, byte* payload, unsigned int length)
{
  send_weight();
}
 
void setup()
{
  Serial.begin(115200);
  connect_to_aws();
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  delay(50);

  long zero_factor = scale.read_average(100);
  scale.set_offset(zero_factor);
}
 
void loop()
{
  client.loop();
  delay(100);
}