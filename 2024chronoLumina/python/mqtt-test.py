import paho.mqtt.client as mqtt
import json

# MQTT broker details
mqtt_broker = 'mqtt.cetools.org'
mqtt_port = 1884
mqtt_username = 'xxx'
mqtt_password = 'xxx'


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

def publish_message(broker, port, topic, payload):
    client = mqtt.Client()
    client.username_pw_set(mqtt_username, mqtt_password)
    client.connect(mqtt_broker, mqtt_port, 60)

    client.on_connect = on_connect

    client.connect(broker, port, keepalive=60)

    # Convert payload to JSON string
    payload_json = json.dumps(payload)

    # Publish message
    client.publish(topic, payload_json)

    client.disconnect()

if __name__ == "__main__":
    topic = "student/CASA0014/light/1/pixel/"
    payload = {"pixelid": 5, "R": 26, "G": 38, "B": 176, "W": 0}

    publish_message(mqtt_broker, mqtt_port, topic, payload)
