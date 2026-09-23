# Getting Started with Vespera Workshop Sketches

The sketches in this folder are designed to run on your own **Arduino MKR1010** to send real-time light animations to the central **Vespera luminaire** over MQTT.

We recommend exploring and uploading the sketches in the following step-by-step order:

---

## Recommended Learning Pathway

```
Step 1: Simple RGB ──► Step 2: Random Colours ──► Step 3: Breathing White ──► Step 4: Column Rainbow Sweep
```

---

### Step 1: The Basics — [`mkr1010_mqtt_simple`](mkr1010_mqtt_simple/mkr1010_mqtt_simple.ino)
* **Goal**: Understand the core communication pipeline.
* **What it does**: Connects to WiFi, connects to the MQTT broker, and cycles through Red, Green, and Blue washes.
* **Key Learning**:
  - Setting up `arduino_secrets.h` with your WiFi and MQTT credentials (but make sure to .gitignore this file!).
  - Setting your allocated `lightId` (e.g. `String lightId = "1";`).
  - How the 72 LEDs are represented as a 216-byte RGB payload array (`72 * 3 = 216 bytes`).

---

### Step 2: Pixel Patterns — [`mkr1010_mqtt_simple_random`](mkr1010_mqtt_simple_random/mkr1010_mqtt_simple_random.ino)
* **Goal**: Learn how to manipulate individual LEDs independently.
* **What it does**: Generates sparkling, dynamic random color values across individual pixels.
* **Key Learning**:
  - Indexing individual LEDs in the payload array using `pixel * 3`.
  - Creating animated noise and randomized effects.

---

### Step 3: Fading & Ambiance — [`mkr1010_mqtt_simple_white`](mkr1010_mqtt_simple_white/mkr1010_mqtt_simple_white.ino)
* **Goal**: Create smooth, calming lighting transitions.
* **What it does**: Gradually fades all LEDs in and out to turn Vespera into a breathing ambient nightlight.
* **Key Learning**:
  - Using loops to smoothly ramp color intensities up and down.
  - Keeping the MQTT connection alive during animation loops (`mqttClient.loop()`).

---

### Step 4: Spatial Geometry & Rotation — [`mkr1010_mqtt_simple_sweep`](mkr1010_mqtt_simple_sweep/mkr1010_mqtt_simple_sweep.ino)
* **Goal**: Master the physical layout of Vespera to create rotating 3D animations.
* **What it does**: Rotates a continuous 360-degree rainbow wheel around the perimeter of the hemisphere.
* **Key Learning**:
  - Understanding Vespera's physical arrangement: **12 perimeter columns** with **6 vertical LEDs each**:
    - **Column 0**: Pixels `0 to 5`
    - **Column 1**: Pixels `6 to 11`
    - ...
    - **Column 11**: Pixels `66 to 71`
  - Mapping colors spatially and calculating phase offsets using the color wheel.

---

## Essential Setup Checklist Before Uploading

1. **Set Your `lightId`**:
   - In each sketch, update the `lightId` variable to your allocated student user number (10–40):
     ```cpp
     String lightId = "11"; // Change to your allocated number
     ```
2. **Configure `arduino_secrets.h`**:
   - Enter your WiFi network details and the workshop MQTT credentials in the `arduino_secrets.h` tab.
3. **Test Without Vespera**:
   - You can preview your animations in real-time by opening the [Web Visualiser](../web-viewer) in your browser and selecting your lightId!