// Sensor pin definitions
int s1 = 9;
int s2 = 10;
int s3 = 11;

// Variables to hold current and last states
int lastReportedState = -1; 
int potentialNewState = -1;

// Debounce variables
unsigned long lastChangeTime = 0;
unsigned long debounceDelay = 150; // Adjust as needed

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting cube orientation tracker...");

  pinMode(s1, INPUT);
  pinMode(s2, INPUT);
  pinMode(s3, INPUT);

  // Read initial state and report it - am storing as decimal from binary inputs
  // e.g. 111 = 4*1 + 2*1 + 1 = 7 ; 010 = 4*0 +2*1 + 0 = 2
  potentialNewState = (digitalRead(s1) * 4) + (digitalRead(s2) * 2) + digitalRead(s3);
  lastReportedState = potentialNewState;
  printSide(potentialNewState);
}

void loop() {
  // Read current sensor values and combine them into a single state variable
  int currentRead = (digitalRead(s1) * 4) + (digitalRead(s2) * 2) + digitalRead(s3);

  // If the current reading is different from the potential new state,
  // we've detected a change. Reset the timer and the potential state.
  if (currentRead != potentialNewState) {
    lastChangeTime = millis();
    potentialNewState = currentRead;
  }

  // Check if the potential new state has been stable long enough AND
  // if it's different from the last state we reported.
  if ((millis() - lastChangeTime) > debounceDelay) {
    if (potentialNewState != lastReportedState) {
      // The new state is stable and hasn't been reported. Report it.
      lastReportedState = potentialNewState;
      printSide(lastReportedState);
    }
  }
}

// Function to print the side based on a combined state value
void printSide(int state) {
  switch (state) {
    case 0: // Binary 000: 4 + 2 + 1
      Serial.println("Side 0 is UP");
      break;
    case 1: // Binary 001: 0 + 0 + 1
      Serial.println("Side 1 is UP");
      break;
    case 2: // Binary 010: 0 + 2 + 0
      Serial.println("Side 2 is UP");
      break;
    case 3: // Binary 011: 0 + 2 + 1
      Serial.println("Side 3 is UP");
      break;
    case 4: // Binary 100: 0 + 2 + 1
      Serial.println("Side 4 is UP");
      break;
    case 5: // Binary 101: 4 + 0 + 1
      Serial.println("Side 5 is UP");
      break;
    case 6: // Binary 110: 4 + 2 + 0
      Serial.println("Side 6 is UP");
      break;
    case 7: // Binary 111: 4 + 2 + 1
      Serial.println("Side 7 is UP");
      break;
    default:
      Serial.println("No recognised state.");
      break;
  }
}