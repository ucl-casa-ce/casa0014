// Duncan Wilson Sept 2026 - Vespera Column Rainbow Rotation
// Works with Arduino MKR1010
//
// The 72 NeoPixel LEDs in Vespera are arranged as a hemisphere of
// 12 columns around the perimeter, with 6 vertical pixels in each column:
//   - Column 0:  Pixels  0 to  5
//   - Column 1:  Pixels  6 to 11
//   - Column 2:  Pixels 12 to 17
//   - ...
//   - Column 11: Pixels 66 to 71
//
// This sketch creates a smooth rainbow wheel that rotates around the 12 columns.

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" 
#include <utility/wifi_drv.h> // Library to drive RGB LED on MKR1010

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

// Update lightId with your allocated user number (e.g. "0", "1", "2", etc.)
String lightId = "99";
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;            
String clientId = ""; // Unique client ID based on MAC address

// Hemisphere Grid Geometry
const int NUM_COLUMNS = 12;      // 12 columns around the perimeter
const int PIXELS_PER_COL = 6;    // 6 vertical LEDs per column
const int num_leds = NUM_COLUMNS * PIXELS_PER_COL; // 72 LEDs total
const int payload_size = num_leds * 3;             // 3 bytes per LED (R, G, B)

// Array holding RGB values for all 72 LEDs to send in MQTT payload
byte RGBpayload[payload_size];

// Function Prototypes
void startWifi();
void reconnectMQTT();
void LedRed();
void LedGreen();
void LedBlue();
void set_pixel(int pixel, int r, int g, int b);
void set_column_color(int col, int r, int g, int b);
void send_payload();
void getWheelColor(byte WheelPos, int &r, int &g, int &b);

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting Vespera Column Rainbow Rotation..."));

  // Status LEDs on MKR1010 NINA module
  WiFiDrv::pinMode(25, OUTPUT); // R
  WiFiDrv::pinMode(26, OUTPUT); // G
  WiFiDrv::pinMode(27, OUTPUT); // B
  LedRed();

  // Generate a unique client ID using MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  clientId = "MKR1010_Rainbow_" + String(macStr);

  Serial.print(F("Device Client ID: "));
  Serial.println(clientId);
  Serial.print(F("Publishing to topic: "));
  Serial.println(mqtt_topic);

  // Connect to WiFi & MQTT
  startWifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(512);
  mqttClient.setKeepAlive(30);

  Serial.println(F("Setup complete!"));
}

void loop() {
  // Ensure network connectivity
  if (WiFi.status() != WL_CONNECTED) {
    startWifi();
  }
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Rotate the rainbow around the 12 columns
  // 'step' shifts the starting color position on each frame
  for (int step = 0; step < 256; step += 4) {
    for (int col = 0; col < NUM_COLUMNS; col++) {
      // Offset each column by ~21 units (256 / 12) so all 12 columns form a complete 360-degree rainbow
      byte wheelPos = (step + (col * (256 / NUM_COLUMNS))) & 255;
      
      int r, g, b;
      getWheelColor(wheelPos, r, g, b);

      // Set all 6 vertical LEDs in this column to this color
      set_column_color(col, r, g, b);
    }

    // Send the completed frame to Vespera
    send_payload();
    mqttClient.loop(); // Keep MQTT connection alive
    delay(100);         // Control rotation speed (lower = faster)
  }
}

// ====================================================================
// Helper Functions
// ====================================================================

// Sets the color of all 6 vertical pixels in a specific column (0 to 11)
void set_column_color(int col, int r, int g, int b) {
  if (col < 0 || col >= NUM_COLUMNS) return;

  int startPixel = col * PIXELS_PER_COL; // e.g. Col 0 -> 0..5, Col 1 -> 6..11
  for (int row = 0; row < PIXELS_PER_COL; row++) {
    set_pixel(startPixel + row, r, g, b);
  }
}

// Sets the RGB color of a single pixel (0 to 71) in the local buffer
void set_pixel(int pixel, int r, int g, int b) {
  if (pixel < 0 || pixel >= num_leds) return;

  RGBpayload[pixel * 3 + 0] = (byte)constrain(r, 0, 255);
  RGBpayload[pixel * 3 + 1] = (byte)constrain(g, 0, 255);
  RGBpayload[pixel * 3 + 2] = (byte)constrain(b, 0, 255);
}

// Publishes the RGBpayload buffer to the MQTT broker
void send_payload() {
  if (mqttClient.connected()) {
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
  }
}

// Converts a position (0-255) on the color wheel to smooth R, G, B values
void getWheelColor(byte WheelPos, int &r, int &g, int &b) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    r = 255 - WheelPos * 3;
    g = 0;
    b = WheelPos * 3;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    r = 0;
    g = WheelPos * 3;
    b = 255 - WheelPos * 3;
  } else {
    WheelPos -= 170;
    r = WheelPos * 3;
    g = 255 - WheelPos * 3;
    b = 0;
  }
}
