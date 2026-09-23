// Connect wifi and print setup info
void startWifi() {
  LedBlue();
  Serial.print(F("Connecting to WiFi: "));
  Serial.println(ssid);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed!"));
    while (true)
      ;
  }

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5) {
    WiFi.begin(ssid, password);
    Serial.print(F("."));
    delay(3000);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nWiFi connected!"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
    LedGreen();
  } else {
    Serial.println(F("\nWiFi connection failed, will retry in loop."));
    LedRed();
  }
}

// Handle reconnects to MQTT broker
void reconnectMQTT() {
  LedBlue();
  wifiClient.stop();

  if (!mqttClient.connected()) {
    Serial.println(F("Connecting to MQTT..."));
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println(F("connected"));
      LedGreen();
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqttClient.state());
      Serial.println(F(" try again in 5 seconds"));
      LedRed();
      delay(5000);
    }
  }
}
