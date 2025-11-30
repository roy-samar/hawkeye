# Hawkeye

Summary

Description

- Simple Arduino sketch to drive a stepper (step/direction) with two limit switches and a stop switch. When a limit switch is hit the motor pauses, reverses direction, and ignores that switch until it releases.

Files

- `hawkeye.ino`

Board example

- Arduino Uno (avr)

Wiring / Pinout

- Direction pin: `dirPin = 2`
- Step pin: `stepPin = 3`
- Microstep pins (MS1/MS2/MS3): `ms1=8`, `ms2=9`, `ms3=10` (set to `HIGH` in sketch = quarter step on many drivers)
- Limit switch 1: `limit1pin = 11` (active LOW with `INPUT_PULLUP`)
- Limit switch 2: `limit2pin = 12` (active LOW with `INPUT_PULLUP`)
- Stop switch: `limitStopPin = 13` (active LOW with `INPUT_PULLUP`)

Key configuration (in `hawkeye.ino`)

```cpp
// step timing
int motorSpeed = 50;       // delayMicroseconds(motorSpeed) between step transitions
int delayMotion = 1000;    // milliseconds to pause when a limit switch is hit

// pins
const int dirPin = 2;
const int stepPin = 3;
const int ms1 = 8;
const int ms2 = 9;
const int ms3 = 10;
const int limit1pin = 11;
const int limit2pin = 12;
const int limitStopPin = 13;
```

Setup example (from `hawkeye.ino`)

```cpp
void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(ms1, OUTPUT);
  pinMode(ms2, OUTPUT);
  pinMode(ms3, OUTPUT);
  pinMode(limit1pin, INPUT_PULLUP);
  pinMode(limit2pin, INPUT_PULLUP);
  pinMode(limitStopPin, INPUT_PULLUP);

  // initial state
  digitalWrite(dirPin, HIGH);
  digitalWrite(ms1, HIGH);
  digitalWrite(ms2, HIGH);
  digitalWrite(ms3, HIGH);
}
```

Loop / stepping snippet

```cpp
void loop() {
  if (digitalRead(limitStopPin) == LOW) return; // stop pressed

  // simple step pulse
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(motorSpeed);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(motorSpeed);
}
```

Behavior summary

- The sketch checks the stop switch first and halts movement if pressed.
- When moving it checks the relevant limit for the current direction; if pressed it pauses `delayMotion` ms, toggles `direction`, and sets an ignore flag until the switch releases.

Build / Verify (arduino-cli)

Example compile command (run from the project folder):

```powershell
arduino-cli compile -b arduino:avr:uno --build-path ".\.arduino_build" "."
```

Example upload (replace COM port and board as needed):

```powershell
arduino-cli upload -p COM3 -b arduino:avr:uno --input-dir ".\.arduino_build"
```

VS Code task (optional)

Add a `.vscode/tasks.json` entry to run the compile command from VS Code. Example:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Arduino: Compile",
      "type": "shell",
      "command": "arduino-cli compile -b arduino:avr:uno --build-path \"${workspaceFolder}\\.arduino_build\" \"${workspaceFolder}\"",
      "group": "build"
    }
  ]
}
```

Troubleshooting

- "missing hawkeye.ino": Ensure the main sketch filename matches the folder name; this project contains `hawkeye.ino`.
- "cannot find Arduino.h" squiggles in VS Code: install the Arduino extension or add the Arduino core include paths to `.vscode/c_cpp_properties.json`.
- "Output path is not specified": use `--build-path` with `arduino-cli` or let the Arduino extension manage builds to enable incremental builds.

Customizing

- `motorSpeed` controls step pulse period; smaller = faster, but respect driver/motor limits.
- `ms1/ms2/ms3` values set microstepping on many drivers — consult your driver datasheet.

Safety

- Use proper current limiting on your stepper driver and a suitable power supply.
- Double-check limit switch wiring (using `INPUT_PULLUP` expects switches to pull the pin LOW when closed).

Next steps I can help with

- Add a wiring diagram (SVG) and embed it in this README.
- Create a `tasks.json` for upload or a full PlatformIO migration.

# Hawkeye

Summary

- **Description:**: Simple Arduino sketch to drive a stepper (step/direction) with two limit switches and a stop switch. When a limit switch is hit the motor pauses, reverses direction, and ignores that switch until it releases.

**Files:**: `hawkeye.ino`

**Board example:**: Arduino Uno (avr)

Wiring / Pinout

- **Direction pin:**: `dirPin` = `2`
- **Step pin:**: `stepPin` = `3`
- **Microstep pins (MS1/MS2/MS3):**: `ms1`=`8`, `ms2`=`9`, `ms3`=`10` (set to `HIGH` in sketch = quarter step in your driver)
- **Limit switch 1:**: `limit1pin` = `11` (active LOW with `INPUT_PULLUP`)
- **Limit switch 2:**: `limit2pin` = `12` (active LOW with `INPUT_PULLUP`)
- **Stop switch:**: `limitStopPin` = `13` (active LOW with `INPUT_PULLUP`)

Behavior / Configuration

- **motorSpeed**: `motorSpeed = 50` — used as `delayMicroseconds(motorSpeed)` between step pulses. Lower values = faster stepping (watch current/driver specs).
- **delayMotion**: `delayMotion = 1000` — milliseconds to pause after hitting a limit before reversing.
- **Ignore flags**: `ignoreLimit1` / `ignoreLimit2` prevent repeated triggers while the switch is still pressed (debounce + safe reverse).
- **direction**: boolean state written to `dirPin`. The sketch toggles this when a limit is hit.

How it works (brief)

- Main loop checks the stop switch first; if pressed the motor stops moving.
- If moving, it checks the currently relevant limit switch (based on `direction`). When the switch is pressed (LOW):
  - Pause for `delayMotion` ms
  - Toggle `direction` and update `dirPin`
  - Set the corresponding `ignoreLimit` flag until the switch releases
- While moving, the sketch toggles `stepPin` and waits `motorSpeed` microseconds between transitions to create step pulses.

Build / Verify (arduino-cli)

- Example compile command (from project root):

  ```powershell
  arduino-cli compile -b arduino:avr:uno --build-path ".\.arduino_build" "."
  ```

- If you use the Arduino IDE, open the folder as a sketch (file named `hawkeye.ino` inside the folder) and Verify/Upload normally.

Editor / IntelliSense notes

- If you use VS Code with the C/C++ extension, IntelliSense needs the Arduino core include paths. Example `.vscode/c_cpp_properties.json` should include the AVR core and avr-gcc include directories.

Troubleshooting

- "missing hawkeye.ino" — ensure the main sketch filename matches the folder name. This project includes `hawkeye.ino`.
- "cannot find Arduino.h" squiggles in VS Code — install the Arduino extension or add the core include paths to `.vscode/c_cpp_properties.json`.
- Build slow warning "Output path is not specified" — use `--build-path` with `arduino-cli` or let the Arduino extension manage builds.

Customizing

- Change `motorSpeed` to adjust step frequency. Use safe values for your stepper motor and driver.
- Adjust microstep pins `ms1/ms2/ms3` according to your stepper driver datasheet.

Safety

- Ensure motor current limiting and proper power supply for your stepper driver.
- Confirm limit switch wiring is correct (active LOW with pullups) to avoid unexpected motion.

If you want I can:

- Add a `README.md` with wiring diagram image placeholders, or
- Generate a `tasks.json` VS Code task to compile/upload with `arduino-cli`.
