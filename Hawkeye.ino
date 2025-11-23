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
int motorSpeed = 83;
int delayMotion = 1000;

bool ignoreLimit1 = false;
bool ignoreLimit2 = false;
bool direction = true;  

bool ledState = false;       
bool lastButtonState = HIGH; 

// step counter
long stepCount = 0;
const long targetSteps = 12000; // 10 mL in 2 seconds

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
  // --- LED toggle logic ---
  bool buttonState = digitalRead(toggleButtonPin);
  if (lastButtonState == HIGH && buttonState == LOW) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? HIGH : LOW);
    delay(50); 
    moving = !moving;
    stepCount = 0; // reset counter on new start
  }
  lastButtonState = buttonState;

  if (digitalRead(limitStopPin) == LOW) {
    moving = false;
  }

  if (moving) {
    // limit switch logic
    if (!ignoreLimit1 && direction && digitalRead(limit1pin) == LOW) {
      delay(delayMotion);
      direction = !direction;
      digitalWrite(dirPin, direction);
      ignoreLimit1 = true;
    }
    if (!ignoreLimit2 && !direction && digitalRead(limit2pin) == LOW) {
      delay(delayMotion);
      direction = !direction;
      digitalWrite(dirPin, direction);
      ignoreLimit2 = true;
    }
    if (ignoreLimit1 && digitalRead(limit1pin) == HIGH) ignoreLimit1 = false;
    if (ignoreLimit2 && digitalRead(limit2pin) == HIGH) ignoreLimit2 = false;

    // Step the motor
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);

    // --- NEW: conditional stop logic ---
    if (digitalRead(dirPin) == HIGH) {
      // auto-stop after 10 mL
      stepCount++;
      if (stepCount >= targetSteps) {
        moving = false;
        digitalWrite(ledPin, LOW);
      }
    }
    // if dirPin == LOW → no auto-stop, only manual button toggles moving
  }
}