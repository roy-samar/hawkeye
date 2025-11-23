#line 1 "C:\\Users\\roysa\\OneDrive\\Documents\\Projects\\hawkeye\\hawkeye.ino"
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

bool moving = true;
int motorSpeed = 50;
int delayMotion = 1000;

// Track if we're ignoring the limit switch until it releases
bool ignoreLimit1 = false;
bool ignoreLimit2 = false;
bool direction = true;  // true for one direction, false for the other

#line 22 "C:\\Users\\roysa\\OneDrive\\Documents\\Projects\\hawkeye\\hawkeye.ino"
void setup();
#line 39 "C:\\Users\\roysa\\OneDrive\\Documents\\Projects\\hawkeye\\hawkeye.ino"
void loop();
#line 22 "C:\\Users\\roysa\\OneDrive\\Documents\\Projects\\hawkeye\\hawkeye.ino"
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
}

void loop() {
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
