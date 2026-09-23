// Duncan Wilson Sept 2026 - Breathing White Nightlight MQTT Messenger to Vespera
// Works with Arduino MKR1010
//
// This sketch creates a calming breathing effect by smoothly ramping the white
// light intensity (R=G=B) up and down across all 72 LEDs.

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" 
#include <utility/wifi_drv.h> // Library to drive the onboard RGB LED on the MKR1010

/*
**** please enter your sensitive data in the Secret tab/arduino_secrets.h
**** using format below
#define SECRET_SSID "ssid name"
#define SECRET_PASS "ssid password"
#define SECRET_MQTTUSER "user name - eg student"
#define SECRET_MQTTPASS "password";
*/
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* ssid1         = SECRET_SSID1;
const char* password1     = SECRET_PASS1;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = "mqtt.cetools.org";
const int mqtt_port       = 1884;

// Create WiFi and MQTT client objects
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Update lightId with your allocated student number (10–40)
String lightId = "99";

// MQTT topic to publish color data to: "student/CASA0014/luminaire/<lightId>"
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;            
String clientId = ""; // Unique client ID based on MAC address

// NeoPixel Configuration (72 LEDs * 3 bytes for RGB = 216 bytes)
const int num_leds = 72;
const int payload_size = num_leds * 3; // x3 for RGB

// Create the byte array to send in MQTT payload
byte RGBpayload[payload_size];

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting Vespera Breathing White Sender..."));

  // Status LEDs on MKR1010 NINA module
  WiFiDrv::pinMode(25, OUTPUT); // R
  WiFiDrv::pinMode(26, OUTPUT); // G
  WiFiDrv::pinMode(27, OUTPUT); // B
  LedRed();

  // Generate a unique client ID using the device's MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  clientId = "MKR1010_White_" + String(macStr);

  Serial.print(F("Device Client ID: "));
  Serial.println(clientId);
  Serial.print(F("Target Light ID: "));
  Serial.println(lightId);
  Serial.print(F("Publishing to topic: "));
  Serial.println(mqtt_topic);

  // Connect to WiFi
  startWifi();

  // Connect to MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2048);
  mqttClient.setKeepAlive(30);
  
  Serial.println(F("Setup complete - starting breathing white cycle!"));
}
 
void loop() {
  // Check Wi-Fi and MQTT connections
  if (WiFi.status() != WL_CONNECTED) {
    startWifi();
  }
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // 1. Fade up (inhale)
  Serial.println(F("Breathing in (fading up)..."));
  for (int b = 10; b <= 185; b += 3) {
    send_colour_to_all(b, b, b);
    mqttClient.loop(); // Keep MQTT connection alive during loop
    delay(30);
  }
  delay(500); // Brief hold at peak brightness

  // 2. Fade down (exhale)
  Serial.println(F("Breathing out (fading down)..."));
  for (int b = 185; b >= 10; b -= 3) {
    send_colour_to_all(b, b, b);
    mqttClient.loop(); // Keep MQTT connection alive during loop
    delay(30);
  }
  delay(500); // Brief hold at trough
}

// Sets all 72 LEDs to the given (r, g, b) color and publishes to MQTT
void send_colour_to_all(int r, int g, int b) {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)constrain(r, 0, 255);
      RGBpayload[pixel * 3 + 1] = (byte)constrain(g, 0, 255);
      RGBpayload[pixel * 3 + 2] = (byte)constrain(b, 0, 255);
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
  } else {
    Serial.println(F("MQTT client not connected, cannot publish."));
  }
}
