#include <Arduino.h>

// pin connections
const int dirPin = 2; // direction pin
const int stepPin = 3; // step pin
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

// Track if we're ignoring the limit switch until it releases
bool ignoreLimit1 = false;
bool ignoreLimit2 = false;
bool direction = true;  // true for one direction, false for the other

bool ledState = false;       // track LED state
bool lastButtonState = HIGH; // track previous button state

void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(ms1, OUTPUT);
  pinMode(ms2, OUTPUT);
  pinMode(ms3, OUTPUT);
  pinMode(limit1pin, INPUT_PULLUP);
  pinMode(limit2pin, INPUT_PULLUP);
  pinMode(limitStopPin, INPUT_PULLUP);
  // set direction of rotation to clockwise
  digitalWrite(dirPin, direction);
  // set stepping mode to quarter step
  digitalWrite(ms1, HIGH);
  digitalWrite(ms2, HIGH);
  digitalWrite(ms3, HIGH);

  pinMode(ledPin, OUTPUT);
  pinMode(toggleButtonPin, INPUT_PULLUP);

  //digitalWrite(ledPin, HIGH);

}

void loop() {

  // --- LED toggle logic ---
  bool buttonState = digitalRead(toggleButtonPin);
  // detect transition: HIGH -> LOW (button press)
  if (lastButtonState == HIGH && buttonState == LOW) {
    ledState = !ledState; // flip LED state
    digitalWrite(ledPin, ledState ? HIGH : LOW);
    delay(50); // debounce
    moving = !moving;
  }

  lastButtonState = buttonState;


  if (digitalRead(limitStopPin)== LOW) {
    moving = false;
  }
  if (moving){
      // Check limit switches only if not ignoring
    if (!ignoreLimit1 && direction && digitalRead(limit1pin) == LOW) {
      // Hit limit 1, pause, reverse direction, and start ignoring this switch
      delay(delayMotion);
      direction = !direction;
      digitalWrite(dirPin, direction);
      ignoreLimit1 = true;
    }

    if (!ignoreLimit2 && !direction && digitalRead(limit2pin) == LOW) {
      // Hit limit 2, pause, reverse direction, and start ignoring this switch
      delay(delayMotion);
      direction = !direction;
      digitalWrite(dirPin, direction);
      ignoreLimit2 = true;
    }

    // After reversing, wait for switch to be released
    if (ignoreLimit1 && digitalRead(limit1pin) == HIGH) {
      ignoreLimit1 = false;
    }

    if (ignoreLimit2 && digitalRead(limit2pin) == HIGH) {
      ignoreLimit2 = false;
    }

    // Step the motor
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
  }
  
}