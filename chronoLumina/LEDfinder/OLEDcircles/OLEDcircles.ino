#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // Adjust for your display
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

// Structure to store circle information
struct Circle {
  int x, y, radius;
};

// Maximum number of circles
const int maxCircles = 8;

// Array to store circle information
Circle circles[maxCircles];

void setup() {
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Adjust address if needed

  // Initialize circles
  for (int i = 1; i < maxCircles; i++) {
    circles[i].x = SCREEN_WIDTH / 2;
    circles[i].y = SCREEN_HEIGHT / 2;
    circles[i].radius = i*5;
  }
}

void loop() {
  
  wormhole(80);
  display.clearDisplay();
  display.display();
  delay(1000);

}


void wormhole(int cycles){
  // Starting position at the center
  int x = SCREEN_WIDTH / 2;
  int y = SCREEN_HEIGHT / 2;
  int circlestart = x; 
  int direction = 1;

  for (int i = 0; i < cycles; i++) {
    display.clearDisplay();

    circlestart += direction;

    if(circlestart < x - 20){
      direction = 1; 
    }else if(circlestart > x+20){
      direction = -1; 
    } 
 
    // Move circles and draw them
    for (int i = 1; i < maxCircles; i++) {
      circles[i].radius *= 1.2; // Adjust expansion rate
      display.drawCircle(circles[i].x, circles[i].y, circles[i].radius, WHITE);
      
      if (circles[i].radius > SCREEN_WIDTH / 2) {
        // Reset the circle
        circles[i].radius = 6;
        circles[i].x = circlestart;
      }

    }
    display.display();
    delay(10);
  }

}



