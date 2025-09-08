/*
    Duncan Wilson
    CASA0014 - 2 - Plant Monitor Workshop

    This version is adapted from the tutorial version to show the continuous monitoring - ie nails powered continuously
    
    Oct 2021
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wifi and MQTT
#include "arduino_secrets.h" 
/*
**** please enter your sensitive data in the Secret tab/arduino_secrets.h
**** using format below

#define SECRET_SSID "ssid name"
#define SECRET_PASS "ssid password"
#define SECRET_MQTTUSER "user name - eg student"
#define SECRET_MQTTPASS "password";
 */

const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

const char* mqtt_server = "mqtt.cetools.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


uint8_t soilPin = 0;    // we read a value between 0-1023 from analog pin
int moisture_val;       // the variable for storing the moisture level

int sensorVCC = 13;
int counter = 0;

void setup() {
  Serial.begin(115200);     //open serial port
  pinMode(sensorVCC, OUTPUT); 
  digitalWrite(sensorVCC, HIGH);

  // Initiate the connecting to wifi routine
  startWifi();

  // Once connected to wifi establish connection to mqtt broker
  client.setServer(mqtt_server, 1884);  
}

void loop() {
  delay(1000);
  // read the value from the sensor:
  moisture_val = analogRead(soilPin);   // read the resistance      
  Serial.print("sensor = " );                       
  Serial.println(moisture_val); 
  sendMQTT();
}


// This function is used to set-up the connection to the wifi
// using the user and passwords defined in the secrets file
// It then prints the connection status to the serial monitor

void startWifi(){
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // check to see if connected and wait until you are
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


// This function is used to make sure the arduino is connected
// to an MQTT broker before it tries to send a message and to 
// keep alive subscriptions on the broker (ie listens for inTopic)

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {    // while not (!) connected....
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// This function sends (publishes) a message to a MQTT topic
// once a connection is established with the broker. It sends
// an incrementing variable called value to the topic:
// "student/CASA0014/plant/ucjtdjw"

void sendMQTT() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  moisture_val = analogRead(soilPin);   // read the resistance      
  snprintf (msg, 50, "%ld", moisture_val);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucxxxxM/moisture", msg);

}
