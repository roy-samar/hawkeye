// -------------------- Pin Assignments --------------------
const int dirPin        = 2;   // Direction pin
const int stepPin       = 3;   // Step pin
const int epiDisButton  = 5;   // Epi Dispenser Toggle Button
const int apSwitch      = 6;   // AP Switch
const int ms1           = 52;  // Microstep select 1
const int ms2           = 50;  // Microstep select 2
const int ms3           = 48;  // Microstep select 3
const int epiIndLED     = 46;  // Indicator LED
const int snSwitch      = 44;  // SN Switch
const int startLimit    = 40;  // Start limit switch
const int endLimit      = 42;  // End limit switch

// -------------------- Motor Parameters --------------------
int motorDirection      = HIGH;    // Initial direction
const int adultsteps    = 22720;   // ~7.1 revs at 3200 steps/rev
int pediatricsteps= 4500;    // ~1.4 revs at 3200 steps/rev
int motorSpeed          = 40;      // Microseconds delay for step timing

// -------------------- State Variables --------------------
bool motorRunning       = false;
long stepCounter        = 0;       // Counts steps taken
long targetSteps        = 0;       // Steps to run
int startLimitState     = 0;
int endLimitState       = 0;

unsigned long stopStartTime = 0;   // For 10s wait
unsigned long stopStartCounter = 1000;   // For 10s wait
bool waitingAfterRun    = false;
unsigned long lastLogTime = 0;     // For logging


// =========================================================
// Setup
// =========================================================
void setup() {
  pinMode(ms1, OUTPUT);
  pinMode(ms2, OUTPUT);
  pinMode(ms3, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(epiIndLED, OUTPUT);

  pinMode(apSwitch, INPUT_PULLUP);
  pinMode(epiDisButton, INPUT_PULLUP);
  pinMode(snSwitch, INPUT_PULLUP);
  pinMode(startLimit, INPUT_PULLUP);
  pinMode(endLimit, INPUT_PULLUP);

  // Set direction of rotation to clockwise
  digitalWrite(dirPin, motorDirection);

  // Configure microstepping (all HIGH = 1/16 step)
  digitalWrite(ms1, HIGH);
  digitalWrite(ms2, HIGH);
  digitalWrite(ms3, HIGH);

  Serial.begin(9600); // Open serial monitor
}

// =========================================================
// Main Loop
// =========================================================
void loop() {
  if (Serial.available()) {
  String cmd = Serial.readStringUntil('\n'); // read a line
  if (cmd.startsWith("peds")) {
    pediatricsteps = cmd.substring(5).toInt();
    Serial.print("pediatricsteps set to: ");
    Serial.println(pediatricsteps);
  }
  else if (cmd.startsWith("speed")) {
    motorSpeed = cmd.substring(6).toInt();
    Serial.print("motorSpeed set to: ");
    Serial.println(motorSpeed);
  }
}
  // -------------------- LED Indicator --------------------
  bool snMode = (digitalRead(snSwitch) == LOW);
  digitalWrite(epiIndLED, snMode ? HIGH : LOW);

  // -------------------- Input States --------------------
  bool epiPressed = (digitalRead(epiDisButton) == LOW); // Dispenser button pressed?
  bool apMode     = (digitalRead(apSwitch) == LOW);
  startLimitState = digitalRead(startLimit);
  endLimitState   = digitalRead(endLimit);

  // -------------------- Run Initialization --------------------
  if (epiPressed && !motorRunning) {
    motorRunning = true;
    stepCounter  = 0;
    targetSteps  = apMode ? adultsteps : pediatricsteps;
  }

  // -------------------- Direction & Run Logic --------------------
  if (digitalRead(epiDisButton) != motorDirection) {
    motorRunning = true;
  } else {
    motorRunning = false;
  }

  if (endLimitState == LOW && digitalRead(epiDisButton) == LOW) {
    motorDirection = LOW;
    digitalWrite(dirPin, motorDirection);
  } else if (startLimitState == LOW && digitalRead(epiDisButton) == HIGH) {
    motorDirection = HIGH;
    digitalWrite(dirPin, motorDirection);
  }

  // -------------------- Motor Stepping --------------------
  if (motorRunning && !waitingAfterRun) {
    if (stepCounter < targetSteps) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorSpeed);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorSpeed);
      stepCounter++;   // Increment step count
    }
    else {
      motorRunning = false;
      stepCounter  = 0;

      if (motorDirection == HIGH) {
        waitingAfterRun = true;
        stopStartTime   = millis();
      }
      // if motorDirection == LOW, skip waitingAfterRun entirely
    }
  }
  if (waitingAfterRun) {
    if (millis() - stopStartTime >= stopStartCounter) {
      waitingAfterRun = false;  // Done waiting, motor can run again
    }
  }
}