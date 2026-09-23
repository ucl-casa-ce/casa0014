// Duncan Wilson June 2024 - v1 - MQTT controlled NeoPixel Ring
// Steven Gray - August 2024 - v1.1 - WebBluetooth Additions
// Duncan Wilson - September 2024 - v1.2 - uploaded to 50 ESP32-C3-Zero devices for CASA0014

// works with ESP32-S3-Zero and 12 Neopixel ring
// https://www.waveshare.com/wiki/ESP32-S3-Zero 
// https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html

// This code also works on ESP32 C3 Zero but pin layout is different - see github page for wiring
// https://www.waveshare.com/wiki/ESP32-C3-Zero

//***********************************************************************************************
// make sure to select USB CDC boot to enabled in tools menu
// if adding BLE to device make sure that partition scheme is set to Huge App in tools menu
//***********************************************************************************************

#include <Adafruit_NeoPixel.h>

#define PIN         1   // data pin of neopixel 
#define NUMPIXELS   60  // length of neopixels
#define STARTBRIGHT 200  // starting value of brightness from 0 to 255
#define MAXBRIGHT   220 // brightness limited to 120 via MQTT

// create the pixels object for holding all the info about led values 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);

// Array to store RGBW values for each NeoPixel
// This is used so that I can maintain state (ie i can temporarily run animation and then return)
uint8_t pixelColorsArray[NUMPIXELS][4]; // Each element is an array [R, G, B, W]


void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("This device is Lumina");
  Serial.println(STARTBRIGHT);

  // configure neopixels
  pinMode(LED_BUILTIN, OUTPUT);
  pixels.begin();
  pixels.setBrightness(STARTBRIGHT);

  // quick test to see if all leds are working
  // i use this rather than pulseWhite so that i can see if all rgb leds are also working
  setAllPixels(100); // set all values to 100
  breatheEffect(1, 12); // 2 loops, 12 msec delay
  setAllPixels(0); // set all values to 0 "off"
  
  Serial.println("Set-up complete");

}
 
void loop() {
  Serial.println(STARTBRIGHT);  
  firelighter(10);
  setRGB(0,200,0);
  delay(10000);
  setAllPixels(100); // set all values to 100
  breatheEffect(3, 12); // 2 loops, 12 msec delay
  setAllPixels(0); // set all values to 0 "off"
  delay(2000);
  Serial.println(STARTBRIGHT);  

}

// loops through all stored pixel values and runs pixels.show to update LED's
// brightness can be used to dim the values - e.g. is used in breathe function
void writeAllPixels(uint8_t brightness) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(pixelColorsArray[i][0] * brightness / 255, 
                                          pixelColorsArray[i][1] * brightness / 255, 
                                          pixelColorsArray[i][2] * brightness / 255, 
                                          pixelColorsArray[i][3] * brightness / 255));
  }
}

// given values for one led it updates the array in memory 
void setPixelColor(int pixelIndex, uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  // Ensure the pixelIndex is within bounds
  if (pixelIndex >= 0 && pixelIndex < NUMPIXELS) {
    pixelColorsArray[pixelIndex][0] = red;
    pixelColorsArray[pixelIndex][1] = green;
    pixelColorsArray[pixelIndex][2] = blue;
    pixelColorsArray[pixelIndex][3] = white;
  }
}

// sets all the pixels in the array to the value n
void setAllPixels(int n){
  // Initialize the array of all pixelColorsArray values to value passed in e.g. to 0 at start
  for (int i = 0; i < NUMPIXELS; i++) {
    for (int j = 0; j < 4; j++) {
      pixelColorsArray[i][j] = n;
    }
  }
}

// sets all the pixels in the array to a random value
void setAllPixelsRandom(){
  for (int i = 0; i < NUMPIXELS; i++) {
    for (int j = 0; j < 3; j++) {
      pixelColorsArray[i][j] = random(20, 100);
    }
    pixelColorsArray[i][3] = 0; // turn off the white pixel
  }
}

// sets one LED to a random RGB colour (white is off)
void setOneRGBPixelRandom(){
  int p = random(0,NUMPIXELS);
  for (int j = 0; j < 3; j++) {
    pixelColorsArray[p][j] = random(20, 120);
  }
  pixelColorsArray[p][3] = 0; // turn off the white pixel
}

// pulses all leds white
void pulseWhite(uint8_t wait) {
  Serial.println("pulse white");
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire pixel with white at gamma-corrected brightness level 'j':
    pixels.fill(pixels.Color(0, 0, 0, pixels.gamma8(j)));
    pixels.show();
    delay(wait);
  }

  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    pixels.fill(pixels.Color(0, 0, 0, pixels.gamma8(j)));
    pixels.show();
    delay(wait);
  }
}

void pulseWhiteNoDelay(uint8_t brightnessStep) {
}

void setRGB(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, r, g, b); // Blue color
  }
  pixels.show();
}

// used at start up to show device is alive
void breatheEffect(int loops, int speed) {
  // Define the breathing speed (adjust as needed)
  int breatheSpeed = speed;
  int breatheLimit = 30; // the value from 0-255

  for (int n = 0; n < loops; n++) {
    for (int i = 0; i < breatheLimit; i++) {
      int brightness = sin(i * 3.14159 / 256.0) * 255;
      writeAllPixels(brightness);
      pixels.show();
      delay(breatheSpeed);
    }

    for (int i = breatheLimit; i >= 0; i--) {
      int brightness = sin(i * 3.14159 / 256.0) * 255;
      writeAllPixels(brightness);
      pixels.show();
      delay(breatheSpeed);
    }
  }
}

void chase(uint8_t cycles) {

  uint16_t i, j;

  for(j=0; j<256*cycles; j++) { // n cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(2);
  }

}

void firelighter(uint8_t cycles) {
  for(int j=0; j < cycles; j++){
    for(int i=0; i< pixels.numPixels(); i++) {
      int brightness = random(10, MAXBRIGHT); // Random brightness for each pixel
      int red = random(150, 255); // Random red value
      int green = random(100, 200); // Random green value

      pixels.setPixelColor(i, pixels.Color(red, 0, 0)); // Set pixel color to red and green (yellow)
      pixels.setBrightness(brightness);
      pixels.show();

    }
    delay(50);
    
  }
  
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos; // Reverse color wheel direction for a more natural effect
  if (WheelPos < 85) {
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
}

