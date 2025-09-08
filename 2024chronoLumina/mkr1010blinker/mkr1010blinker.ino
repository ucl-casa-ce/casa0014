/* 
Chrono Lumina Blinker example for MKR1010
*/

#include <WiFiNINA.h>   
#include <PubSubClient.h>
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
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiServer server(80);
WiFiClient wificlient;

WiFiClient mkrClient;
PubSubClient client(mkrClient);

// edit this for the light you are connecting to
char mqtt_topic_demo[] = "student/CASA0014/light/1/pixel/";

void setup() {
  // Start the serial monitor to show output
  Serial.begin(115200);
  delay(1000);

  WiFi.setHostname("Lumina ucjtdjw");
  startWifi();
  client.setServer(mqtt_server, mqtt_port);
  Serial.println("setup complete");
}

void loop() {

  // we need to make sure the arduino is still connected to the MQTT broker
  // otherwise we will not receive any messages
  if (!client.connected()) {
    reconnectMQTT();
  }

  // we also need to make sure we are connected to the wifi
  // otherwise it will be impossible to connect to MQTT!
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }

  // check for messages from the broker and ensuring that any outgoing messages are sent.
  client.loop();

  sendmqtt();

  Serial.println("sent a message");
  delay(10000);
  
}

void sendmqtt(){

  // send a message to update the light
  char mqtt_message[100];
  sprintf(mqtt_message, "{\"pixelid\": %d, \"R\": 0, \"G\": 200, \"B\": 128, \"W\": 20}", 2);
  Serial.println(mqtt_topic_demo);
  Serial.println(mqtt_message);
  
  if (client.publish(mqtt_topic_demo, mqtt_message)) {
    Serial.println("Message published");
  } else {
    Serial.println("Failed to publish message");
  }

}

void startWifi(){
    
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
    Serial.println(ssid);
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

}

void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  } else {
    //Serial.println(WiFi.localIP());
  }

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
