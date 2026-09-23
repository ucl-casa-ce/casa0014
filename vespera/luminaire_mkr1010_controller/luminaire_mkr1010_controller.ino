// This sketch allows an Arduino MKR1010 to control a string of 72 NeoPixel RGB
// LEDs via MQTT. It subscribes to a specific topic and interprets binary
// payloads as RGB values for each LED.
//
// Enhanced with:
// - Instant packet queue draining (eliminates 30-60s user change latency)
// - Authenticated MQTT connection (Port 1884 with credentials)
// - SAMD21 Boot/Reset Reason diagnosis (BOD33 brownout, WDT watchdog, POR)
// - SAMD21 Hardware Watchdog Timer (16s auto-recovery)
// - Free SRAM and Loop Heartbeat telemetry
// - Retained MQTT Status message published to student/CASA0014/luminaire/status
// - MQTT Last Will and Testament (LWT) for offline detection
// - NeoPixel frame rate throttling to prevent SPI / WiFiNINA lockups

// --- Libraries ---
// You'll need to install these libraries via the Arduino IDE's Library Manager:
// 1. WiFiNINA by Arduino: For Wi-Fi connectivity on MKR boards.
// 2. PubSubClient by Nick O'Leary: For MQTT communication.
// 3. Adafruit NeoPixel by Adafruit: For controlling NeoPixel LEDs.

#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>
#include <SPI.h> // Required for WiFiNINA
#include <WiFiNINA.h>
#include <utility/wifi_drv.h> // library to drive RGB LED on the MKR1010

#include "arduino_secrets.h" // so that you can exclude these from your GitHub repo

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS; // the Wifi radio's status

// --- MQTT Broker Configuration ---
const char *mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884; // Authenticated MQTT port
const char *mqtt_username = SECRET_MQTTUSER;
const char *mqtt_password = SECRET_MQTTPASS;
const char *mqtt_client_id = "MKR1010_NeoPixel_Luminaire_Vespera"; // Unique client ID for your device

// --- MQTT Topic Configuration ---
// Base topic for the entire luminaire group
const char *mqtt_base_topic = "student/CASA0014/luminaire";

// The full topic string for THIS device's updates (will be generated dynamically)
char mqtt_data_topic[64];

// Fixed control & telemetry topics
const char *user_update_topic = "student/CASA0014/luminaire/user";
const char *brightness_update_topic = "student/CASA0014/luminaire/brightness";
const char *status_topic = "student/CASA0014/luminaire/status";

// --- Use this variable to see if we should change lights or not
// --- Defaults to 0 - staff user - on start up
int LUMINAIRE_USER = 0;
int LUMINAIRE_BRIGHTNESS = 150;

// Pending state change flags (processed safely in loop(), outside MQTT callback)
volatile bool pendingUserChange = false;
volatile int pendingUserId = 0;

volatile bool pendingBrightnessChange = false;
volatile int pendingBrightness = 150;

// String to store human-readable reset reason for status telemetry
char resetReasonStr[32] = "Unknown";

// --- NeoPixel Configuration ---
#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 72
#define NEOPIXEL_DATA_LENGTH (NEOPIXEL_COUNT * 3) // 3 bytes per LED (R, G, B)

// Double-buffer for NeoPixel RGB data
byte ledDataBuffer[NEOPIXEL_DATA_LENGTH];
volatile bool newPixelDataAvailable = false;

// Frame-rate throttle: minimum time between pixels.show() calls (~33 FPS)
// Prevents frequent interrupt disablement from corrupting WiFiNINA SPI transfers
unsigned long lastPixelRender = 0;
const unsigned long MIN_PIXEL_INTERVAL_MS = 30;

// --- Global Objects ---
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// --- Connection & Telemetry Timers ---
unsigned long lastConnectionAttempt = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastStatusPublish = 0;
unsigned long lastMessageReceivedTime = 0;
unsigned long totalMessagesReceived = 0;

