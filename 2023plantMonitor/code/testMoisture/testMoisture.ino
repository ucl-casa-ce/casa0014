//one nail goes to +3.2V (ie pin 13), the other nail goes to this analogue pin
// since AO on the feather huzzah only reads from 0-1V we also use a voltage divider 
// to drop the voltage from the range 0-3.2V down to 0-1V
uint8_t soilPin = 0;   
int moisture_val;

int sensorVCC = 13;
int counter = 0;

void setup() {
  Serial.begin(115200);     //open serial port
  pinMode(sensorVCC, OUTPUT); 
  digitalWrite(sensorVCC, LOW);
}

void loop() {
  counter++;
  if(counter> 6){      // change this value to set "not powered" time. higher number bigger gap
    // power the sensor
    digitalWrite(sensorVCC, HIGH);
    delay(1000);
    // read the value from the sensor:
    moisture_val = analogRead(soilPin);   // read the resistance      
    //stop power 
    digitalWrite(sensorVCC, LOW);  
    delay(100);
    
    Serial.print("sensor = " );                       
    Serial.println(moisture_val);     
    counter=0;    
  }  
  //wait
  Serial.print(".");                       
  delay(1000);
}
