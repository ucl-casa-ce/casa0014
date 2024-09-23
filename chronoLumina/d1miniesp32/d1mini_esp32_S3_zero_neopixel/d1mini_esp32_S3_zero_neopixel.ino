// Duncan Wilson June 2024 - v1 - MQTT controlled NeoPixel Ring
// Steven Gray - August 2024 - v1.1 - WebBluetooth Additions
// Duncan Wilson - September 2024 - v1.2 - uploaded to 50 ESP32-C3-Zero devices for CASA0014

// works with ESP32-S3-Zero and 12 Neopixel ring
// https://www.waveshare.com/wiki/ESP32-S3-Zero 
// https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html

// This code also works on ESP32 C3 Zero but pin layout is different - see github page for wiring
// https://www.waveshare.com/wiki/ESP32-C3-Zero

//***********************************************************************************************
// make sure to select USB CDC boot to enabled in tools menu
// if adding BLE to device make sure that partition scheme is set to Huge App in tools menu
//***********************************************************************************************

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 
#include "esp_mac.h"  // exposes esp_mac_type_t values

// USER SET VARIABLE FOR EACH DEPLOYMENT START
// Each device should connect to the feed that belongs to it
// e.g. light/1/ or light/2/ etc.
String lightId = "45"; // Or get this value from a sensor, user input, etc.
// USER SET VARIABLE FOR EACH DEPLOYMENT END

// BLE Libraries - Make sure ArduinoBLE is NOT installed (conflicts)
// This uses ESP32 BLE Library
#if (defined(BLE) && BLE == 1)
  #include "BLEDevice.h"
  #include "BLEServer.h"
  #include "BLEUtils.h"
  #include "BLE2902.h"

  BLECharacteristic *LEDCharacteristic;
  bool deviceConnected = false;
  bool restartAdvertising = false;

  class LEDWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic);
  };

  class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
      deviceConnected = true;
      restartAdvertising = false;
      Serial.println("Central Connected: Device Connected");
      
      // Flash All Blue
    };

    void onDisconnect(BLEServer *pServer) {
      deviceConnected = false;
      restartAdvertising = true;

      // Flash All Red

    }
  };

  class RGBWParser {
    public:
    int *parseRGBWString(String rgbString) {
      if (!rgbString.startsWith("[") || !rgbString.endsWith("]")) {
        Serial.println("Invalid RGBW string format: missing brackets.");
        return NULL;  // Indicate error
      }

      int commaCount = 0;
      for (int i = 0; i < rgbString.length(); i++) {
        if (rgbString.charAt(i) == ',') {
          commaCount++;
        }
      }

      if (commaCount != 3) {
        Serial.println("Invalid RGBW string format: wrong number of commas.");
        return NULL;  // Indicate error
      }

      // Extract values
      int commaIndex1 = rgbString.indexOf(',');
      int commaIndex2 = rgbString.indexOf(',', commaIndex1 + 1);
      int commeIndex3 = rgbString.indexOf(',', commaIndex2 + 1);
      String redStr = rgbString.substring(1, commaIndex1);  // Red first
      String greenStr = rgbString.substring(commaIndex1 + 1, commaIndex2);
      String blueStr = rgbString.substring(commaIndex2 + 1, commeIndex3);
      String whiteStr = rgbString.substring(commeIndex3 + 1, rgbString.length() - 1);

      int red = redStr.toInt();
      int green = greenStr.toInt();
      int blue = blueStr.toInt();
      int white = whiteStr.toInt();

      // Create and return the array
      int *rgbValues = new int[4];  // Dynamically allocate memory
      rgbValues[0] = red;
      rgbValues[1] = green;
      rgbValues[2] = blue;
      rgbValues[3] = white;

      return rgbValues;
    }
  };
#endif

/*
**** please enter your sensitive data in the Secret tab/arduino_secrets.h
**** using format below

#define SECRET_SSID "ssid name"
#define SECRET_PASS "ssid password"
#define SECRET_MQTTUSER "user name - eg student"
#define SECRET_MQTTPASS "password";
 */

