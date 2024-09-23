// works with D1 Mini 8266 and 12 Neopixel ring
// https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html


#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 


// USER SET VARIABLE FOR EACH DEPLOYMENT START
// Each device should connect to the feed that belongs to it
// e.g. light/1/ or light/2/ etc.
const char* mqtt_topic_pixel = "student/CASA0014/light/2/pixel/";
const char* mqtt_topic_all = "student/CASA0014/light/2/all/";
#define LIGHT 2
// USER SET VARIABLE FOR EACH DEPLOYMENT END


#define PIN         2   // data pin of neopixel (pin 2 seems to be D4 on ESP8266?)
#define NUMPIXELS   12  // length of neopixels
#define STARTBRIGHT 100 // value from 0 to 255
 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);

// Array to store RGBW values for each NeoPixel
uint8_t pixelColorsArray[NUMPIXELS][4]; // Each element is an array [R, G, B, W]

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
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;


WiFiClient espClient;
PubSubClient client(espClient);

// Function to handle incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // Check which topic the message is received from
  if (strcmp(topic, mqtt_topic_pixel) == 0) {
    handlePixelUpdate(payload, length);
  } else if (strcmp(topic, mqtt_topic_all) == 0) {
    handleAllUpdate(payload, length);
  } else {
    Serial.println("Unknown topic");
  }
}

void printKeys(const JsonObject& obj) {
  for (JsonPair kv : obj) {
    Serial.print("Key: ");
    Serial.println(kv.key().c_str());
  }
}

// Function to handle messages received on topic pixel
void handlePixelUpdate(byte* payload, unsigned int length) {

  // Parse JSON
  DynamicJsonDocument doc(200);
  deserializeJson(doc, payload, length);

  //expecting to receive a JSON packet in this format
  //{
  //  "pixelid": 1,
  //  "R": 255,
  //  "G": 0,
  //  "B": 128,
  //  "W": 200
  //}

  // Extract values
  int pixelid = doc["pixelid"];
  int R = doc["R"];
  int G = doc["G"];
  int B = doc["B"];
  int W = doc["W"];
  setPixelColor(pixelid, R, G, B, W);

  // Print values
  Serial.print("Pixel ID: ");
  Serial.println(pixelid);
  Serial.print("R: ");
  Serial.println(R);
  Serial.print("G: ");
  Serial.println(G);
  Serial.print("B: ");
  Serial.println(B);
  Serial.print("W: ");
  Serial.println(W);
}

// Function to handle messages received on topic all
void handleAllUpdate(byte* payload, unsigned int length) {

  Serial.println(length);
  // Parse JSON
  DynamicJsonDocument doc(2000);
  DeserializationError error = deserializeJson(doc, payload, length);

  // Check for parsing errors
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Print all keys in the JSON document
  Serial.println("Keys in the JSON document:");
  printKeys(doc.as<JsonObject>());

  //expecting to receive a JSON packet in this format
  //{
  //  "method": "clear"
  //}

  // Check if the method is "clear"
  if (doc.containsKey("method")) {
    const char* method = doc["method"];
    if (strcmp(method, "clear") == 0) {
      setAllPixels(0); // set all values to 0 "off"
      Serial.println("setAllPixels(0) run.");
    } else if (strcmp(method, "onerandom") == 0) {
      setOneRGBPixelRandom();
      Serial.println("setOneRGBPixelRandom() run.");
    } else if (strcmp(method, "allrandom") == 0) {
      setAllPixelsRandom();
    } else {
      Serial.println("Unknown method");
    }
  }

  // Check if "colors" array exists
  if (!doc.containsKey("allLEDs")) {
    Serial.println("No 'allLEDs' array found in the JSON document.");
    return;
  }else{
    Serial.println("'allLEDs' array found in the JSON document.");
    // Extract colors array from the JSON packet
    //Serial.println(static_cast<const char*>(doc["allLEDs"]));

    //String jsonString;
    //serializeJson(doc["allLEDs"], jsonString);
    //Serial.println(jsonString);

    JsonArray colorsArray = doc["allLEDs"];

    // Check if "allLEDs" is an array
    if (colorsArray.isNull()) {
      Serial.println("The 'allLEDs' variable is not an array.");
      return;
    }

    // Iterate over each color object in the colors array
    int i = 0;
    for (JsonObject colorObject : colorsArray) {
      // Extract individual color data
      int pixelid = colorObject["pixelid"];
      uint8_t R = colorObject["R"];
      uint8_t G = colorObject["G"];
      uint8_t B = colorObject["B"];
      uint8_t W = colorObject["W"];

      // Assign color data to the pixelColorsArray
      pixelColorsArray[pixelid][0] = R;
      pixelColorsArray[pixelid][1] = G;
      pixelColorsArray[pixelid][2] = B;
      pixelColorsArray[pixelid][3] = W;

      // Increment index
      i++;
    }

    // Print the parsed color data
    for (int j = 0; j < NUMPIXELS; j++) {
      Serial.print("Color ");
      Serial.print(j);
      Serial.print(": Pixel ID: ");
      Serial.print(j);
      Serial.print(", R: ");
      Serial.print(pixelColorsArray[j][0]);
      Serial.print(", G: ");
      Serial.print(pixelColorsArray[j][1]);
      Serial.print(", B: ");
      Serial.print(pixelColorsArray[j][2]);
      Serial.print(", W: ");
      Serial.println(pixelColorsArray[j][3]);
    }
    breatheEffect(1,50);
  }

  
}

