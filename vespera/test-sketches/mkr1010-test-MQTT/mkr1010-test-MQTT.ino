// This sketch allows an Arduino MKR1010 to control a string of 72 NeoPixel RGB LEDs
// via MQTT. It subscribes to a specific topic and interprets binary payloads
// as RGB values for each LED. 

#include <SPI.h> // Required for WiFiNINA
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <utility/wifi_drv.h>

#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1883;
const char* mqtt_client_id = "MKR1010_NeoPixel_Luminaire_Test";
const char* mqtt_subscribe_topic = "student/CASA0014/luminaire/#";
const char* user_update_topic = "student/CASA0014/luminaire/user";

int LUMINAIRE_USER = 0;

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 72
#define NEOPIXEL_DATA_LENGTH (NEOPIXEL_COUNT * 3)

byte ledDataBuffer[NEOPIXEL_DATA_LENGTH];
volatile bool newPixelDataAvailable = false;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastConnectionAttempt = 0;
unsigned long lastWifiCheck = 0;
const unsigned long RECONNECT_INTERVAL_MS = 5000;
const unsigned long WIFI_CHECK_INTERVAL_MS = 5000;

void setup_wifi();
bool reconnect_mqtt();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);

  Serial.println(F("Starting MKR1010 NeoPixel MQTT Controller (Test)..."));

  pixels.begin();
  pixels.clear();
  pixels.show();
  pixels.setBrightness(50);

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2048);
  mqttClient.setKeepAlive(30);
  mqttClient.setCallback(mqtt_callback);

  setup_wifi();
}

void loop() {
  unsigned long now = millis();

  if (now - lastWifiCheck > WIFI_CHECK_INTERVAL_MS) {
    lastWifiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("Wi-Fi is disconnected. Reconnecting..."));
      setup_wifi();
      return;
    }
  }

  if (!mqttClient.connected()) {
    if (now - lastConnectionAttempt > RECONNECT_INTERVAL_MS) {
      lastConnectionAttempt = now;
      reconnect_mqtt();
    }
  } else {
    mqttClient.loop();
  }

  if (newPixelDataAvailable) {
    newPixelDataAvailable = false;
    for (int i = 0; i < NEOPIXEL_COUNT; i++) {
      byte r = ledDataBuffer[i * 3 + 0];
      byte g = ledDataBuffer[i * 3 + 1];
      byte b = ledDataBuffer[i * 3 + 2];
      pixels.setPixelColor(i, r, g, b);
    }
    pixels.show();
  }
}

void setup_wifi() {
  Serial.print(F("Connecting to WiFi: "));
  Serial.println(ssid);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed!"));
    while (true);
  }

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5) {
    WiFi.begin(ssid, pass);
    Serial.print(F("."));
    delay(3000);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nWiFi connected!"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("\nWiFi connection failed, will retry in loop."));
  }
}

bool reconnect_mqtt() {
  Serial.print(F("Attempting MQTT connection..."));
  wifiClient.stop();

  if (mqttClient.connect(mqtt_client_id)) {
    Serial.println(F("connected!"));
    if (mqttClient.subscribe(mqtt_subscribe_topic)) {
      Serial.print(F("Subscribed to topic: "));
      Serial.println(mqtt_subscribe_topic);
      return true;
    } else {
      Serial.println(F("Failed to subscribe to topic!"));
      return false;
    }
  } else {
    Serial.print(F("failed, rc="));
    Serial.print(mqttClient.state());
    Serial.println(F(" trying again in 5 seconds"));
    return false;
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, user_update_topic) == 0) {
    char str[16];
    unsigned int len = (length < sizeof(str) - 1) ? length : sizeof(str) - 1;
    memcpy(str, payload, len);
    str[len] = '\0';
    LUMINAIRE_USER = atoi(str);
    Serial.print(F("LUMINAIRE_USER updated to: "));
    Serial.println(LUMINAIRE_USER);
    return;
  }

  char* last_slash = strrchr(topic, '/');
  if (last_slash != NULL) {
    int topic_user_id = atoi(last_slash + 1);
    if (topic_user_id == LUMINAIRE_USER && length == NEOPIXEL_DATA_LENGTH) {
      memcpy(ledDataBuffer, payload, NEOPIXEL_DATA_LENGTH);
      newPixelDataAvailable = true;
    }
  }
}