// several topics are used to interact with the lights 
String mqtt_topic_pixel = "student/CASA0014/light/" + lightId + "/pixel/";            // used to control individual pixels on the ring
String mqtt_topic_all = "student/CASA0014/light/" + lightId + "/all/";                // used to control all LED's in one go
String mqtt_topic_brightness = "student/CASA0014/light/" + lightId + "/brightness/";  // used to set the brightness of all the LED's 
String mqtt_topic_demo = "student/CASA0014/light/" + lightId + "/demo/";            // used to control individual pixels on the ring
String mqtt_topic_fire = "student/CASA0014/light/" + lightId + "/fire/";            // used to control individual pixels on the ring
String clientId = ""; // will set once i have mac address so that it is unique

#define PIN         1   // data pin of neopixel 
#define NUMPIXELS   12  // length of neopixels
#define STARTBRIGHT 50  // starting value of brightness from 0 to 255
#define MAXBRIGHT   120 // brightness limited to 120 via MQTT

bool interactive = 1;   // when false run a demo

// create the pixels object for holding all the info about led values 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);

// Array to store RGBW values for each NeoPixel
// This is used so that I can maintain state (ie i can temporarily run animation and then return)
uint8_t pixelColorsArray[NUMPIXELS][4]; // Each element is an array [R, G, B, W]

#if (defined(WIFI_MQTT) && WIFI_MQTT == 1)
  const char* ssid          = SECRET_SSID;
  const char* password      = SECRET_PASS;
  const char* ssid1         = SECRET_SSID1;
  const char* password1     = SECRET_PASS1;
  const char* mqtt_username = SECRET_MQTTUSER;
  const char* mqtt_password = SECRET_MQTTPASS;
  const char* mqtt_server = "mqtt.cetools.org";
  const int mqtt_port = 1884;
#endif

// create wifi object and mqtt object
#if (defined(WIFI_MQTT) && WIFI_MQTT == 1)
  WiFiClient espClient;
  PubSubClient client(espClient);
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.print("MAC address: ");
  Serial.println(getDefaultMacAddress());
  Serial.print("Bluetooth: ");
  Serial.println(getInterfaceMacAddress(ESP_MAC_BT));
  delay(10);

  Serial.print("This device is Lumina");
  Serial.println(lightId);

  // configure neopixels
  pinMode(LED_BUILTIN, OUTPUT);
  pixels.begin();
  pixels.setBrightness(STARTBRIGHT);

  // BLE Server 
  #if (defined(BLE) && BLE == 1)
    Serial.println(F("Starting BLE work!"));

    BLEDevice::init("Lumina-BLE-" + lightId);
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pServer->setCallbacks(new MyServerCallbacks());

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE
                                        );

    BLEDescriptor *LEDDescriptor = new BLEDescriptor((uint16_t)0x2901); 
    LEDCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_LED_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

    LEDCharacteristic->setCallbacks(new LEDWriteCallback());
    LEDCharacteristic->addDescriptor(LEDDescriptor);

    LEDDescriptor->setValue("LED RGB Value [255,255,255]");   
    pCharacteristic->setValue("L 1 [255,255,255] 100");

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);

    restartAdvertising = false;
    BLEDevice::startAdvertising();
    Serial.println(F("All Characteristic defined!"));
  #endif

  // Connect to WiFi
  #if (defined(WIFI_MQTT) && WIFI_MQTT == 1)
    startWifi();
  #endif

  // quick test to see if all leds are working
  // i use this rather than pulseWhite so that i can see if all rgb leds are also working
  setAllPixels(100); // set all values to 100
  breatheEffect(1, 12); // 2 loops, 12 msec delay
  setAllPixels(0); // set all values to 0 "off"

  
  // Connect to MQTT broker
  #if (defined(WIFI_MQTT) && WIFI_MQTT == 1)
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(2000);
  client.setCallback(callback);
  #endif
  
  Serial.println("Set-up complete");

}
 