void startWifi(){
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // check to see if connected and wait until you are
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(600);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}


void reconnectMQTT() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client-neo2-ucjtdjw", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to the topic you want to listen to
      client.subscribe(mqtt_topic_pixel);
      client.subscribe(mqtt_topic_all);
      Serial.println("Subscribed to MQTT topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // configure neopixels
  pinMode(LED_BUILTIN, OUTPUT);
  pixels.begin();
  pixels.setBrightness(STARTBRIGHT);

  // Connect to WiFi
  startWifi();

  // quick test to see if all leds are working
  // i use this rather than pulseWhite so that i can see if all rgb leds are also working
  setAllPixels(100); // set all values to 100
  breatheEffect(1, 12); // 2 loops, 12 msec delay
  setAllPixels(0); // set all values to 0 "off"
  
  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(2000);
  client.setCallback(callback);

  Serial.println("Set-up complete");

}
 
void loop() {
  // Reconnect if necessary
  if (!client.connected()) {
    reconnectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  // keep mqtt alive
  client.loop();

  writeAllPixels(255); // leave as same colour set in random 255/255 = 1
  pixels.show();

}



// loops through all stored pixel values and runs pixels.show to update LED's
// brightness can be used to dim the values - e.g. is used in breathe function
void writeAllPixels(uint8_t brightness) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(pixelColorsArray[i][0] * brightness / 255, 
                                          pixelColorsArray[i][1] * brightness / 255, 
                                          pixelColorsArray[i][2] * brightness / 255, 
                                          pixelColorsArray[i][3] * brightness / 255));
  }
  pixels.show();
}

// given values for one led it updates the array in memory 
void setPixelColor(int pixelIndex, uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  // Ensure the pixelIndex is within bounds
  if (pixelIndex >= 0 && pixelIndex < NUMPIXELS) {
    pixelColorsArray[pixelIndex][0] = red;
    pixelColorsArray[pixelIndex][1] = green;
    pixelColorsArray[pixelIndex][2] = blue;
    pixelColorsArray[pixelIndex][3] = white;
  }
}

// sets all the pixels in the array to the value n
void setAllPixels(int n){
  // Initialize the array of all pixelColorsArray values to value passed in e.g. to 0 at start
  for (int i = 0; i < NUMPIXELS; i++) {
    for (int j = 0; j < 4; j++) {
      pixelColorsArray[i][j] = n;
    }
  }
}

// sets all the pixels in the array to a random value
void setAllPixelsRandom(){
  for (int i = 0; i < NUMPIXELS; i++) {
    for (int j = 0; j < 3; j++) {
      pixelColorsArray[i][j] = random(20, 100);
    }
    pixelColorsArray[i][3] = 0; // turn off the white pixel
  }
}

// sets one LED to a random RGB colour (white is off)
void setOneRGBPixelRandom(){
  int p = random(0,NUMPIXELS);
  for (int j = 0; j < 3; j++) {
    pixelColorsArray[p][j] = random(20, 120);
  }
  pixelColorsArray[p][3] = 0; // turn off the white pixel
}

void pulseWhite(uint8_t wait) {
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire pixel with white at gamma-corrected brightness level 'j':
    pixels.fill(pixels.Color(0, 0, 0, pixels.gamma8(j)));
    pixels.show();
    delay(wait);
  }

  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    pixels.fill(pixels.Color(0, 0, 0, pixels.gamma8(j)));
    pixels.show();
    delay(wait);
  }
}

void breatheEffect(int loops, int speed) {
  // Define the breathing speed (adjust as needed)
  int breatheSpeed = speed;
  int breatheLimit = 30; // the value from 0-255

  for (int n = 0; n < loops; n++) {
    for (int i = 0; i < breatheLimit; i++) {
      int brightness = sin(i * 3.14159 / 256.0) * 255;
      writeAllPixels(brightness);
      delay(breatheSpeed);
    }

    for (int i = breatheLimit; i >= 0; i--) {
      int brightness = sin(i * 3.14159 / 256.0) * 255;
      writeAllPixels(brightness);
      delay(breatheSpeed);
    }
  }
}

void printPixelsValues() {
  for (int i = 0; i < NUMPIXELS; i++) {
    Serial.print(pixelColorsArray[i][0]); 
    Serial.print(","); 
    Serial.print(pixelColorsArray[i][1]); 
    Serial.print(","); 
    Serial.print(pixelColorsArray[i][2]); 
    Serial.print(","); 
    Serial.print(pixelColorsArray[i][3]); 
    Serial.println(" --- ");             
  }
}