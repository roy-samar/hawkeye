#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD at I2C address 0x27, 20 columns, 4 rows
LiquidCrystal_I2C lcd(0x27, 20, 4);

// -------------------- Pin Assignments --------------------
const int dirPin        = 2;   // Direction pin
const int stepPin       = 3;   // Step pin
const int epiDisButton  = 5;   // Epi Dispenser Toggle Button
const int apSwitch      = 6;   // AP Switch
const int ms1           = 52;  // Microstep select 1
const int ms2           = 50;  // Microstep select 2
const int ms3           = 48;  // Microstep select 3
const int epiIndLED     = 46;  // Indicator LED
const int epiDisLED     = 9;   // epi dispending blue LED
const int epiEmptyLED   = 10;  // epi ran out yellow LED FD 
const int snSwitch      = 44;  // SN Switch
const int startLimit    = 40;  // Start limit switch
const int endLimit      = 42;  // End limit switch

// -------------------- Motor Parameters --------------------
int motorDirection      = HIGH;    // Initial direction
const int adultsteps    = 22720;   // ~7.1 revs at 3200 steps/rev
int pediatricsteps = 0;    // ~1.4 revs at 3200 steps/rev
int pediatricsWeight = 0;    // ~1.4 revs at 3200 steps/rev
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

// -------------------- LCD Logging --------------------
// -------------------- LCD Logging --------------------
const int LCD_HISTORY = 100;   // store up to 100 lines
String lcdBuffer[LCD_HISTORY]; // global buffer
int lcdIndex = 0;              // tracks where we are

void logToLCD(String newLine) {
  // Add new line to buffer
  lcdBuffer[lcdIndex] = newLine;
  lcdIndex = (lcdIndex + 1) % LCD_HISTORY; // wrap around if needed

  // Redraw only the last 4 lines
  lcd.clear();
  for (int row = 0; row < 4; row++) {
    // Calculate which buffer entry to show
    int idx = (lcdIndex - 4 + row + LCD_HISTORY) % LCD_HISTORY;
    lcd.setCursor(0, row);
    lcd.print(lcdBuffer[idx]);
  }
}

// Convert patient weight (kg) into step count
long calculateStepsFromWeight(float weightKg) {
  // Dose in mg
  float doseMg = 0.01 * weightKg;

  // Volume in mL (since concentration is 0.1 mg/mL)
  float volumeMl = doseMg / 0.1;

  // Steps per mL (22720 steps = 10 mL)
  float stepsPerMl = 22720.0 / 10.0;

  // Final step count
  long steps = (long)(volumeMl * stepsPerMl);

  return steps;
}

// Convert steps into mL with one decimal place
String stepsToMl(long steps) {
  float ml = (float)steps / 2272.0;   // 2272 steps = 1 mL
  return String(ml, 1);               // format with 1 decimal place
}

// Convert millis() into HH:MM string (24-hour format)
String getTimeString() {
  unsigned long totalSeconds = millis() / 1000;   // ms → seconds
  unsigned long minutes      = totalSeconds / 60;
  unsigned long hours        = (minutes / 60) % 24;
  minutes = minutes % 60;

  // Format as HH:MM with leading zeros
  char buffer[6];
  sprintf(buffer, "%02lu:%02lu", hours, minutes);
  return String(buffer);
}

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
  pinMode(epiDisLED, OUTPUT);
  pinMode(epiEmptyLED, OUTPUT);

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
  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.begin(9600); // Open serial monitor
}

// =========================================================
// Main Loop
// =========================================================
void loop() {
  if (Serial.available()) {
  String cmd = Serial.readStringUntil('\n'); // read a line
  if (cmd.startsWith("weight")) {
    pediatricsWeight = cmd.substring(7).toInt(); // patient weight in kg
    pediatricsteps = calculateStepsFromWeight(pediatricsWeight);
    Serial.print("Weight (kg): ");
    Serial.println(pediatricsWeight);
    Serial.print("Calculated pediatricsteps: ");
    Serial.println(pediatricsteps);
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

  // -------------------- Epi Dispenser LED --------------------
if (epiPressed && endLimitState != LOW) {
  digitalWrite(epiDisLED, HIGH);
} else {
  digitalWrite(epiDisLED, LOW);
}

  // -------------------- Epi Empty LED --------------------
digitalWrite(epiEmptyLED, (endLimitState == LOW) ? HIGH : LOW);

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
      // Logging to Screen
      if (stepCounter != 0 && motorDirection == HIGH)
      {
        logToLCD(
          getTimeString() + " " +
          ((apMode == 1) ? "A" : "P") +
          " EPI " +
          stepsToMl(stepCounter) +
          "mL"
        );
      }
      
      // end Logging
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