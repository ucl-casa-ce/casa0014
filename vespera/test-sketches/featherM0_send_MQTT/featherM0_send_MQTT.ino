// This sketch allows an Adafruit Feather M0 to update a string of 72 NeoPixel RGB LEDs
// via MQTT. It publishes to a specific topic, sending binary payloads
// as RGB values for each LED. 

// --- Libraries ---
// You'll need to install these libraries via the Arduino IDE's Library Manager:
// 1. WiFi101 by Arduino: For Wi-Fi connectivity on Adafruit ATWINC1500 Feather M0
// 2. PubSubClient by Nick O'Leary: For MQTT communication.

#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>

#include "arduino_secrets.h" // so that you can exclude these from your GitHub repo


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


// --- MQTT Broker Configuration ---
// Replace with your MQTT broker's IP address or hostname
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;                                // Default MQTT port
const char* mqtt_client_id = "Luminaire_colour_box_ucjtdjw"; // Unique client ID for your device
const char* mqtt_publish_topic = "student/CASA0014/luminaire/0"; // Topic to subscribe to
char mqtt_user[] = MQTT_USERNAME;
char mqtt_pass[] = MQTT_PASSWORD;

// --- NeoPixel Configuration ---
const int num_leds = 72;
const int payload_size = num_leds * 3; // x3 for RGB

// Create the byte array to send in MQTT payload
// this stores all the colours in memory so that 
// they can be accessed in for example the rainbow function
byte payload[payload_size];

// Global variable for the scrolling effect
int scrollOffset = 0;

// --- Global Objects ---
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// --- Function Prototypes ---
void setup_wifi();
void reconnect_mqtt();

int temp = 1;

void setup() {
  //Configure pins for Adafruit ATWINC1500 Feather
  // https://learn.adafruit.com/adafruit-feather-m0-wifi-atwinc1500/using-the-wifi-module
  WiFi.setPins(8,7,4,2);

  // Initialize serial communication for debugging
  Serial.begin(115200);
  //while (!Serial); // Wait for serial port to connect (useful for debugging)

  Serial.println("Starting Feather M0 MQTT Controller...");

  // Set the MQTT server and callback function
  mqttClient.setServer(mqtt_server, mqtt_port);

  // Connect to Wi-Fi
  setup_wifi();
}

void loop() {
  // Ensure MQTT connection is maintained
  if (!mqttClient.connected()) {
    reconnect_mqtt();
  }
  // Process incoming MQTT messages and maintain connection
  mqttClient.loop();

  send_RGB(255,0,0);
  delay(1000);
  //send_RGB(0,255,0);
  //delay(1000);
  //send_RGB(0,0,255);
  //delay(1000);

  // Call the rainbow effect function
  //send_rainbow_effect();
  
  // A small delay to control the scroll speed
  delay(80); 

}

void send_all_green() {
  // Check if the client is connected before publishing
  if (mqttClient.connected()) {
    // Fill the byte array with the green color pattern
    for (int i = 0; i < num_leds; i++) {
      payload[i * 3 + 0] = 0x00; // Red
      payload[i * 3 + 1] = 0xFF; // Green
      payload[i * 3 + 2] = 0x00; // Blue
    }
    
    // Publish the byte array
    mqttClient.publish(mqtt_publish_topic, payload, payload_size);
    
    Serial.println("Published an all-green byte array.");
  } else {
    Serial.println("MQTT client not connected, cannot publish.");
  }
}

void send_RGB(int r, int g, int b) {
  // Check if the client is connected before publishing
  if (mqttClient.connected()) {
    // Fill the byte array with the specified RGB color pattern
    for (int i = 0; i < num_leds; i++) {
      payload[i * 3 + 0] = (byte)r; // Red
      payload[i * 3 + 1] = (byte)g; // Green
      payload[i * 3 + 2] = (byte)b; // Blue
    }

    // Publish the byte array
    mqttClient.publish(mqtt_publish_topic, payload, payload_size);
    
    Serial.println("Published an all-green byte array.");
  } else {
    Serial.println("MQTT client not connected, cannot publish.");
  }
}

// Helper function to convert HSV to RGB
void hsvToRgb(int h, int s, int v, byte& r, byte& g, byte& b) {
  h = h % 360;
  float c = v * s;
  float x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
  float m = v - c;
  
  if (h >= 0 && h < 60) {
    r = (c + m) * 255; g = (x + m) * 255; b = m * 255;
  } else if (h >= 60 && h < 120) {
    r = (x + m) * 255; g = (c + m) * 255; b = m * 255;
  } else if (h >= 120 && h < 180) {
    r = m * 255; g = (c + m) * 255; b = (x + m) * 255;
  } else if (h >= 180 && h < 240) {
    r = m * 255; g = (x + m) * 255; b = (c + m) * 255;
  } else if (h >= 240 && h < 300) {
    r = (x + m) * 255; g = m * 255; b = (c + m) * 255;
  } else if (h >= 300 && h < 360) {
    r = (c + m) * 255; g = m * 255; b = (x + m) * 255;
  }
}

// --- Function to create and send the scrolling rainbow effect ---
void send_rainbow_effect() {
  // Check if the client is connected before publishing
  if (mqttClient.connected()) {
    // Generate the rainbow pattern for all 72 LEDs
    for (int i = 0; i < num_leds; i++) {
      // Calculate a unique hue for each LED based on its position and the scroll offset
      int hue = ((i * 360 / num_leds) + scrollOffset) % 360;
      
      byte r, g, b;
      // Convert the HSV color (hue, saturation=1.0, value=1.0) to RGB
      hsvToRgb(hue, 1.0, 1.0, r, g, b);
      
      // Populate the payload array with the calculated RGB values
      payload[i * 3 + 0] = r;
      payload[i * 3 + 1] = g;
      payload[i * 3 + 2] = b;
    }

    // Publish the byte array
    mqttClient.publish(mqtt_publish_topic, payload, payload_size);
    
    // Increment the offset for the next frame to create the scrolling effect
    // We only need a small increment for a smooth scroll.
    scrollOffset++; 
    
    // Optional: Print to serial for debugging
    // Serial.println("Published a scrolling rainbow frame.");
  } else {
    // Serial.println("MQTT client not connected, cannot publish.");
  }
}

// --- WiFi Setup Function ---
void setup_wifi() {
  delay(10);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  // Check if the WiFi module is present
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Communication with WiFi shield failed!");
    // Don't continue if module is not present
    while (true);
  }

  // Attempt to connect to WiFi network
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected!");
  printCurrentNet();
  printWiFiData();
}

// --- MQTT Reconnection Function ---
void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect with a unique client ID
    if (mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected!");
      // Once connected, subscribe to the topic

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP address : ");
  Serial.println(ip);

  Serial.print("Subnet mask: ");
  Serial.println((IPAddress)WiFi.subnetMask());

  Serial.print("Gateway IP : ");
  Serial.println((IPAddress)WiFi.gatewayIP());

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type: ");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}


