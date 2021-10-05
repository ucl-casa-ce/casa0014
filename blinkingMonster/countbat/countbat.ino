
/*
  CountBat project by Duncan Wilson
  Oct 2021
    
*/

// Adafruit NeoPixel library
#include <Adafruit_NeoPixel.h>

// Data Pin for NeoPixels?
#define PIN        6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 12 // we are using a 12 LED RGBW NeoPixel

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. 
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGBW + NEO_KHZ800);

#define DELAYVAL 300 // Time (in milliseconds) to pause between updates
int MQTTupdates[NUMPIXELS];
int r,g;

void setup() {
  pixels.begin(); // INITIALIZE NeoPixel strip object
  pixels.clear(); // Set all pixel colors to 'off'

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));  
  for(int i=0; i<NUMPIXELS; i++) {
    MQTTupdates[i] = random(2); // either 0 or 1
  }
}

void loop() {

  // check for MQTTupdates
  // --- add code in here ---
  
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    r=150; //default to red being on
    g=0;
    // work out if LED is red or green
    if(MQTTupdates[i]){
      r=0;
      g=200;      
    }
    // pixels.Color() takes GRBW values, not sure why GR are switched
    pixels.setPixelColor(i, pixels.Color(g, r, 0, 12));

  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  
  for (int i=50; i<200; i++) { pixels.setBrightness(i); pixels.show(); delay(10); }
  for (int i=200; i>50; i--) { pixels.setBrightness(i); pixels.show(); delay(10); }

}
