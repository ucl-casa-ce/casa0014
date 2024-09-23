/* Based on Oleg Mazurov's code for rotary encoder interrupt service routines for AVR micros
   here https://chome.nerpa.tech/mcu/reading-rotary-encoder-on-arduino/
   and using interrupts https://chome.nerpa.tech/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros/

   This example does not use the port read method. Tested with Nano and ESP32
   both encoder A and B pins must be connected to interrupt enabled pins, see here for more info:
   https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

   Great Explanation of Rotary Encoders here: https://www.youtube.com/watch?v=fgOfSHTYeio 


   on RE - middle pin is GND left and right are clockwise and anticlockwise rotation depending 
   on which is connected to ENC_A or B pins
*/


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // Note when using the adafruit examples i needed to change the address to 0x3C
#include <WiFiNINA.h>   
#include <PubSubClient.h>
#include <utility/wifi_drv.h>   // library to drive to RGB LED on the MKR1010
#include "arduino_secrets.h" 

#define SCREEN_WIDTH 128 // Adjust for your display
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

// Define rotary encoder pins - check which pins have interupts - these are for MKR boards
#define ENC_A 4
#define ENC_B 5

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int counter = 1;
int upperlimit = 50;
int lowerlimit = 0;

// Structure to store circle information
struct Circle {
  int x, y, radius;
};

// Maximum number of circles
const int maxCircles = 8;

// Array to store circle information
Circle circles[maxCircles];

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
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiServer server(80);
WiFiClient wificlient;

WiFiClient mkrClient;
PubSubClient client(mkrClient);

void setup() {

  // RGB LED's
  WiFiDrv::pinMode(25, OUTPUT); // G
  WiFiDrv::pinMode(26, OUTPUT); // R
  WiFiDrv::pinMode(27, OUTPUT); // B  

  LedRed(); // board alive but not yet ready

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // these amazon screens are different address to adafruit ones
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  displayhello();

  // Initialize circles
  for (int i = 1; i < maxCircles; i++) {
    circles[i].x = SCREEN_WIDTH / 2;
    circles[i].y = SCREEN_HEIGHT / 2;
    circles[i].radius = i*5;
  }

  // Set encoder pins and attach interrupts
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);

  pinMode(8, INPUT_PULLUP);  // Pin 8 reads 1 if not connected to GND
  pinMode(LED_BUILTIN, OUTPUT); // Use the built in LED to check status

  // Start the serial monitor to show output
  Serial.begin(115200);
  delay(1000);

  WiFi.setHostname("LuminaSelector");
  startWifi();
  client.setServer(mqtt_server, mqtt_port);
//  client.setCallback(callback);

}

void loop() {

  if (!client.connected()) {
    reconnectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  // keep mqtt alive
  client.loop();

  static int lastCounter = 0;

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
  
  display.clearDisplay();
  displaytitle();
  displaycounter(counter);

  digitalWrite(LED_BUILTIN, digitalRead(8)); // echo status of pin 8 input on the built in LED

  if(!digitalRead(8)){
    sendmqtt(counter);
    wormhole(120);
  }
}

void sendmqtt(int LEDring){

  char mqtt_topic_demo[50];

  if(LEDring < 1){
    for(int i = 1; i <= upperlimit; i++){
      sprintf(mqtt_topic_demo, "student/CASA0014/light/%d/all/", i);
    
      String message = "{\"method\": \"pulsewhite\"}";

      if (client.publish(mqtt_topic_demo, message.c_str(), message.length())) {
        Serial.println("Message published");
      } else {
        Serial.println("Failed to publish message");
      }

    }

  }else{

    sprintf(mqtt_topic_demo, "student/CASA0014/light/%d/all/", LEDring);
  
    String message = "{\"method\": \"pulsewhite\"}";

    if (client.publish(mqtt_topic_demo, message.c_str(), message.length())) {
      Serial.println("Message published");
    } else {
      Serial.println("Failed to publish message");
    }
  }

}

void displaytitle(){
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Select LED");
}

void displaywifititle(){
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Looking for Wifi");
}

void displaymqtttitle(){
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Reconnecting to MQTT");
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

void displayhello(){
  display.setCursor(3, 20); // Reset cursor position
  display.setTextSize(3);
  display.print("Hello!");
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

void startWifi(){
  
  LedBlue(); // show Blue LED when looking for wifi
  displaywifititle();
  display.display(); // Update the display
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

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
    Serial.print("Trying to connect to: ");
    Serial.println(ssid1);
    for (int i = 0; i < n; ++i){
      String availablessid = WiFi.SSID(i);
      // Primary network
      if (availablessid.equals(ssid)) {
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
      } else if (availablessid.equals(ssid1)) {
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
        Serial.print(availablessid);
        Serial.println(" - this network is not in my list");
      }

    }
  }


  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  LedGreen(); // all good so show green for go
}


void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  } else {
    //Serial.println(WiFi.localIP());
  }
  LedBlue();
  displaymqtttitle();
  display.display(); // Update the display

  // Loop until we're reconnected
  while (!client.connected()) {    // while not (!) connected....
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "LuminaSelector";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // ... and subscribe to messages on broker
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  LedGreen();
}

void callback(char* topic, byte* payload, int length) {
  // Handle incoming messages
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}
