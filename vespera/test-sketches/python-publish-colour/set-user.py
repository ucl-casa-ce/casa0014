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
MQTT_TOPIC = "student/CASA0014/luminaire/user"
CLIENT_ID = "Python_Luminaire_Set_User"








user_id = 0 # Change this to the desired user ID (0-255)







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

    print("\n--- Sending Test Payloads ---")

    print(f"\nSetting user id: {user_id}")
    client.publish(MQTT_TOPIC, user_id, qos=0)
    time.sleep(2)

    print("\nFinished sending test payloads. Disconnecting...")
    client.loop_stop() # Stop the loop
    client.disconnect() # Disconnect from the broker

if __name__ == "__main__":
    main()