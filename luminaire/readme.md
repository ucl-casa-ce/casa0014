# Luminaire Readme

next up
- project name
- need to add in way to define who can currently control the luminaire (use dial like on chrono lumina)
- add in documentation for how to address the lights
- build version using WS2812b leds being used in tutorial
- build MKR1010 based controller for install with lights (and sort out power)
- build demo controller using MKR1010 for tutorial (in cardboard box - tilt switch to change colour)
- build python demo script to control the lights
- build web visualisation that can subscribe to MQTT feed and display the current state of the lights (to allow for everyone to test in parallel)


## API for the Luminaire

In the `mqtt_callback` function, the payload array contains the binary RGB values for each LED. Each LED's color is defined by three consecutive bytes: Red, Green, and Blue. The values for each color component range from 0 to 255.

Here are some examples of what those r, g, and b byte values would represent:

Red (full brightness):

`R = 255`
`G = 0`
`B = 0`

In the payload, this would be `FF 00 00` (hexadecimal)

Green (full brightness):

`R = 0`
`G = 255`
`B = 0`

In the payload, this would be `00 FF 00` (hexadecimal)

Blue (full brightness):

`R = 0`
`G = 0`
`B = 255`

In the payload, this would be `00 00 FF` (hexadecimal)

White (full brightness):

`R = 255`

`G = 255`

`B = 255`

In the payload, this would be `FF FF FF` (hexadecimal)

Black (off):

`R = 0`

`G = 0`

`B = 0`

In the payload, this would be `00 00 00` (hexadecimal)

Purple (mix of red and blue):

`R = 128` (medium red)

`G = 0`

`B = 128` (medium blue)

In the payload, this would be `80 00 80` (hexadecimal)