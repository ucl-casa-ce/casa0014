
// Function to handle incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
}


// Connect wifi and get mac address for unique clientId and print out some setup info
void startWifi(){
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
    status = WiFi.begin(ssid, password);
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  LedGreen(); // all good so show green for go 
}




// handle reconnects to MQTT broker
void reconnectMQTT() {
  // Loop until we're reconnected
  LedBlue();
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to the topic you want to listen to
      mqttClient.subscribe(mqtt_topic.c_str());
      Serial.println("Subscribed to MQTT topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
  LedGreen();
}

