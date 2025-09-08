/* Based on Oleg Mazurov's code for rotary encoder interrupt service routines for AVR micros
   here https://chome.nerpa.tech/mcu/reading-rotary-encoder-on-arduino/
   and using interrupts https://chome.nerpa.tech/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros/

   This example does not use the port read method. Tested with Nano and ESP32
   both encoder A and B pins must be connected to interrupt enabled pins, see here for more info:
   https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

   Great Explanation of Rotary Encoders here: https://www.youtube.com/watch?v=fgOfSHTYeio 


   on RE - middle pin is GND left and right are clockwise and anticlockwise rotation depending 
   on which is connected to ENC_A or B pins
*/

// Define rotary encoder pins - check which pins have interupts - these are for MKR boards
#define ENC_A 4
#define ENC_B 5

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int counter = 0;
int upperlimit = 50;
int lowerlimit = 0;

void setup() {

  // Set encoder pins and attach interrupts
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);

  pinMode(8, INPUT_PULLUP);  // Pin 8 reads 1 if not connected to GND
  pinMode(LED_BUILTIN, OUTPUT); // Use the built in LED to check status

  // Start the serial monitor to show output
  Serial.begin(115200);
}

void loop() {
  static int lastCounter = 0;

  // If count has changed print the new value to serial
  if(counter != lastCounter){
    if (counter > upperlimit) {
      counter = upperlimit;
    } else if (counter < lowerlimit) {
      counter = lowerlimit;
    }    
    Serial.println(counter);
    lastCounter = counter;
  }

 digitalWrite(LED_BUILTIN, digitalRead(8)); // echo status of pin 8 input on the built in LED

}

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
} 