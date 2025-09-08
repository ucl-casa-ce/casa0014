#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // Note when using the adafruit examples i needed to change the address to 0x3C

Adafruit_SSD1306 display(128, 64); // dimensions of OLED display in pixels

int counter = 0;

void setup() {
  Wire.begin(2,3);  // SDA 2, SCL 3
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // these amazon screens are different address to adafruit ones
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  display.clearDisplay();
  displaytitle();
  displaycounter(counter);

  delay(100); // Adjust delay for desired update rate
  counter++;

  // Reset counter if it reaches 100
  if (counter >= 51) {
    counter = 0;
  }
}

void displaytitle(){
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Select LED");
}

void displaycounter(int counter){
  // Convert counter to a string for display
  String counterStr = String(counter);

  // Pad with leading zeros if necessary
  if (counter < 10) {
    counterStr = "0" + counterStr;
  }
  
  display.setCursor(30, 20); // Reset cursor position
  display.setTextSize(6);
  display.print(counterStr);

  display.drawRect((counter*2)+14, 12, 5, 2, WHITE);

  display.display(); // Update the display

}