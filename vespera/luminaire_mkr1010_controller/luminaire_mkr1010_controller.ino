// This sketch allows an Arduino MKR1010 to control a string of 72 NeoPixel RGB LEDs
// via MQTT. It subscribes to a specific topic and interprets binary payloads
// as RGB values for each LED. 

// --- Libraries ---
// You'll need to install these libraries via the Arduino IDE's Library Manager:
// 1. WiFiNINA by Arduino: For Wi-Fi connectivity on MKR boards.
// 2. PubSubClient by Nick O'Leary: For MQTT communication.
// 3. Adafruit NeoPixel by Adafruit: For controlling NeoPixel LEDs.

#include <SPI.h> // Required for WiFiNINA
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <utility/wifi_drv.h>   // library to drive to RGB LED on the MKR1010

#include "arduino_secrets.h" // so that you can exclude these from your GitHub repo


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


// --- MQTT Broker Configuration ---
// Replace with your MQTT broker's IP address or hostname
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1883;                                // Default MQTT port (unsecured)
const char* mqtt_client_id = "MKR1010_NeoPixel_Luminaire_Vespera"; // Unique client ID for your device

// --- MQTT Topic Configuration ---
// Base topic for the entire luminaire group
const char* mqtt_base_topic = "student/CASA0014/luminaire";

// The full topic string for THIS device's updates (will be generated dynamically)
// Assuming user IDs are up to 4 digits, a length of 64 is safe.
char mqtt_data_topic[64];

// Fixed control topics
const char* user_update_topic = "student/CASA0014/luminaire/user";
const char* brightness_update_topic = "student/CASA0014/luminaire/brightness";

// --- Use this variable to see if we should change lights or not
// --- Defaults to 0 - staff user - on start up
int LUMINAIRE_USER = 0;
int LUMINAIRE_BRIGHTNESS = 150;

// --- NeoPixel Configuration ---
#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 72
#define NEOPIXEL_DATA_LENGTH (NEOPIXEL_COUNT * 3) // 3 bytes per LED (R, G, B)


// --- Global Objects ---
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// --- Connection Recovery Variables ---
unsigned long lastConnectionAttempt = 0;
const long RECONNECT_INTERVAL_MS = 5000; // Try reconnecting every 5 seconds

// --- Function Prototypes ---
void setup_wifi();
void reconnect_mqtt();
void mqtt_callback(char* topic, byte* payload, unsigned int length);


void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  //while (!Serial); // Wait for serial port to connect (useful for debugging)

  // RGB LED's
  WiFiDrv::pinMode(25, OUTPUT); // G
  WiFiDrv::pinMode(26, OUTPUT); // R
  WiFiDrv::pinMode(27, OUTPUT); // B  

  LedRed(); // board alive but not yet ready


  Serial.println("Starting MKR1010 NeoPixel MQTT Controller...");

  // Initialize NeoPixels
  pixels.begin(); // Initialize the NeoPixel library
  pixels.show();  // Turn all pixels off initially
  pixels.setBrightness(LUMINAIRE_BRIGHTNESS); // Set a default brightness (0-255) to avoid blinding light

  // Set the MQTT server and callback function
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqtt_callback);

  // Generate the initial data subscription topic ---
  // Format: "student/CASA0014/luminaire/0"
  snprintf(mqtt_data_topic, sizeof(mqtt_data_topic), "%s/%d", mqtt_base_topic, LUMINAIRE_USER);
  Serial.print("Initial data topic set to: ");
  Serial.println(mqtt_data_topic);

  // Connect to Wi-Fi
  setup_wifi();
}

void loop() {
  // check to make sure wifi is connected, if not setup again
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi is disconnected. Re-running setup_wifi...");
    setup_wifi(); // This function already has the loop to wait for connection
    // If setup_wifi() succeeds, it will fall through to MQTT check.
    return; // Optionally return to start loop again immediately
  }

  // Ensure MQTT connection is maintained
  if (!mqttClient.connected()) {
    reconnect_mqtt();
  }
  // Process incoming MQTT messages and maintain connection
  mqttClient.loop();
}

// --- WiFi Setup Function ---
void setup_wifi() {
  delay(10);
  LedBlue(); // show Blue LED when looking for wifi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  // Check if the WiFi module is present
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  LedGreen(); // all good so show green for go  
}

// --- MQTT Reconnection Function ---
void reconnect_mqtt() {
  // Only attempt reconnection if the interval has passed
  if (millis() - lastConnectionAttempt > RECONNECT_INTERVAL_MS) {
    lastConnectionAttempt = millis();
    // Loop until we're reconnected
    LedBlue();
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect with a unique client ID
    if (mqttClient.connect(mqtt_client_id)) {
      Serial.println("connected!");

// Once connected, subscribe to all necessary topics
      bool subscribed_ok = true;

      // 1. Subscribe to the 'user' update topic
      if (mqttClient.subscribe(user_update_topic)) {
        Serial.print("Subscribed to user topic: ");
        Serial.println(user_update_topic);
      } else {
        Serial.println("Failed to subscribe to user topic!");
        subscribed_ok = false;
      }
      
      // 2. Subscribe to the 'brightness' update topic
      if (mqttClient.subscribe(brightness_update_topic)) {
        Serial.print("Subscribed to brightness topic: ");
        Serial.println(brightness_update_topic);
      } else {
        Serial.println("Failed to subscribe to brightness topic!");
        subscribed_ok = false;
      }

      // 3. Subscribe to the device's specific data topic
      if (mqttClient.subscribe(mqtt_data_topic)) {
        Serial.print("Subscribed to data topic: ");
        Serial.println(mqtt_data_topic);
      } else {
        Serial.println("Failed to subscribe to data topic!");
        subscribed_ok = false;
      }

      if (subscribed_ok) {
        LedGreen(); // Success!
      } else {
        LedRed(); // Failure!
      }

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      LedRed(); // Still failing
    }
  }
}

