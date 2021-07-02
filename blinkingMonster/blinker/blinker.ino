/*
  Blinker project by Duncan Wilson
  March 2020
  
  Ping code based on example by David A. Mellis http://www.arduino.cc/en/Tutorial/Ping 
  but adapted to use the HC-SR04 ultrasonic sensor (which uses 4 wires rather than 3)
  
*/

#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.

// this constant won't change. It's the pin number of the sensor's output:
const int trigPin = 9;
const int echoPin = 10;
const int LED = 13;
const boolean debug = true    ; 
       
// establish variables for duration of the ping, and the distance result
// in centimeters and one for LED blink rate:
long duration, cm;
int blinkrate = 0;


void setup() {
  // initialize serial communication:
  if(debug){
    Serial.begin(9600);
    Serial.println("Starting blinker");
  }
  pinMode(LED, OUTPUT); 
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input     
}

void loop() {

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);

  if(cm < 100){

    if(cm < 30){
      blinkrate = map(cm, 1, 30, 10, 300);
    }
    else if(cm < 90){
      blinkrate = map(cm, 30, 90, 300, 1000);
    }
    else{
      blinkrate = 0;
    }
  
    if(debug){
      Serial.print(" cm: ");
      Serial.print(cm); 
      Serial.print("   blinkrate: ");  
      Serial.println(blinkrate); 
    }
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(blinkrate);                       // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    delay(blinkrate); 

  }

  delay(20);
    
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}
