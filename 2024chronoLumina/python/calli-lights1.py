import paho.mqtt.client as mqtt
import json
import time
##import keyboard
import random
import datetime



# MQTT broker details
mqtt_broker = 'mqtt.cetools.org'
mqtt_port = 1884
mqtt_username = 'xxx'
mqtt_password = 'xxx'
lightID = "1"
topic = "student/CASA0014/light/" + lightID + "/pixel/"

# variables

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

def reset(x):
    if x == 'all':
        for i in range(0,12):
            payload = {"pixelid": i, "R": 0, "G":0 , "B":0 , "W": 0}
            publish_message(mqtt_broker, mqtt_port, topic, payload)
    else:
        payload = {"pixelid": x, "R": 0, "G":0 , "B":0 , "W": 0}
        publish_message(mqtt_broker, mqtt_port, topic, payload)
        
def create_payload(color_sequence):
    payload = {"allLEDs": []}
    for i in range(len(color_sequence)):
        # Pre-calculate color indices for each LED
        color_index = (i + 1) % len(color_sequence)
        payload["allLEDs"].append({
            "pixelid": i,
            "R": color_sequence[color_index][0],
            "G": color_sequence[color_index][1],
            "B": color_sequence[color_index][2],
            "W": 0
        })
    return payload



def powering_rainbow():
    if 1:
        for i in range(0,12):
                payload = {"pixelid": i, "R": lst[i][0], "G": lst[i][1], "B": lst[i][2], "W": 5}
                publish_message(mqtt_broker, mqtt_port, topic, payload)
                time.sleep(0.5)


def walking_rainbow():
    while 1:
        for i in range(0,12):
            if i != 0:
                y = i-1
                reset(y)
            else:
                reset(11)
            topic = "student/CASA0014/light/" + lightID + "/pixel/"
            payload = {"pixelid": i, "R": lst[i][0], "G": lst[i][1], "B": lst[i][2], "W": 0}
            publish_message(mqtt_broker, mqtt_port, topic, payload)
            time.sleep(0.5)


def spinning_rainbow():
    topic = "student/CASA0014/light/" + lightID + "/all/"
    payload = create_payload(lst)
    publish_message(mqtt_broker, mqtt_port, topic, payload)
    time.sleep(0.5)

    while True:   
        for i in range(len(lst)):
             # Create a new color sequence with the first color shifted
            shifted_colors = lst[i:] + lst[:i]
            payload = create_payload(shifted_colors)
            publish_message(mqtt_broker, mqtt_port, topic, payload)
            time.sleep(0.005)  # Adjust rotation speed




def pride(): 
    while True:
        topic = "student/CASA0014/light/" + lightID + "/all/"
        pride_flags = {
        "rainbow": lst,
        "l": [[255, 128, 0], [255, 255, 255], [255,0,255]],  # orange, white, pink
        "g": [[38, 206, 140], [255,255,255], [80, 73, 170]],  # green, white, ?indigo
         "b": [[214, 2, 112], [170, 40, 255], [0, 56, 168]],  # Pink, Purple, Blue
        "t": [[0, 150, 200], [255, 255, 255], [200, 30, 150]],  # Light Blue, White, Pink
        
        
      }
        for flag_name, colors in pride_flags.items():
            payload = {"allLEDs": []}
            for led in range(len(lst)):
                flag_index = led % len(colors)
                payload["allLEDs"].append({
                "pixelid": led,
                "R": colors[flag_index][0],
                "G": colors[flag_index][1],
                "B": colors[flag_index][2],
                "W": 0
            })
            publish_message(mqtt_broker, mqtt_port, topic, payload)
            time.sleep(3)

def arc_reactor():
    stop = 0 
    topic = "student/CASA0014/light/" + lightID + "/all/"
    arc_pattern = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    blue_color = {"R": 3, "G": 0, "B": 255, "W": 5}
    while True:
        for i in range(len(arc_pattern)):
            payload = {"allLEDs": []}
            for led in range(len(arc_pattern)):
                if led == arc_pattern[i]:
                    payload["allLEDs"].append({
                        "pixelid": led,
                        **blue_color
                    })
                else:
                    payload["allLEDs"].append({
                        "pixelid": led,
                        "R": 0,
                        "G": 0,
                        "B": 0,
                        "W": 0
                    })
            publish_message(mqtt_broker, mqtt_port, topic, payload)
            time.sleep(0.002)
            stop += 1
        if stop > 36:
            break

def double():
    topic = "student/CASA0014/light/" + lightID + "/all/"
    arc_pattern = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    blue_color = {"R": 3, "G": 0, "B": 255, "W": 5}
    color2 = {"R": 3, "G": 0, "B": 255, "W": 5}
    red_pattern = [11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0]
    thr_pattern = [6, 5, 4, 3, 2, 1, 0, 11, 10, 9, 8, 7]
    thr_pattern = [6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5]
    while True:
        for i in range(len(arc_pattern)):
            payload = {"allLEDs": []}
            for led in range(len(arc_pattern)):      
                if led == arc_pattern[i]:
                    payload["allLEDs"].append({
                        "pixelid": led,
                        **blue_color
              })
                elif led == red_pattern[i]:
                    payload["allLEDs"].append({
                        "pixelid": led,
                        **color2                    
                    })
                elif led == thr_pattern[i]:
                    payload["allLEDs"].append({
                        "pixelid": led,
                        **color2                    
                    })
                elif led == thr_pattern[i]:
                    payload["allLEDs"].append({
                        "pixelid": led,
                        **color2                    
                    })
                else:
                    payload["allLEDs"].append({
                        "pixelid": led,
                        "R": 0,
                        "G": 0,
                        "B": 0,
                        "W": 0
                    })
            publish_message(mqtt_broker, mqtt_port, topic, payload)
            time.sleep(0.01)
            
        
  



def clock():
    while True:
        now = datetime.datetime.now()
        timer = now.strftime('%H:%M:%S')
        hrs , mins , secs = timer.split(":")
        hrs = int(hrs) - 1
        mins = round((int(mins)/5-1))
        secs = round((int(secs)/5-1))
        
        payload = {"pixelid": int(hrs), "R": 0, "G": 255, "B": 0, "W": 0}
        publish_message(mqtt_broker, mqtt_port, topic, payload)

        payload = {"pixelid": int(mins), "R": 0, "G": 0, "B": 255, "W": 0}
        publish_message(mqtt_broker, mqtt_port, topic, payload)

        payload = {"pixelid": int(secs), "R": 255, "G": 0, "B": 0, "W": 0}
        publish_message(mqtt_broker, mqtt_port, topic, payload)

        time.sleep(5)
        payload = {"pixelid": int(secs), "R": 0, "G": 0, "B": 0, "W": 0}
        publish_message(mqtt_broker, mqtt_port, topic, payload)



    
if __name__ == "__main__":
    reset('all')     

    arc_reactor()