const unsigned long RECONNECT_INTERVAL_MS = 5000;
const unsigned long WIFI_CHECK_INTERVAL_MS = 5000;
const unsigned long HEARTBEAT_INTERVAL_MS = 10000; // Local serial log interval
const unsigned long STATUS_PUBLISH_INTERVAL_MS = 60000; // MQTT network publish interval

// --- Function Prototypes ---
void setup_wifi();
bool reconnect_mqtt();
void mqtt_callback(char *topic, byte *payload, unsigned int length);
void publish_status();
void LedRed();
void LedGreen();
void LedBlue();
void toggleRGB();
void printResetReason();
int getFreeRam();
void watchdogSetup();
void watchdogReset();

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Status RGB LED pins on MKR1010 NINA module
  WiFiDrv::pinMode(25, OUTPUT); // R
  WiFiDrv::pinMode(26, OUTPUT); // G
  WiFiDrv::pinMode(27, OUTPUT); // B

  LedRed(); // Board alive but not yet connected

  // Delay slightly to give Serial time to enumerate if USB monitor is opened
  delay(1000);

  Serial.println(F("\n=================================================="));
  Serial.println(F("Starting MKR1010 NeoPixel MQTT Controller..."));
  Serial.println(F("=================================================="));

  // Diagnostic: Report why the microcontroller reset (Brownout, Watchdog, Power, etc.)
  printResetReason();

  Serial.print(F("Initial Free RAM: "));
  Serial.print(getFreeRam());
  Serial.println(F(" bytes"));

  // Initialize NeoPixels
  pixels.begin();
  pixels.setBrightness(LUMINAIRE_BRIGHTNESS);
  pixels.clear();
  pixels.show();

  // Allocate MQTT buffer size to 2048 bytes
  mqttClient.setBufferSize(2048);
  mqttClient.setKeepAlive(30);
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqtt_callback);

  // Generate the initial data subscription topic: "student/CASA0014/luminaire/0"
  snprintf(mqtt_data_topic, sizeof(mqtt_data_topic), "%s/%d", mqtt_base_topic, LUMINAIRE_USER);
  Serial.print(F("Initial data topic set to: "));
  Serial.println(mqtt_data_topic);

  // Connect to Wi-Fi
  setup_wifi();

  // Enable the SAMD21 Hardware Watchdog Timer (16 second timeout)
  watchdogSetup();
  Serial.println(F("SAMD21 Hardware Watchdog enabled (16s timeout)."));
}

