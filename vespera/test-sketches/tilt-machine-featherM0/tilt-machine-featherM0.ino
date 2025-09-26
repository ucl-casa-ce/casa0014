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

// Sensor pin definitions
int s1 = 9;
int s2 = 10;
int s3 = 11;

// Variables to hold current and last states
int lastReportedState = -1; 
int potentialNewState = -1;

// Debounce variables
unsigned long lastChangeTime = 0;
unsigned long debounceDelay = 150; // Adjust as needed

int temp = 1;

void setup() {
  //Configure pins for Adafruit ATWINC1500 Feather
  // https://learn.adafruit.com/adafruit-feather-m0-wifi-atwinc1500/using-the-wifi-module
  WiFi.setPins(8,7,4,2);

  // Initialize serial communication for debugging
  Serial.begin(115200);
  //while (!Serial); // Wait for serial port to connect (useful for debugging)
  // wait a little for serial port to catch up
  delay(1000);



  // Set the MQTT server and callback function
  mqttClient.setServer(mqtt_server, mqtt_port);

  // Connect to Wi-Fi
  setup_wifi();

  // Connect to mqtt broker
  Serial.println("Starting Feather M0 MQTT Controller...");
  reconnect_mqtt();

  // switch setup
  pinMode(s1, INPUT);
  pinMode(s2, INPUT);
  pinMode(s3, INPUT);


  // wait a little bit so that wifi and mqtt can be
  // Read initial state and report it - am storing as decimal from binary inputs
  // e.g. 111 = 4*1 + 2*1 + 1 = 7 ; 010 = 4*0 +2*1 + 0 = 2
  potentialNewState = (digitalRead(s1) * 4) + (digitalRead(s2) * 2) + digitalRead(s3);
  lastReportedState = potentialNewState;
  updateWhichSideUp(potentialNewState);
}

void loop() {
  // Ensure MQTT connection is maintained
  if (!mqttClient.connected()) {
    reconnect_mqtt();
  }
  // Process incoming MQTT messages and maintain connection
  mqttClient.loop();


  // Read current sensor values and combine them into a single state variable
  int currentRead = (digitalRead(s1) * 4) + (digitalRead(s2) * 2) + digitalRead(s3);

  // If the current reading is different from the potential new state,
  // we've detected a change. Reset the timer and the potential state.
  if (currentRead != potentialNewState) {
    lastChangeTime = millis();
    potentialNewState = currentRead;
  }

  // Check if the potential new state has been stable long enough AND
  // if it's different from the last state we reported.
  if ((millis() - lastChangeTime) > debounceDelay) {
    if (potentialNewState != lastReportedState) {
      // The new state is stable and hasn't been reported. Report it.
      lastReportedState = potentialNewState;
      Serial.println(lastReportedState);
      updateWhichSideUp(lastReportedState);
    }
  }

}

// Function to send MQTT based on a combined state value
void updateWhichSideUp(int state) {
  switch (state) {
    Serial.println(state);
    case 0: // Binary 000: 4 + 2 + 1
      send_RGB(255,0,0);
      //Serial.println("Side 0 is UP");
      break;
    case 1: // Binary 001: 0 + 0 + 1
      send_RGB(0,0,0);
      //Serial.println("Side 1 is UP");
      break;
    case 2: // Binary 010: 0 + 2 + 0
      send_RGB(255,0,0);
      //Serial.println("Side 2 is UP");
      break;
    case 3: // Binary 011: 0 + 2 + 1
      send_RGB(255,255,0);
      //Serial.println("Side 3 is UP");
      break;
    case 4: // Binary 100: 0 + 2 + 1
      send_RGB(255,0,255);
      //Serial.println("Side 4 is UP");
      break;
    case 5: // Binary 101: 4 + 0 + 1
      send_RGB(255,255,255);
      //Serial.println("Side 5 is UP");
      break;
    case 6: // Binary 110: 4 + 2 + 0
      send_RGB(0,0,255);
      //Serial.println("Side 6 is UP");
      break;
    case 7: // Binary 111: 4 + 2 + 1
      send_RGB(0,0,0);
      //Serial.println("Side 7 is UP");
      break;
    default:
      //Serial.println("No recognised state.");
      break;
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
    
    //Serial.println("Published an all-green byte array.");
  } else {
    //Serial.println("MQTT client not connected, cannot publish.");
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


