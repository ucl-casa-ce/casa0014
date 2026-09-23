// Duncan Wilson Sept 2026 - Random Colours MQTT Messenger to Vespera
// Works with Arduino MKR1010
//
// This sketch demonstrates manipulating individual LEDs in the 216-byte buffer
// by assigning randomized RGB values to each of the 72 LEDs.

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
  Serial.println(F("Starting Vespera Random Colors Sender..."));

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
  clientId = "MKR1010_Random_" + String(macStr);

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
  
  Serial.println(F("Setup complete - sending random sparkles!"));
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

  // Generate and send a new random frame to all 72 LEDs
  send_all_random();
  delay(100); // 10 frames per second
}

// Assigns a random RGB color to every LED and publishes the frame over MQTT
void send_all_random() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)random(50, 256); // Red
      RGBpayload[pixel * 3 + 1] = (byte)random(50, 256); // Green
      RGBpayload[pixel * 3 + 2] = (byte)random(50, 256); // Blue
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
  } else {
    Serial.println(F("MQTT client not connected, cannot publish."));
  }
}
