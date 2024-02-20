# BATTLEBOTS

This repository contains the code that controls the Outreach's Battlebots.

### Which Files to read

single_joystick_control.ino is the most up to date version of the Battle Bot code.
The usability of the other files are unknown.

### Implementation Details

single_joystick_control.ino uses Bluepad32.h to control the Battlebots. 

ezButton.h - reads the limit switches, when 3 limit switches are pressed the bot will lose a life.

### Known Issues

The servo code is still WIP.

Sometimes the gamepads can become unpaired, or both paired to a single bot. In case of this, unpair one of the controllers, then ensure that only the bot you want to pair with is turned on. Then follow the pairing instructions for your selected gamepad.


Servo swing/rebound time still needs tuning.

Currently needs to have a print statement run in each loop, other wise none of the mechanical components work. Speculated to be a synchonization issue but its still unclear.

Make sure to use S mode on the gamepads

### Dependencies
# Libraries:
ezButton
# Board:
DOIT ESP32 DEVKIT V1

# Setup Guide
https://docs.google.com/document/d/1oNa_Z8-wAw9iX4GBooJLb7eVVErQqwH0L6ALPNRNXWY/edit
