import paho.mqtt.client as mqtt
import time
import random
import os # Import the os module to access environment variables


# from terminal cd to directory with this file and then:
# export MQTT_USERNAME="your_mqtt_username"
# export MQTT_PASSWORD="your_mqtt_password"
# python send-colour.py

# --- MQTT Configuration ---
MQTT_BROKER = "mqtt.cetools.org" 
MQTT_PORT = 1884
MQTT_TOPIC = "student/CASA0014/luminaire/1"
CLIENT_ID = "Python_NeoPixel_Publisher"

# Load MQTT username and password from environment variables
# IMPORTANT: Replace these placeholder values with actual environment variables
# Set these variables in your terminal before running the script:
# export MQTT_USERNAME="your_mqtt_username"
# export MQTT_PASSWORD="your_mqtt_password"
MQTT_USERNAME = os.getenv("MQTT_USERNAME")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")

# Basic check to ensure credentials are set
if not MQTT_USERNAME or not MQTT_PASSWORD:
    print("Error: MQTT_USERNAME and/or MQTT_PASSWORD environment variables are not set.")
    print("Please set them before running the script (e.g., export MQTT_USERNAME='your_user').")
    exit(1) # Exit the script if credentials are missing

# --- NeoPixel Configuration ---
NEOPIXEL_COUNT = 72
BYTES_PER_LED = 3 # Red, Green, Blue
PAYLOAD_LENGTH = NEOPIXEL_COUNT * BYTES_PER_LED # 48 * 3 = 144 bytes

# --- MQTT Callbacks ---
def on_connect(client, userdata, flags, rc):
    """Callback function for when the client connects to the MQTT broker."""
    if rc == 0:
        print(f"Connected to MQTT Broker: {MQTT_BROKER} on port {MQTT_PORT}")
    else:
        print(f"Failed to connect, return code {rc}\n")

def on_publish(client, userdata, mid):
    """Callback function for when a message is published."""
    print(f"Message Published (MsgID: {mid})")

# --- Helper Functions to Create Payloads ---

def create_solid_color_payload(r, g, b):
    """
    Creates a binary payload to set all NeoPixels to a single solid color.
    Args:
        r (int): Red component (0-255).
        g (int): Green component (0-255).
        b (int): Blue component (0-255).
    Returns:
        bytearray: The 144-byte binary payload.
    """
    if not all(0 <= val <= 255 for val in [r, g, b]):
        raise ValueError("RGB values must be between 0 and 255.")

    # Create a bytearray of the correct length
    payload = bytearray(PAYLOAD_LENGTH)
    for i in range(NEOPIXEL_COUNT):
        payload[i * BYTES_PER_LED] = r
        payload[i * BYTES_PER_LED + 1] = g
        payload[i * BYTES_PER_LED + 2] = b
    return payload

def create_gradient_payload(start_rgb, end_rgb):
    """
    Creates a binary payload for a smooth gradient across the strip.
    Args:
        start_rgb (tuple): (R, G, B) tuple for the first LED.
        end_rgb (tuple): (R, G, B) tuple for the last LED.
    Returns:
        bytearray: The 144-byte binary payload.
    """
    payload = bytearray(PAYLOAD_LENGTH)
    start_r, start_g, start_b = start_rgb
    end_r, end_g, end_b = end_rgb

    for i in range(NEOPIXEL_COUNT):
        # Calculate interpolated color for each LED
        ratio = i / (NEOPIXEL_COUNT - 1)
        r = int(start_r + (end_r - start_r) * ratio)
        g = int(start_g + (end_g - start_g) * ratio)
        b = int(start_b + (end_b - start_b) * ratio)

        print(f"LED {i}: R={r} G={g} B={b}") # Debug print for each LED's color

        payload[i * BYTES_PER_LED] = r
        payload[i * BYTES_PER_LED + 1] = g
        payload[i * BYTES_PER_LED + 2] = b
    return payload

def create_random_color_payload():
    """
    Creates a binary payload with random colors for each NeoPixel.
    Returns:
        bytearray: The 144-byte binary payload.
    """
    payload = bytearray(PAYLOAD_LENGTH)
    for i in range(NEOPIXEL_COUNT):
        r = random.randint(0, 255)
        g = random.randint(0, 255)
        b = random.randint(0, 255)
        payload[i * BYTES_PER_LED] = r
        payload[i * BYTES_PER_LED + 1] = g
        payload[i * BYTES_PER_LED + 2] = b
    return payload

def create_random_red_payload():
    """
    Creates a binary payload with random colors for each NeoPixel.
    Returns:
        bytearray: The 144-byte binary payload.
    """
    payload = bytearray(PAYLOAD_LENGTH)
    for i in range(NEOPIXEL_COUNT):
        r = random.randint(50, 255)
        payload[i * BYTES_PER_LED] = r
        payload[i * BYTES_PER_LED + 1] = 0
        payload[i * BYTES_PER_LED + 2] = 0
    return payload

# --- Main Publishing Logic ---
def main():
    # Create an MQTT client instance
    client = mqtt.Client(CLIENT_ID)

    # Set username and password for authentication
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    # Assign callback functions
    client.on_connect = on_connect
    client.on_publish = on_publish

    # Connect to the MQTT broker
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60) # 60-second keepalive
    except Exception as e:
        print(f"Error connecting to broker: {e}")
        return

    # Start the MQTT client loop in a non-blocking way
    client.loop_start()

    # print("\nSetting all pixels to off (000000)...")
    # off_payload = create_solid_color_payload(0, 0, 0)
    # client.publish(MQTT_TOPIC, off_payload, qos=0)

    print("\n--- Sending Test Payloads ---")

    # Example 1: Set all pixels to solid Red
    # print("\nSetting all pixels to RED (FF0000)...")
    # red_payload = create_solid_color_payload(255, 0, 0)
    # client.publish(MQTT_TOPIC, red_payload, qos=0)
    # time.sleep(2) # Wait a bit to see the effect
    # print(f"Last payload: ({red_payload})")

    # Example 8: Green to Red Gradient fading in and out
    for n in range(10):
        
        print("\nSetting a Green to Blue gradient...")
        for i in range(50):
            gradient_payload = create_gradient_payload((0, i*5, 0), (i*5, 0, 0))
            client.publish(MQTT_TOPIC, gradient_payload, qos=0)
            time.sleep(0.1)
        for i in range(50):
            colour = 255-(i*5)
            gradient_payload = create_gradient_payload((0, colour, 0), (colour, 0, 0))
            client.publish(MQTT_TOPIC, gradient_payload, qos=0)
            time.sleep(0.1)

    print("\nFinished sending test payloads. Disconnecting...")
    client.loop_stop() # Stop the loop
    client.disconnect() # Disconnect from the broker

if __name__ == "__main__":
    main()