void loop() {
  // Reconnect if necessary
  #if (defined(WIFI_MQTT) && WIFI_MQTT == 1)
    if (!client.connected()) {
      reconnectMQTT();
    }
    
    if (WiFi.status() != WL_CONNECTED){
      startWifi();
    }
    // keep mqtt alive
    client.loop();
  #endif
  
  #if (defined(BLE) && BLE == 1)
    if (!deviceConnected) {
      if (restartAdvertising) {
        BLEDevice::startAdvertising();
        restartAdvertising = false;
        Serial.println("Central Disconnected: Restarting BLE ...");
      }
    }
  #endif


  writeAllPixels(255); // doesnt change the colour brightness since 255/255 = 1
  //writeAllPixels(random(200,250)); // creates a flicker effect since led brightness shifting between 200 and 255
  pixels.show();

}

// Function to handle incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // Check which topic the message is received from and then handle appropriately
  // If you want to add in more functions (new topics) then use format below
  if (strcmp(topic, mqtt_topic_pixel.c_str()) == 0) {
    handlePixelUpdate(payload, length);
  } else if (strcmp(topic, mqtt_topic_all.c_str()) == 0) {
    handleAllUpdate(payload, length);
  } else if (strcmp(topic, mqtt_topic_brightness.c_str()) == 0) {
    handleBrightness(payload, length);
  } else if (strcmp(topic, mqtt_topic_demo.c_str()) == 0) {
    demo(payload, length);
  } else if (strcmp(topic, mqtt_topic_fire.c_str()) == 0) {
    fire(payload, length);
  } else {
    Serial.println("Unknown topic");
  }
}

// Function to handle messages received on topic pixel
// This is used to update individual pixels
void handlePixelUpdate(byte* payload, unsigned int length) {

  // Parse JSON
  DynamicJsonDocument doc(200);
  deserializeJson(doc, payload, length);

  // on the topic:
  // student/CASA0014/light/2/pixel/
  
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
  //Serial.print("Pixel ID: ");
  //Serial.println(pixelid);
  //Serial.print("R: ");
  //Serial.println(R);
  //Serial.print("G: ");
  //Serial.println(G);
  //Serial.print("B: ");
  //Serial.println(B);
  //Serial.print("W: ");
  //Serial.println(W);
}

// Function to handle messages received on topic all
// This is used to update all the pixels in one go
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
  //Serial.println("Keys in the JSON document:");
  //printKeys(doc.as<JsonObject>());

  // on the topic:
  // student/CASA0014/light/2/all/

  //expecting to receive a JSON packet in this format
  //{
  //  "method": "clear"
  //}

  // or 

  //{
  //  "method": "allrandom"
  //}

  // Check if the method is "clear"
  if (doc.containsKey("method")) {
    const char* method = doc["method"];
    if (strcmp(method, "clear") == 0) {
      setAllPixels(0); // set all values to 0 "off"
    } else if (strcmp(method, "onerandom") == 0) {
      setOneRGBPixelRandom();
    } else if (strcmp(method, "allrandom") == 0) {
      setAllPixelsRandom();
    } else if (strcmp(method, "pulsewhite") == 0) {
      pulseWhite(10);
    } else {
      Serial.println("Unknown method");
      return;
    }
  }

  // Check if "allLEDs" array exists
  if (!doc.containsKey("allLEDs")) {
    Serial.println("Warning: Trying to change all but no 'allLEDs' array found in the JSON document.");
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

    // Array should look somethign like:
    /*
    {
      "allLEDs":[
          {
            "pixelid":0,
            "R":255,
            "G":0,
            "B":0,
            "W":0
          },
          {
            "pixelid":1,
            "R":231,
            "G":23,
            "B":0,
            "W":0
          },
    ...
          {
            "pixelid":11,
            "R":0,
            "G":255,
            "B":0,
            "W":0
          }
      ]
    }
    */

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
    //breatheEffect(1,50);
  }  
}

