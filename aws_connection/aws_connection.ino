#include "secrets.h"
#include "esp_camera.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <base64.h>
#include <EEPROM.h>
#include "HX711.h"

#define EEPROM_SIZE 1

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/info"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/req"

// based on https://towardsaws.com/using-aws-iot-core-to-watch-my-cat-a12876965e30
// based on https://github.com/leandrodamascena/aws-detect-a-cat/blob/master/arduino/iot_aws_core.ino

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(15000);

void connect_to_aws()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.onMessage(on_message_receive);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  
  Serial.println("");

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void on_message_receive(String &topic, String &payload) {
  send_image();
}

void start_camera() {

  pinMode(4, OUTPUT);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  #if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
  #endif

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  #if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
  #endif
  
}

void send_image() {
  // turn on flash
  digitalWrite(4, HIGH);

  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb) {
     Serial.println("Could not take photo");
      return;
  } else {
      Serial.println("Took photo!");
  }
  
  const char *data = (const char *)fb->buf;
  String encoded = base64::encode(fb->buf, fb->len);

  if (!client.publish(AWS_IOT_PUBLISH_TOPIC, encoded)) {
    lwMQTTErr(client.lastError());
  }
  Serial.println("Published");

  esp_camera_fb_return(fb);

  // turn off flash
  digitalWrite(4, LOW);
 
}

void lwMQTTErr(lwmqtt_err_t reason)
{
  if (reason == lwmqtt_err_t::LWMQTT_SUCCESS)
    Serial.println("Success");
  else if (reason == lwmqtt_err_t::LWMQTT_BUFFER_TOO_SHORT)
    Serial.println("Buffer too short");
  else if (reason == lwmqtt_err_t::LWMQTT_VARNUM_OVERFLOW)
    Serial.println("Varnum overflow");
  else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_FAILED_CONNECT)
    Serial.println("Network failed connect");
  else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_TIMEOUT)
    Serial.println("Network timeout");
  else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_FAILED_READ)
    Serial.println("Network failed read");
  else if (reason == lwmqtt_err_t::LWMQTT_NETWORK_FAILED_WRITE)
    Serial.println("Network failed write");
  else if (reason == lwmqtt_err_t::LWMQTT_REMAINING_LENGTH_OVERFLOW)
    Serial.println("Remaining length overflow");
  else if (reason == lwmqtt_err_t::LWMQTT_REMAINING_LENGTH_MISMATCH)
    Serial.println("Remaining length mismatch");
  else if (reason == lwmqtt_err_t::LWMQTT_MISSING_OR_WRONG_PACKET)
    Serial.println("Missing or wrong packet");
  else if (reason == lwmqtt_err_t::LWMQTT_CONNECTION_DENIED)
    Serial.println("Connection denied");
  else if (reason == lwmqtt_err_t::LWMQTT_FAILED_SUBSCRIPTION)
    Serial.println("Failed subscription");
  else if (reason == lwmqtt_err_t::LWMQTT_SUBACK_ARRAY_OVERFLOW)
    Serial.println("Suback array overflow");
  else if (reason == lwmqtt_err_t::LWMQTT_PONG_TIMEOUT)
    Serial.println("Pong timeout");
}

void lwMQTTErrConnection(lwmqtt_return_code_t reason)
{
  if (reason == lwmqtt_return_code_t::LWMQTT_CONNECTION_ACCEPTED)
    Serial.println("Connection Accepted");
  else if (reason == lwmqtt_return_code_t::LWMQTT_UNACCEPTABLE_PROTOCOL)
    Serial.println("Unacceptable Protocol");
  else if (reason == lwmqtt_return_code_t::LWMQTT_IDENTIFIER_REJECTED)
    Serial.println("Identifier Rejected");
  else if (reason == lwmqtt_return_code_t::LWMQTT_SERVER_UNAVAILABLE)
    Serial.println("Server Unavailable");
  else if (reason == lwmqtt_return_code_t::LWMQTT_BAD_USERNAME_OR_PASSWORD)
    Serial.println("Bad UserName/Password");
  else if (reason == lwmqtt_return_code_t::LWMQTT_NOT_AUTHORIZED)
    Serial.println("Not Authorized");
  else if (reason == lwmqtt_return_code_t::LWMQTT_UNKNOWN_RETURN_CODE)
    Serial.println("Unknown Return Code");
}

void setup() {
  Serial.begin(9600);
  start_camera();
  connect_to_aws();
}

void loop() {
  client.loop();
  delay(1000);
}