// --- MQTT Message Callback Function ---
// This function is called whenever a message is received on a subscribed topic.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Convert the payload to a C-style string for easier manipulation
  // memcpy and payload_str[length] = '\0' lines are a standard way to safely convert 
  // the received byte array payload into a null-terminated string (char*), 
  // which is required for functions like atoi.
  char payload_str[length + 1];
  memcpy(payload_str, payload, length);
  payload_str[length] = '\0'; 
  
  // Print the received message for debugging
  //Serial.print("Message received on topic: ");
  //Serial.println(topic);
  //Serial.print("Message: ");
  //Serial.println(payload_str);

  // 1. Check if the topic is for updating the LUMINAIRE_USER variable
  // strcmp returns 0 if the two strings are identical
  if (strcmp(topic, user_update_topic) == 0) {
    int new_user_id = atoi(payload_str);
    
    // Only update if the user ID has actually changed
    if (new_user_id != LUMINAIRE_USER) {
      
      // Unsubscribe from the OLD topic
      Serial.print("Unsubscribing from old topic: ");
      Serial.println(mqtt_data_topic);
      mqttClient.unsubscribe(mqtt_data_topic);

      // Update the global LUMINAIRE_USER variable
      LUMINAIRE_USER = new_user_id;

      // Generate the NEW topic string
      snprintf(mqtt_data_topic, sizeof(mqtt_data_topic), "%s/%d", mqtt_base_topic, LUMINAIRE_USER);

      // Subscribe to the NEW topic
      if (mqttClient.subscribe(mqtt_data_topic)) {
        Serial.print("Subscribed to NEW topic: ");
        Serial.println(mqtt_data_topic);
      } else {
        Serial.println("Failed to subscribe to NEW topic!");
      }
      
      Serial.print("LUMINAIRE_USER updated to: ");
      Serial.println(LUMINAIRE_USER);

      pixels.clear();
      pixels.show();

    } else {
      Serial.println("LUMINAIRE_USER is already set to this ID. No change.");
    } 
    
  } else if (strcmp(topic, brightness_update_topic) == 0){
    // Convert the string payload to an integer
    int new_brightness_id = atoi(payload_str);
    
    // Update the global LUMINAIRE_USER variable
    LUMINAIRE_BRIGHTNESS = new_brightness_id;
    
    Serial.print("LUMINAIRE_BRIGHTNESS updated to: ");
    Serial.println(LUMINAIRE_BRIGHTNESS);  
    pixels.setBrightness(LUMINAIRE_BRIGHTNESS); // Set a default brightness (0-255) to avoid blinding light
  
  }
  // 2. Check if the topic ends with the current LUMINAIRE_USER ID
  else {
    // Find the last '/' in the topic string to isolate the number
    char* last_slash = strrchr(topic, '/');
    if (last_slash != NULL) {
      // Move the pointer past the slash
      char* number_str = last_slash + 1;
      int topic_user_id = atoi(number_str);
      
      // Compare the extracted number with our current LUMINAIRE_USER
      if (topic_user_id == LUMINAIRE_USER) {
        //Serial.print("This message is for my luminaire (ID: ");
        //Serial.print(LUMINAIRE_USER);
        //Serial.println("). Processing payload...");
        
        // Add your specific code to process the payload here.
        // For example, control a light or other component.
        // For now, we'll just print it.
        //Serial.print("Payload to process: ");
        //Serial.println(payload_str);


        // Check if the payload length matches the expected length for all LEDs
        if (length == NEOPIXEL_DATA_LENGTH) {
          // Iterate through the payload, 3 bytes at a time for each LED
          for (int i = 0; i < NEOPIXEL_COUNT; i++) {
            // Extract RGB values. Payload format: R1, G1, B1, R2, G2, B2, ...
            // NeoPixel library expects GRB order for WS2812B, but we set it as RGB
            // and the library handles the conversion if NEO_GRB is used.
            // So, payload[index] is Red, payload[index+1] is Green, payload[index+2] is Blue.
            byte r = payload[i * 3];     // Red component
            byte g = payload[i * 3 + 1]; // Green component
            byte b = payload[i * 3 + 2]; // Blue component

            // Set the color for the current pixel
            pixels.setPixelColor(i, r, g, b);
          }
          // Update the NeoPixels to show the new colors
          pixels.show();
          //Serial.println("NeoPixels updated!");
        } else {
          //Serial.print("Warning: Received payload length (");
          //Serial.print(length);
          //Serial.print(") does not match expected length (");
          //Serial.print(NEOPIXEL_DATA_LENGTH);
          //Serial.println(") for 48 RGB LEDs. Ignoring update.");
        }




      } else {
       //Serial.print("This message is for a different luminaire (ID: ");
        //Serial.print(topic_user_id);
        //Serial.println("). Ignoring.");
      }
    }
  }
}