// Function to handle messages received on topic brightness
// this is used to make overall brightness more or less for indoor outdoor
void handleBrightness(byte* payload, unsigned int length) {

  // Parse JSON
  DynamicJsonDocument doc(200);
  deserializeJson(doc, payload, length);

  // on the topic:
  // student/CASA0014/light/2/brightness/

  //expecting to receive a JSON packet in this format
  //{
  //  "brightness": 50
  //}

  // Extract values
  int brightness = doc["brightness"];
  // i want to limit the brightness 
  if(brightness < MAXBRIGHT){
    pixels.setBrightness(brightness);
    pixels.show();
  }
  Serial.print("Current brightness setting is: ");
  Serial.println(pixels.getBrightness());
}

// Function to handle messages received on topic demo
void demo(byte* payload, unsigned int length) {

  // Parse JSON
  DynamicJsonDocument doc(200);
  deserializeJson(doc, payload, length);

  // on the topic:
  // student/CASA0014/light/2/demo/

  //expecting to receive a JSON packet in this format
  //{
  //  "spin": 50
  //}

  // Extract values
  int spin = doc["spin"];

  chase(spin);
}

// Function to handle messages received on topic demo
void fire(byte* payload, unsigned int length) {

  // Parse JSON
  DynamicJsonDocument doc(200);
  deserializeJson(doc, payload, length);

  // on the topic:
  // student/CASA0014/light/2/fire/

  //expecting to receive a JSON packet in this format
  //{
  //  "spin": 50
  //}

  // Extract values
  int spin = doc["spin"];
  
  firelighter(spin);
}

// Connect wifi and get mac address for unique clientId and print out some setup info
#if (defined(WIFI_MQTT) && WIFI_MQTT == 1)
void startWifi(){
  // Function for connecting to a WiFi network
  // is looking for UCL_IoT and a back up network (usually a home one!)
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    // loop through all the networks and if you find UCL_IoT or the backup - ssid1
    // then connect to wifi
    for (int i = 0; i < n; ++i){
      // Primary network
      if (WiFi.SSID(i) == ssid) {
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
          delay(600);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to " + String(ssid));
          break; // Exit the loop if connected
        } else {
          Serial.println("Failed to connect to " + String(ssid));
        }
      // Secondary Network
      } else if (WiFi.SSID(i) == ssid1) {
        Serial.print("Connecting to ");
        Serial.println(ssid1);
        WiFi.begin(ssid1, password1);
        while (WiFi.status() != WL_CONNECTED) {
          delay(600);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to " + String(ssid1));
          break; // Exit the loop if connected
        } else {
          Serial.println("Failed to connect to " + String(ssid1));
        }
      // No Network!
      } else {
        Serial.println("Couldn't find any configured networks");
      }

    }
  }


  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // need to create a unique id to connect to MQTT broker - will use mac address
  String macAddress = WiFi.macAddress();
  macAddress.replace(":", "-"); 
  clientId = "CE-NeoPixel-" + macAddress;
  Serial.print("MQTT Client-id: ");  
  Serial.println(clientId);

}

// handle reconnects to MQTT broker
void reconnectMQTT() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to the topic you want to listen to
      client.subscribe(mqtt_topic_pixel.c_str());
      client.subscribe(mqtt_topic_all.c_str());
      client.subscribe(mqtt_topic_brightness.c_str());
      client.subscribe(mqtt_topic_demo.c_str());
      client.subscribe(mqtt_topic_fire.c_str());  
      Serial.println("Subscribed to MQTT topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

#endif

// loops through all stored pixel values and runs pixels.show to update LED's
// brightness can be used to dim the values - e.g. is used in breathe function
void writeAllPixels(uint8_t brightness) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(pixelColorsArray[i][0] * brightness / 255, 
                                          pixelColorsArray[i][1] * brightness / 255, 
                                          pixelColorsArray[i][2] * brightness / 255, 
                                          pixelColorsArray[i][3] * brightness / 255));
  }
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

// pulses all leds white
void pulseWhite(uint8_t wait) {
  Serial.println("pulse white");
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

void pulseWhiteNoDelay(uint8_t brightnessStep) {
}

void pulseBlue(int r, int g, int b) {
  static unsigned long previousMillis = 0;
  const long interval = 500; // Interval between flashes

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Toggle the pixel's color
    if (pixels.getPixelColor(0) == 0) {
      pixels.setPixelColor(0, r, g, b); // Blue color
    } else  {
      pixels.setPixelColor(0, 0, 0, 0); // Turn off
    }
    pixels.show();
  }
}

