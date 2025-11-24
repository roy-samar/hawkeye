#include <Arduino.h>

// pin connections
const int dirPin = 2; 
const int stepPin = 3; 
const int ms1 = 8;
const int ms2 = 9;
const int ms3 = 10;
const int limit1pin = 11;
const int limit2pin = 12;
const int limitStopPin = 13;
const int ledPin = 22;
const int toggleButtonPin = 23;

bool moving = false;
int motorSpeed = 40;
int delayMotion = 1000;

bool ignoreLimit1 = false;
bool ignoreLimit2 = false;
bool direction = true;  

bool lastButtonState = HIGH; 

// step counter
long stepCount = 0;
const long targetSteps = 24000; // 10 mL in 2 seconds

void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(ms1, OUTPUT);
  pinMode(ms2, OUTPUT);
  pinMode(ms3, OUTPUT);
  pinMode(limit1pin, INPUT_PULLUP);
  pinMode(limit2pin, INPUT_PULLUP);
  pinMode(limitStopPin, INPUT_PULLUP);

  digitalWrite(dirPin, direction);
  digitalWrite(ms1, HIGH);
  digitalWrite(ms2, HIGH);
  digitalWrite(ms3, HIGH);

  pinMode(ledPin, OUTPUT);
  pinMode(toggleButtonPin, INPUT_PULLUP);
}

void loop() {
  // --- Button logic ---
  bool buttonState = digitalRead(toggleButtonPin);
  if (lastButtonState == HIGH && buttonState == LOW) {
    delay(50);  // debounce
    moving = !moving;
    stepCount = 0; // reset counter on new start
  }
  lastButtonState = buttonState;

  // LED follows motor state
  digitalWrite(ledPin, moving ? HIGH : LOW);

  if (digitalRead(limitStopPin) == LOW) {
    moving = false;
  }

  if (moving) {
    // limit switch logic
    if (!ignoreLimit1 && digitalRead(limit1pin) == LOW) {
      delay(delayMotion);
      direction = !direction;              // reverse direction
      digitalWrite(dirPin, direction);
      moving = false;                      // NEW: stop motor until button pressed
      ignoreLimit1 = true;
    }
    if (!ignoreLimit2 && digitalRead(limit2pin) == LOW) {
      delay(delayMotion);
      direction = !direction;              // reverse direction
      digitalWrite(dirPin, direction);
      moving = false;                      // NEW: stop motor until button pressed
      ignoreLimit2 = true;
    }
    if (ignoreLimit1 && digitalRead(limit1pin) == HIGH) ignoreLimit1 = false;
    if (ignoreLimit2 && digitalRead(limit2pin) == HIGH) ignoreLimit2 = false;

    // Step the motor
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);

    // auto-stop only when dirPin HIGH
    if (digitalRead(dirPin) == HIGH) {
      stepCount++;
      if (stepCount >= targetSteps) {
        moving = false;
      }
    }
  }
}