void loop() {
  // Feed the hardware watchdog to prevent reset during normal execution
  watchdogReset();

  unsigned long now = millis();

  // Check Wi-Fi connection periodically
  if (now - lastWifiCheck > WIFI_CHECK_INTERVAL_MS) {
    lastWifiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("Wi-Fi is disconnected. Re-running setup_wifi..."));
      setup_wifi();
      return;
    }
  }

  // Ensure MQTT connection is maintained
  if (!mqttClient.connected()) {
    if (now - lastConnectionAttempt > RECONNECT_INTERVAL_MS) {
      lastConnectionAttempt = now;
      reconnect_mqtt();
    }
  } else {
    // Process incoming MQTT messages and immediately drain any queued packets in the WiFi buffer
    mqttClient.loop();
    while (wifiClient.available() > 0 && mqttClient.connected()) {
      mqttClient.loop();
    }
  }

  // Handle deferred user topic changes immediately
  if (pendingUserChange) {
    pendingUserChange = false;

    // Unsubscribe from old topic
    Serial.print(F("Unsubscribing from old topic: "));
    Serial.println(mqtt_data_topic);
    mqttClient.unsubscribe(mqtt_data_topic);

    // Update global LUMINAIRE_USER
    LUMINAIRE_USER = pendingUserId;

    // Generate new topic string
    snprintf(mqtt_data_topic, sizeof(mqtt_data_topic), "%s/%d", mqtt_base_topic, LUMINAIRE_USER);

    // Subscribe to new topic
    if (mqttClient.subscribe(mqtt_data_topic)) {
      Serial.print(F("Subscribed to NEW topic: "));
      Serial.println(mqtt_data_topic);
    } else {
      Serial.println(F("Failed to subscribe to NEW topic!"));
    }

    Serial.print(F("LUMINAIRE_USER updated to: "));
    Serial.println(LUMINAIRE_USER);

    // Clear LEDs immediately on user change
    pixels.clear();
    pixels.show();
    newPixelDataAvailable = false; // Discard any old pending frame from previous user

    // Publish updated status immediately over MQTT
    publish_status();
  }

  // Handle deferred brightness changes safely outside callback
  if (pendingBrightnessChange) {
    pendingBrightnessChange = false;
    LUMINAIRE_BRIGHTNESS = pendingBrightness;
    pixels.setBrightness(LUMINAIRE_BRIGHTNESS);
    pixels.show();
    Serial.print(F("LUMINAIRE_BRIGHTNESS updated to: "));
    Serial.println(LUMINAIRE_BRIGHTNESS);

    // Publish updated status immediately over MQTT
    publish_status();
  }

  // Update NeoPixels outside callback with frame rate throttling (~30 FPS)
  // This prevents back-to-back interrupt disabling which can stall WiFiNINA SPI communications
  if (newPixelDataAvailable && (now - lastPixelRender >= MIN_PIXEL_INTERVAL_MS)) {
    lastPixelRender = now;
    newPixelDataAvailable = false;

    for (int i = 0; i < NEOPIXEL_COUNT; i++) {
      byte r = ledDataBuffer[i * 3 + 0];
      byte g = ledDataBuffer[i * 3 + 1];
      byte b = ledDataBuffer[i * 3 + 2];
      pixels.setPixelColor(i, r, g, b);
    }
    pixels.show();
  }

  // Periodic Telemetry & Heartbeat Log to Serial (Every 10 seconds)
  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeat = now;
    Serial.print(F("[HEARTBEAT] Uptime: "));
    Serial.print(now / 1000);
    Serial.print(F("s | Free RAM: "));
    Serial.print(getFreeRam());
    Serial.print(F("B | WiFi RSSI: "));
    Serial.print(WiFi.RSSI());
    Serial.print(F(" dBm | MQTT: "));
    Serial.print(mqttClient.connected() ? F("OK") : F("DISCONNECTED"));
    Serial.print(F(" | Active User: "));
    Serial.print(LUMINAIRE_USER);
    Serial.print(F(" | Msgs: "));
    Serial.print(totalMessagesReceived);
    if (lastMessageReceivedTime > 0) {
      Serial.print(F(" (Last msg "));
      Serial.print((now >= lastMessageReceivedTime) ? (now - lastMessageReceivedTime) / 1000 : 0);
      Serial.print(F("s ago)"));
    }
    Serial.println();
  }

  // Periodic Background Retained MQTT Status Publish (Every 60 seconds)
  if (now - lastStatusPublish >= STATUS_PUBLISH_INTERVAL_MS) {
    lastStatusPublish = now;
    publish_status();
  }
}

// --- WiFi Setup Function ---
void setup_wifi() {
  LedBlue(); // Show Blue LED when connecting to Wi-Fi
  Serial.print(F("Connecting to WiFi: "));
  Serial.println(ssid);

  // Check if the WiFi module is present
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed!"));
    while (true) {
      // Loop forever, watchdog will trigger reset if enabled
    }
  }

  // Attempt to connect to WiFi network (non-infinite loop to prevent permanent hang)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5) {
    watchdogReset();
    WiFi.begin(ssid, pass);
    watchdogReset();
    Serial.print(F("."));
    delay(2000);
    watchdogReset();
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nWiFi connected!"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
    LedGreen();
  } else {
    Serial.println(F("\nWiFi connection failed, will retry in main loop."));
    LedRed();
  }
  watchdogReset();
}