// used at start up to show device is alive
void breatheEffect(int loops, int speed) {
  // Define the breathing speed (adjust as needed)
  int breatheSpeed = speed;
  int breatheLimit = 30; // the value from 0-255

  for (int n = 0; n < loops; n++) {
    for (int i = 0; i < breatheLimit; i++) {
      int brightness = sin(i * 3.14159 / 256.0) * 255;
      writeAllPixels(brightness);
      pixels.show();
      delay(breatheSpeed);
    }

    for (int i = breatheLimit; i >= 0; i--) {
      int brightness = sin(i * 3.14159 / 256.0) * 255;
      writeAllPixels(brightness);
      pixels.show();
      delay(breatheSpeed);
    }
  }
}

void chase(uint8_t cycles) {

  uint16_t i, j;

  for(j=0; j<256*cycles; j++) { // n cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(2);
  }

}

void firelighter(uint8_t cycles) {
  for(int j=0; j < cycles; j++){
    for(int i=0; i< pixels.numPixels(); i++) {
      int brightness = random(10, MAXBRIGHT); // Random brightness for each pixel
      int red = random(150, 255); // Random red value
      int green = random(100, 200); // Random green value

      pixels.setPixelColor(i, pixels.Color(red, 0, 0)); // Set pixel color to red and green (yellow)
      pixels.setBrightness(brightness);
      pixels.show();

    }
    delay(50);
    
  }
  
}

// helper function to print out keys in a json object
void printKeys(const JsonObject& obj) {
  for (JsonPair kv : obj) {
    Serial.print("Key: ");
    Serial.println(kv.key().c_str());
  }
}

// helper function to print out current values in the pixel colour array
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

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos; // Reverse color wheel direction for a more natural effect
  if (WheelPos < 85) {
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
}

// function to return the MAC address of the device
String getDefaultMacAddress() {
  String mac = "";
  unsigned char mac_base[6] = {0};
  if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
    char buffer[18];  // 6*2 characters for hex + 5 characters for colons + 1 character for null terminator
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    mac = buffer;
  }
  return mac;
}

// function to return the MAC address of the device - BT
String getInterfaceMacAddress(esp_mac_type_t interface) {
  String mac = "";
  unsigned char mac_base[6] = {0};
  if (esp_read_mac(mac_base, interface) == ESP_OK) {
    char buffer[18];  // 6*2 characters for hex + 5 characters for colons + 1 character for null terminator
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    mac = buffer;
  }
  return mac;
}

