#include <WiFiNINA.h>
#include "arduino_secrets.h" 
#include <utility/wifi_drv.h>   // library to drive to RGB LED on the MKR1010

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* ssid1         = SECRET_SSID1;
const char* password1     = SECRET_PASS1;
const char* ssid2         = SECRET_SSID2;
const char* password2     = SECRET_PASS2;

int status = WL_IDLE_STATUS;     // the Wifi radio's status


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);

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

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  printMacAddress(mac);

  startWifi();
}

void loop() {

}

void startWifi(){
  
  LedBlue(); // show Blue LED when looking for wifi

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
          // print your board's IP address:
          IPAddress ip = WiFi.localIP();
          Serial.print("IP Address: ");
          Serial.println(ip);
          LedGreen(); // all good so show green for go
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
          // print your board's IP address:
          IPAddress ip = WiFi.localIP();
          Serial.print("IP Address: ");
          Serial.println(ip);
          LedGreen(); // all good so show green for go
          break; // Exit the loop if connected
        } else {
          Serial.println("Failed to connect to " + String(ssid1));
        }
      // Secondary Network
      } else if (availablessid.equals(ssid2)) {
        Serial.print("Connecting to ");
        Serial.println(ssid2);
        WiFi.begin(ssid2, password2);
        while (WiFi.status() != WL_CONNECTED) {
          delay(600);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to " + String(ssid2));
          // print your board's IP address:
          IPAddress ip = WiFi.localIP();
          Serial.print("IP Address: ");
          Serial.println(ip);
          LedGreen(); // all good so show green for go
          break; // Exit the loop if connected
        } else {
          Serial.println("Failed to connect to " + String(ssid2));
        }
      // No Network!
      } else {
        Serial.print(availablessid);
        Serial.println(" - this network is not in my list");
        LedRed(); // no good 
      }

    }
  }
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