// --- MQTT Reconnection Function ---
bool reconnect_mqtt() {
  watchdogReset();
  LedBlue();
  Serial.print(F("Attempting MQTT connection..."));

  // Ensure stale socket is cleaned up before opening a new connection
  wifiClient.stop();
  watchdogReset();

  // Connect with username, password, and Last Will and Testament (LWT) on status topic with retain=true
  // If connection drops ungracefully, broker publishes {"status":"offline"}
  const char *lwt_payload = "{\"status\":\"offline\"}";
  if (mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password, status_topic, 0, true, lwt_payload)) {
    Serial.println(F("connected!"));
    watchdogReset();

    bool subscribed_ok = true;

    // 1. Subscribe to the 'user' update topic
    if (mqttClient.subscribe(user_update_topic)) {
      Serial.print(F("Subscribed to user topic: "));
      Serial.println(user_update_topic);
    } else {
      Serial.println(F("Failed to subscribe to user topic!"));
      subscribed_ok = false;
    }
    watchdogReset();

    // 2. Subscribe to the 'brightness' update topic
    if (mqttClient.subscribe(brightness_update_topic)) {
      Serial.print(F("Subscribed to brightness topic: "));
      Serial.println(brightness_update_topic);
    } else {
      Serial.println(F("Failed to subscribe to brightness topic!"));
      subscribed_ok = false;
    }
    watchdogReset();

    // 3. Subscribe to the device's specific data topic
    if (mqttClient.subscribe(mqtt_data_topic)) {
      Serial.print(F("Subscribed to data topic: "));
      Serial.println(mqtt_data_topic);
    } else {
      Serial.println(F("Failed to subscribe to data topic!"));
      subscribed_ok = false;
    }
    watchdogReset();

    if (subscribed_ok) {
      LedGreen(); // Success!
      publish_status(); // Publish initial online status immediately
    } else {
      LedRed(); // Failure!
    }
    return true;
  } else {
    Serial.print(F("failed, rc="));
    Serial.print(mqttClient.state());
    Serial.println(F(" Retrying in 5 seconds..."));
    LedRed();
    watchdogReset();
    return false;
  }
}

// --- Publish Retained Telemetry Status to MQTT ---
void publish_status() {
  if (!mqttClient.connected()) {
    return;
  }

  IPAddress ip = WiFi.localIP();
  char statusPayload[256];
  snprintf(statusPayload, sizeof(statusPayload),
    "{\"status\":\"online\",\"uptime_s\":%lu,\"user\":%d,\"brightness\":%d,\"free_ram\":%d,\"wifi_rssi\":%d,\"ip\":\"%u.%u.%u.%u\",\"msgs\":%lu,\"last_reset\":\"%s\"}",
    millis() / 1000,
    LUMINAIRE_USER,
    LUMINAIRE_BRIGHTNESS,
    getFreeRam(),
    WiFi.RSSI(),
    ip[0], ip[1], ip[2], ip[3],
    totalMessagesReceived,
    resetReasonStr
  );

  mqttClient.publish(status_topic, statusPayload, true); // true = retained message
}

