#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NUMPIXELS 8

int red, blue, green = 0;

// Declare NeoPixel strip object (ie set it up):
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


void setup() {
  pixels.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.show();            // Turn OFF all pixels ASAP
  pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop() {

  // On each loop we want to set the colour for each pixel on the neopixel strip
  // So we loop through each one in turn, get a random RGB value and set the pixel
  for(int i=0; i<NUMPIXELS; i++) {

    red = random(0, 5) * 50;    // random() takes two numbers in a range and returns 
    green = random(0, 5) * 50;  // a random value between, we then multiply that by 50
    blue = random(0, 5) * 50;   // so that we get seperation between the colours

    pixels.setPixelColor(i, red, green, blue);
  }

  // now the pixel values have been updated in memory we need to send them to the neopixel
  pixels.show();
  delay(300);

}