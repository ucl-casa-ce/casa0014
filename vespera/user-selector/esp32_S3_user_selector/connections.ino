


// Connect wifi and get mac address for unique clientId and print out some setup info
void startWifi(){

  neopixelWrite(RGB_BUILTIN,0,100,0); // GRB

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
      Serial.println(WiFi.SSID(i));
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
          neopixelWrite(RGB_BUILTIN,100,0,0); // GRB 
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
          neopixelWrite(RGB_BUILTIN,100,0,0); // GRB
          break; // Exit the loop if connected
        } else {
          Serial.println("Failed to connect to " + String(ssid1));
          neopixelWrite(RGB_BUILTIN,0,100,0); // GRB 
        }
      // No Network!
      } else {
        Serial.println("Couldn't find any configured networks");
        neopixelWrite(RGB_BUILTIN,0,0,100); // GRB 
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
  clientId = "CE-Vespera-" + macAddress;
  Serial.print("MQTT Client-id: ");  
  Serial.println(clientId);

}





// handle reconnects to MQTT broker
void reconnectMQTT() {
  // Loop until we're reconnected
  neopixelWrite(RGB_BUILTIN,0,100,0); // GRB 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to the topic you want to listen to
      client.subscribe(mqtt_topic.c_str());
      Serial.println("Subscribed to MQTT topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
  neopixelWrite(RGB_BUILTIN,100,0,0); // GRB
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