// --- MQTT Message Callback Function ---
// This function is kept fast and lightweight. Heavy work (network subscribing, NeoPixel show) is deferred to loop().
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  totalMessagesReceived++;
  lastMessageReceivedTime = millis();

  // Check if the topic is for updating the LUMINAIRE_USER variable
  if (strcmp(topic, user_update_topic) == 0) {
    char str[16];
    unsigned int len = (length < sizeof(str) - 1) ? length : sizeof(str) - 1;
    memcpy(str, payload, len);
    str[len] = '\0';
    int new_user_id = atoi(str);

    if (new_user_id != LUMINAIRE_USER) {
      pendingUserId = new_user_id;
      pendingUserChange = true;
    }
    return;
  }

  // Check if the topic is for updating brightness
  if (strcmp(topic, brightness_update_topic) == 0) {
    char str[16];
    unsigned int len = (length < sizeof(str) - 1) ? length : sizeof(str) - 1;
    memcpy(str, payload, len);
    str[len] = '\0';
    pendingBrightness = constrain(atoi(str), 0, 255);
    pendingBrightnessChange = true;
    return;
  }

  // Check if the topic matches our current data topic
  if (strcmp(topic, mqtt_data_topic) == 0) {
    if (length == NEOPIXEL_DATA_LENGTH) {
      // Copy to double-buffer; rendering will happen in loop()
      memcpy(ledDataBuffer, payload, NEOPIXEL_DATA_LENGTH);
      newPixelDataAvailable = true;
    }
  }
}

// --- Diagnostic: Read SAMD21 Reset Reason ---
void printResetReason() {
  uint8_t rcause = PM->RCAUSE.reg;
  Serial.print(F("Microcontroller Reset Reason (0x"));
  Serial.print(rcause, HEX);
  Serial.print(F("): "));

  if (rcause & PM_RCAUSE_POR) {
    strncpy(resetReasonStr, "Power-On Reset", sizeof(resetReasonStr) - 1);
    Serial.println(F("Power-On Reset (Clean cold start)"));
  } else if (rcause & PM_RCAUSE_BOD33) {
    strncpy(resetReasonStr, "Brownout BOD33 (3.3V)", sizeof(resetReasonStr) - 1);
    Serial.println(F(">>> BROWNOUT DETECTOR BOD33 TRIGGERED (3.3V power dropped!) <<< -> Check LED power draw!"));
  } else if (rcause & PM_RCAUSE_BOD12) {
    strncpy(resetReasonStr, "Brownout BOD12 (1.2V)", sizeof(resetReasonStr) - 1);
    Serial.println(F(">>> BROWNOUT DETECTOR BOD12 TRIGGERED (1.2V core power dropped!) <<<"));
  } else if (rcause & PM_RCAUSE_EXT) {
    strncpy(resetReasonStr, "External Reset Pin", sizeof(resetReasonStr) - 1);
    Serial.println(F("External Reset Pin / Reset Button"));
  } else if (rcause & PM_RCAUSE_WDT) {
    strncpy(resetReasonStr, "Watchdog Timeout (>16s)", sizeof(resetReasonStr) - 1);
    Serial.println(F(">>> WATCHDOG TIMER RESET (Firmware hung for >16s) <<< -> SPI or WiFi deadlock!"));
  } else if (rcause & PM_RCAUSE_SYST) {
    strncpy(resetReasonStr, "Software Reset", sizeof(resetReasonStr) - 1);
    Serial.println(F("Software System Reset"));
  } else {
    strncpy(resetReasonStr, "Warm / Unknown Reset", sizeof(resetReasonStr) - 1);
    Serial.println(F("Unknown / Warm reset"));
  }
}

// --- Diagnostic: Calculate Free SRAM on SAMD21 ---
extern "C" char *sbrk(int i);
int getFreeRam() {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}

// --- SAMD21 Native Hardware Watchdog Timer Setup (16-second timeout) ---
void watchdogSetup() {
  // Use Generic Clock Generator 2 for WDT (running from 32.768kHz Ultra-Low Power oscillator OSCULP32K)
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4); // 32.768kHz / 32 = 1.024kHz
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_CLKCTRL_CLKEN;
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Set WDT period to 16384 clock cycles (~16.0 seconds at 1024Hz)
  WDT->CONFIG.reg = WDT_CONFIG_PER_16K;
  WDT->CTRL.reg |= WDT_CTRL_ENABLE;
  while (WDT->STATUS.bit.SYNCBUSY);
}

// --- Feed Watchdog ---
void watchdogReset() {
  if (!WDT->STATUS.bit.SYNCBUSY) {
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
  }
}