#if (defined(BLE) && BLE == 1)
  
  void processJSON(String payload){
        Serial.println(payload);
        payload = "{\"allLEDs\":[{\"pixelid\":0,\"R\":69,\"G\":156,\"B\":239,\"W\":0},{\"pixelid\":1,\"R\":69,\"G\":156,\"B\":239,\"W\":0},{\"pixelid\":2,\"R\":110,\"G\":194,\"B\":67,\"W\":0},{\"pixelid\":3,\"R\":110,\"G\":194,\"B\":67,\"W\":0},{\"pixelid\":4,\"R\":255,\"G\":209,\"B\":5,\"W\":0},{\"pixelid\":5,\"R\":255,\"G\":213,\"B\":5,\"W\":0},{\"pixelid\":6,\"R\":255,\"G\":170,\"B\":2,\"W\":0},{\"pixelid\":7,\"R\":251,\"G\":168,\"B\":1,\"W\":0},{\"pixelid\":8,\"R\":255,\"G\":91,\"B\":80,\"W\":0},{\"pixelid\":9,\"R\":255,\"G\":77,\"B\":67,\"W\":0},{\"pixelid\":10,\"R\":191,\"G\":101,\"B\":213,\"W\":0},{\"pixelid\":11,\"R\":183,\"G\":85,\"B\":207,\"W\":0}]}";
        DynamicJsonDocument doc(2000);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }

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
        // for (int j = 0; j < NUMPIXELS; j++) {
        //   Serial.print("Color ");
        //   Serial.print(j);
        //   Serial.print(": Pixel ID: ");
        //   Serial.print(j);
        //   Serial.print(", R: ");
        //   Serial.print(pixelColorsArray[j][0]);
        //   Serial.print(", G: ");
        //   Serial.print(pixelColorsArray[j][1]);
        //   Serial.print(", B: ");
        //   Serial.print(pixelColorsArray[j][2]);
        //   Serial.print(", W: ");
        //   Serial.println(pixelColorsArray[j][3]);
        // }

        writeAllPixels(100);
  }

  void LEDWriteCallback::onWrite(BLECharacteristic *pCharacteristic) {
        String functionString = pCharacteristic->getValue();
        char functionChar = functionString[0]; 
        const char* jsonChar = functionString.c_str();
        char charArray[50];
        functionString.toCharArray(charArray, sizeof(charArray));
      
        // Init the variables to split the words we need
        char *word;
        int *rgbValues;
        String rgbString;
        String JSONString;
        RGBWParser parser;
        int r, g, b, w = 0;
        String words[4]; // Assuming maximum 4 words
        int wordCount = 0;

        // Process the written data here
        switch (functionChar) {
          case 'j':
          case 'J':
            JSONString = functionString.substring(2, functionString.length());
            processJSON(JSONString);
            break;
          case 'L':
          case 'l':
           // Split string
            word = strtok(charArray, " ");
            while (word != NULL && wordCount < 4) {
              words[wordCount] = word;
              wordCount++;
              word = strtok(NULL, " ");
            }

            Serial.println("CODE:" + String(words[0]) + "  PIXID: " + String(words[1]) + "  RGBW: " + words[2] + "  BRIGHTNESS: ") + String(words[3]);
            
            //  String should be "L PIXID [R,G,B] BRIGHTNESS"
            // L 1 [255,255,0,0] 50
            rgbValues = parser.parseRGBWString(words[2]);

            if (rgbValues != NULL) {
                r = rgbValues[0];
                g = rgbValues[1];
                b = rgbValues[2];
                w = rgbValues[3];
                Serial.println("RED:" + String(r) + "  GREEN: " + String(g) + "  BLUE: " + String(b) + "  WHITE: " + String(w));

                setPixelColor(atoi(words[1].c_str()), r, g,  b, w);
                pixels.setBrightness(atoi(words[3].c_str()));
            }

            LEDCharacteristic->setValue("FUNCTION SET: setPixelColor ");
            LEDCharacteristic->notify();
            break;
          case 'r':
          case 'R':
            setAllPixelsRandom(); 
            LEDCharacteristic->setValue("FUNCTION SET: setAllPixelsRandom");
            LEDCharacteristic->notify();
            break;
          case 'O':
          case 'o':
            setOneRGBPixelRandom(); 
            LEDCharacteristic->setValue("FUNCTION SET: setOneRGBPixelRandom");
            LEDCharacteristic->notify();
            break;
          case 'b':
          case 'B':
            //breatheEffect(1,50);
            LEDCharacteristic->setValue("FUNCTION SET: breatheEffect");
            LEDCharacteristic->notify();
            break;
          case 'A':
          case 'a':
            LEDCharacteristic->setValue("FUNCTION SET: Brightness 100");
            setAllPixels(100);
            LEDCharacteristic->notify();
            break;
          case 'F':
          case 'f':
            LEDCharacteristic->setValue("FUNCTION SET: Brightness 0");
            setAllPixels(0);
            LEDCharacteristic->notify();
            break;
          case 'p':
          case 'P':
            pulseWhiteNoDelay(30);
            LEDCharacteristic->setValue("FUNCTION SET: pulseEffect");
            LEDCharacteristic->notify();
            break;
          default:
            // ERROR Case
            LEDCharacteristic->setValue("ERROR: COULDN'T FIND FUNCTION");
            LEDCharacteristic->notify();
            break;
        }
        Serial.println("LEDCharacteristic written: " + functionString);
  }
#endif
