// Duncan Wilson Oct 2025 - v1 - MQTT messager to vespera

// works with ESP32-C3-Zero
// https://www.waveshare.com/wiki/ESP32-C3-Zero

//***********************************************************************************************
// make sure to select USB CDC boot to enabled in tools menu
//***********************************************************************************************

/* 
Pinout diagram: https://www.espboards.dev/esp32/esp32-s3-zero/

Rotary Encoder:
black wires to ground
red to 4
green to 5
yellow to 6

OLED screen
gnd to gnd
vdd to 3.3v
SDA to 8
SCK to 9
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_mac.h"  // exposes esp_mac_type_t values
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // Note when using the adafruit examples i needed to change the address to 0x3C
#include "arduino_secrets.h" 

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

// create wifi object and mqtt object
WiFiClient espClient;
PubSubClient client(espClient);

// Here we define the MQTT topic we will be publishing data to
String mqtt_topic = "student/CASA0014/luminaire/user";            
String clientId = ""; // will set once i have mac address so that it is unique
int vesperaUser = 0; // this holds current value of Vespera User on MQTT broker


// Rotary Encoder Variables
// Define rotary encoder pins - check which pins have interupts - these are for MKR boards
#define ENC_A 5
#define ENC_B 6
#define BUTT  4
unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;
volatile int counter = 0;
int upperlimit = 50;
int lowerlimit = 0;

// OLED screen
#define SCREEN_WIDTH 128 // Adjust for your display
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

// This is some setup for the circles animation when button is pressed
// Structure to store circle information
struct Circle {
  int x, y, radius;
};
// Maximum number of circles
const int maxCircles = 8;
// Array to store circle information
Circle circles[maxCircles];

// use the onboard LED to show status of connection
#ifdef RGB_BUILTIN
#undef RGB_BUILTIN
#endif
#define RGB_BUILTIN 21

void setup() {
  Serial.begin(115200);
  delay(1000);

  neopixelWrite(RGB_BUILTIN,0,100,0); // GRB

  Serial.print("MAC address: ");
  Serial.println(getDefaultMacAddress());

  Serial.print("This device is Vespera User Selector");

  neopixelWrite(RGB_BUILTIN,0,0,100); // GRB

  // Connect to WiFi
  startWifi();

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(2000);
  client.setCallback(callback);

  // Set encoder pins and attach interrupts
  Serial.println("Set-up rotary encoder");
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);
  pinMode(BUTT, INPUT_PULLUP);  // Pin 4 reads 1 if not connected to GND

  // Screen setup - Use the default I2C pins for the ESP32-S3-Zero
  // SDA: GPIO 8, SCL: GPIO 9
  Serial.println("Set-up screen");
  Wire.begin(8,9); 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // these amazon screens are different address to adafruit ones
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  // Initialize circles
  for (int i = 1; i < maxCircles; i++) {
    circles[i].x = SCREEN_WIDTH / 2;
    circles[i].y = SCREEN_HEIGHT / 2;
    circles[i].radius = i*5;
  }

  Serial.println("Set-up complete");
  neopixelWrite(RGB_BUILTIN,100,0,0); // GRB
}
 
void loop() {

  static int lastCounter = 0; // this holds current value of encoder display on device

  // Reconnect if necessary
  if (!client.connected()) {
    reconnectMQTT();
  }
  
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  // keep mqtt alive
  client.loop();

  // Rotary Encoder
  // If count has changed print the new value to serial
  if(counter != lastCounter){
    if (counter > upperlimit) {
      counter = upperlimit;
    } else if (counter < lowerlimit) {
      counter = lowerlimit;
    }    
    Serial.println(counter);
    lastCounter = counter;
  }
  if(!digitalRead(BUTT)){
    neopixelWrite(RGB_BUILTIN,0,100,0); // GRB
    vesperaUser = counter;
    Serial.print("vesperaUser:");
    Serial.print(vesperaUser);
    Serial.print(" sending to MQTT");    
    // Publish user value to MQTT broker
    // Convert the integer to a String
    String payloadStr = String(vesperaUser);

    // Publish the String as a character array
    // .c_str() converts the String to a const char*
    // Publish the String with a 'true' flag for retained message
    // The syntax is: publish(topic, payload, retained)
    client.publish(mqtt_topic.c_str(), payloadStr.c_str(), true);

    delay(100);
    neopixelWrite(RGB_BUILTIN,100,0,0); // GRB
  }

  // Screen
  display.clearDisplay();
  displaytitle(vesperaUser);
  displaycounter(lastCounter);  

}

// Function to handle incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // Check if the received topic is the one we care about
  if (strcmp(topic, mqtt_topic.c_str()) == 0) {
    // Create a buffer to hold the payload string
    char payloadStr[length + 1];
    // Copy the payload bytes into the string buffer
    memcpy(payloadStr, payload, length);
    // Add a null terminator to make it a valid C-string
    payloadStr[length] = '\0';

    // Convert the string to an integer
    int receivedValue = atoi(payloadStr);

    // Update the global vesperaUser variable
    vesperaUser = receivedValue;

    wormhole(80);

    Serial.print("Updated vesperaUser to: ");
    Serial.println(vesperaUser);
  }

}


void displaytitle(int user){
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Select User (mqtt=");
  display.print(user);
  display.print(")");
  
}

void displaycounter(int counter){
  // Convert counter to a string for display
  String counterStr = String(counter);

  // Pad with leading zeros if necessary
  if (counter < 10) {
    counterStr = "0" + counterStr;
  }
  
  display.setCursor(30, 20); // Reset cursor position
  display.setTextSize(6);
  display.print(counterStr);

  display.drawRect((counter*2)+14, 12, 5, 2, WHITE);

  display.display(); // Update the display

}

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
} 

void wormhole(int cycles){
  // Starting position at the center
  int x = SCREEN_WIDTH / 2;
  int y = SCREEN_HEIGHT / 2;
  int circlestart = x; 
  int direction = 1;

  for (int i = 0; i < cycles; i++) {
    display.clearDisplay();

    circlestart += direction;

    if(circlestart < x - 20){
      direction = 1; 
    }else if(circlestart > x+20){
      direction = -1; 
    } 
 
    // Move circles and draw them
    for (int i = 1; i < maxCircles; i++) {
      circles[i].radius *= 1.2; // Adjust expansion rate
      display.drawCircle(circles[i].x, circles[i].y, circles[i].radius, WHITE);
      
      if (circles[i].radius > SCREEN_WIDTH / 2) {
        // Reset the circle
        circles[i].radius = 6;
        circles[i].x = circlestart;
      }

    }
    display.display();
    delay(10);
  }

}