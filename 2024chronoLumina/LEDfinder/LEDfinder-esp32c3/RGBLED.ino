void toggleRGB(){

  LedRed();
  delay(1000);
  LedGreen();
  delay(1000);
  LedBlue();
  delay(1000);
  LedOff();
  delay(1000);
}

void LedRed(){
  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
}

void LedGreen(){
  neopixelWrite(RGB_BUILTIN,0,RGB_BRIGHTNESS,0); // Green 
}

void LedBlue(){
  neopixelWrite(RGB_BUILTIN,0,0,RGB_BRIGHTNESS); // Blue 
}

void LedWhite(){
  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,RGB_BRIGHTNESS,RGB_BRIGHTNESS);
}

void LedOff(){
  neopixelWrite(RGB_BUILTIN,0,0,0);
}