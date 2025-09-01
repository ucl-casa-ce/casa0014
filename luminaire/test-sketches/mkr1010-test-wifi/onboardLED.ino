void toggleRGB(){

  WiFiDrv::analogWrite(25, 255);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 0);

  delay(1000);

  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 255);
  WiFiDrv::analogWrite(27, 0);

  delay(1000);

  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 255);

  delay(1000);

  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 0);

  delay(1000);
}

void LedRed(){
  WiFiDrv::analogWrite(25, 155);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 0);  
}

void LedBlue(){
  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 155); 
}

void LedGreen(){
  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 155);
  WiFiDrv::analogWrite(27, 0);  
}