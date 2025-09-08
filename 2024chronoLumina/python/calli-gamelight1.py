import paho.mqtt.client as mqtt
import json
import time
import keyboard
import datetime


 
# MQTT broker details
mqtt_broker = 'mqtt.cetools.org'
mqtt_port = 1884
mqtt_username = 'xxxxxx'
mqtt_password = 'xxxxxx'
topic = "student/CASA0014/light/1/pixel/"

# variables
mode = 3
lst = [[255,0,0],[255, 128, 0],[255, 255, 0],
       [128, 255, 0],[0, 255, 0],[0, 255, 128],
       [0,255,255],[0,128,255],[0,0,255],
       [128,0,255],[255,0,255],[255,0,100]]



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


def powering_rainbow():
        for i in range(0,12):
            payload = {"pixelid": i, "R": lst[i][0], "G": lst[i][1], "B": lst[i][2], "W": 5}
            publish_message(mqtt_broker, mqtt_port, topic, payload)
            time.sleep(0.08)
                
def reset(x):
    if x == 'all':
        for i in range(0,12):
            payload = {"pixelid": i, "R": 0, "G":0 , "B":0 , "W": 0}
            publish_message(mqtt_broker, mqtt_port, topic, payload)
    else:
        payload = {"pixelid": x, "R": 0, "G":0 , "B":0 , "W": 0}
        publish_message(mqtt_broker, mqtt_port, topic, payload)


def chase_game():
    topic = "student/CASA0014/light/216/pixel/"
    player1_pos = 0  # Starting position for player 1 (left arrow)
    player2_pos = 6   # Starting position for player 2 (right arrow)

    while True:
        # Check for player 1 movement (left arrow)
        if keyboard.is_pressed('left'):
            player1_pos = (player1_pos + 1) % 12  # Wrap around if reaching the end

        # Check for player 2 movement (right arrow)
        if keyboard.is_pressed('right'):
            player2_pos = (player2_pos + 1) % 12  # Wrap around if reaching the end


        # Update LEDs based on player positions
        for i in range(12):
            if i == player1_pos:
                payload = {"pixelid": i, "R": 0, "G": 255, "B": 0, "W": 5}  # Green for player 1
                publish_message(mqtt_broker, mqtt_port, topic, payload)
            elif i == player2_pos:
                payload = {"pixelid": i, "R": 255, "G": 0, "B": 0, "W": 5}  # Red for player 2
                publish_message(mqtt_broker, mqtt_port, topic, payload)
            else:
                payload = {"pixelid": i, "R": 0, "G": 0, "B": 0, "W": 0}
                publish_message(mqtt_broker, mqtt_port, topic, payload)

                
        # Check for collision (player2 catches player1)
        if player1_pos == (player2_pos + 1) % 12:
            # Player 1 loses, turn off their LED
            reset(player1_pos)
            print("/nPlayer 2 Wins!")
            break
        elif player2_pos == (player1_pos + 1) % 12:
            # Player 2 loses, turn off their LED
            reset(player2_pos)
            print("/nPlayer 1 Wins!")
            break
  

while True:
    
    reset('all')
    powering_rainbow()
    chase_game()


