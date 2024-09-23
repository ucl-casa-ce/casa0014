# Chrono Lumina v2

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/53984082109/in/album-72177720320549464" title="LEDs"><img src="https://live.staticflickr.com/65535/53984082109_2b0bfe6bcf_c.jpg" width="100%" alt="LEDs"/></a>

LED Neopixel Ring controlled via MQTT. Created for CASA0014 project work / teaching but also used as a display to show status of devices in CE Lab. 



## v2 - ESP32 C3 - How to control the LED ring via MQTT

Arduino code in this [folder](d1miniesp32/). Sketch uses same message structure as v1 so can be tested with same HTML page. 

Info on the [Waveshare ESP32 C3 Zero](https://www.waveshare.com/esp32-c3-zero.htm).

### Controlling via MQTT
NeoPixels are controlled via MQTT. Each Arduino is numbered 1 to 50 and listens for messages on a topic with the following structure:

mqtt.cetools.org/student/CASA0014/light/x/

where x is the number of the device.

### Set a single pixel
Individual LED's can be controlled by sending a message to the "pixel" topic with the following format. In this case we are controlling Arduino number 2 and setting pixel 4 (of 12) to a colour value of red. Note you can also set the white led (W). All RGBW values are 0-254.

topic = student/CASA0014/light/2/pixel/
```  
{
    "pixelid": 3,
    "R": 0,
    "G": 255,
    "B": 128,
    "W": 200
}
```

### Set all pixels
To set all LED's in one go send a message to topic "all" with the following payload. Note this function fades the pixels in rather than setting them instantly.

topic = student/CASA0014/light/2/all/


```
{
  "allLEDs": [
    {
      "pixelid": 0,
      "R": 58,
      "G": 69,
      "B": 168,
      "W": 0
    },
    {
      "pixelid": 1,
      "R": 188,
      "G": 62,
      "B": 129,
      "W": 0
    },
    {
      "pixelid": 2,
      "R": 89,
      "G": 238,
      "B": 253,
      "W": 0
    },
...
...
    {
      "pixelid": 10,
      "R": 240,
      "G": 213,
      "B": 83,
      "W": 0
    },
    {
      "pixelid": 11,
      "R": 250,
      "G": 17,
      "B": 118,
      "W": 0
    }
  ]
}
```

### Change brightness

The overall brightness of the LED's can be set on the topic:

student/CASA0014/light/2/brightness/

expecting to receive a JSON packet in this format
```
{
    "brightness": 50
}
```
Note: brightness is limited to 120 via MQTT which should be bright enough!

### Other functions
A few other internal functions are used for testing the neopixels - for example:

On topic = student/CASA0014/light/2/all/

You can also send the following payloads

```
{
    "method": "clear"
}
```
This sets all LED values to 0

```
{
    "method": "allrandom"
}
```
Randomly assigns values to all pixels

```
{
    "method": "onerandom"
}
```
Randomly assigns an RGD value to a random pixel

```
{
    "method": "pulsewhite"
}
```
Stores the current pixel values, pulses all LED's white and then returns to original LED values


