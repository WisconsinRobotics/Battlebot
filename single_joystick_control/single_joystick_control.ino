#include <Bluepad32.h>

/*
A minimally modified version of the bluepad32 example that will drive the motors using one joystick control
*/

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

const uint8_t PIN_MOTOR_1A = 16; //pwm pins to drive the left motor
const uint8_t PIN_MOTOR_1B = 17;

const uint8_t PIN_MOTOR_2A = 18; //pwm pins to drive the right motor
const uint8_t PIN_MOTOR_2B = 19;

const uint8_t PIN_SERVO = 5; //pwm pin to drive the servo

uint servotimer = 0; //timer for future servo implementation
const uint rebound = 20000; //duration after which to raise the hammer
const uint hammer_cooldown = 40000; //duration after which to allow another hammer strike


// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedGamepad(GamepadPtr gp) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == nullptr) {
      Serial.printf("CALLBACK: Gamepad is connected, index=%d\n", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      GamepadProperties properties = gp->getProperties();
      Serial.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n",
                    gp->getModelName().c_str(), properties.vendor_id,
                    properties.product_id);
      myGamepads[i] = gp;
      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
        "CALLBACK: Gamepad connected, but could not found empty slot");
  }
}

void onDisconnectedGamepad(GamepadPtr gp) {
  bool foundGamepad = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == gp) {
      Serial.printf("CALLBACK: Gamepad is disconnected from index=%d\n", i);
      myGamepads[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }

  if (!foundGamepad) {
    Serial.println(
        "CALLBACK: Gamepad disconnected, but not found in myGamepads");
  }
}

// Arduino setup function. Runs in CPU 1
void setup() {
  Serial.begin(115200);
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t *addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2],
                addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();
}

bool aOS{false};

// Arduino loop function. Runs in CPU 1
void loop() {
  // This call fetches all the gamepad info from the NINA (ESP32) module.
  // Just call this function in your main loop.
  // The gamepads pointer (the ones received in the callbacks) gets updated
  // automatically.
  BP32.update();

  // It is safe to always do this before using the gamepad API.
  // This guarantees that the gamepad is valid and connected.
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    GamepadPtr myGamepad = myGamepads[i];

    if (myGamepad && myGamepad->isConnected()) {
      // There are different ways to query whether a button is pressed.
      // By query each button individually:
      //  a(), b(), x(), y(), l1(), etc...
      if (myGamepad->a()) {
        
        static int colorIdx = 0;
        // Some gamepads like DS4 and DualSense support changing the color LED.
        // It is possible to change it by calling:
        switch (colorIdx % 3) {
        case 0:
          // Red
          myGamepad->setColorLED(255, 0, 0);
          break;
        case 1:
          // Green
          myGamepad->setColorLED(0, 255, 0);
          break;
        case 2:
          // Blue
          myGamepad->setColorLED(0, 0, 255);
          break;
        }
        colorIdx++;
      } 

      if (myGamepad->b() && !aOS) {
        aOS = true;
        // Turn on the 4 LED. Each bit represents one LED.
        static int led = 0;
        led++;
        // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
        // support changing the "Player LEDs": those 4 LEDs that usually
        // indicate the "gamepad seat". It is possible to change them by
        // calling:
        myGamepad->setPlayerLEDs(led & 0x0f);
      } else if (!myGamepad->b()){
        aOS = false;
      }

      if (myGamepad->x()) {
        // Duration: 255 is ~2 seconds
        // force: intensity
        // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S support
        // rumble.
        // It is possible to set it by calling:
        myGamepad->setRumble(0xc0 /* force */, 0xc0 /* duration */);
      }

      // Another way to query the buttons, is by calling buttons(), or
      // miscButtons() which return a bitmask.
      // Some gamepads also have DPAD, axis and more.
      /*Serial.printf(
          "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: "
          "%4d, %4d, brake: %4d, throttle: %4d, misc: 0x%02x, gyro x:%6d y:%6d "
          "z:%6d, accel x:%6d y:%6d z:%6d\n",
          i,                        // Gamepad Index
          myGamepad->dpad(),        // DPAD
          myGamepad->buttons(),     // bitmask of pressed buttons
          myGamepad->axisX(),       // (-511 - 512) left X Axis
          myGamepad->axisY(),       // (-511 - 512) left Y axis
          myGamepad->axisRX(),      // (-511 - 512) right X axis
          myGamepad->axisRY(),      // (-511 - 512) right Y axis
          myGamepad->brake(),       // (0 - 1023): brake button
          myGamepad->throttle(),    // (0 - 1023): throttle (AKA gas) button
          myGamepad->miscButtons(), // bitmak of pressed "misc" buttons
          myGamepad->gyroX(),       // Gyro X
          myGamepad->gyroY(),       // Gyro Y
          myGamepad->gyroZ(),       // Gyro Z
          myGamepad->accelX(),      // Accelerometer X
          myGamepad->accelY(),      // Accelerometer Y
          myGamepad->accelZ()       // Accelerometer Z
      );*/

    int input_Y = -(myGamepad->axisY() >> 1); // y axis is flipped for some reason, so unflip it
    int input_X = myGamepad->axisX() >> 1; // x axis is seemingly not flipped for some reason
    int input_3 = myGamepad->l2() | myGamepad->r2();

    if (abs(input_Y) < 10) input_Y = 0;
    if (abs(input_X) < 10) input_X = 0;

    analogWriteFrequency(300);
    int power_L = input_Y + input_X;
    int power_R = input_Y - input_X;
    Serial.printf("lpower: %d, rpower: %d\n", power_L, power_R);

    if (power_L > 0) {
      analogWrite(PIN_MOTOR_1A, min(power_L, 255));
      analogWrite(PIN_MOTOR_1B, 0);
    } else {
      analogWrite(PIN_MOTOR_1A, 0);
      analogWrite(PIN_MOTOR_1B, min(-power_L, 255));    
    }

    if (power_R > 0) { //handle right motor driving
      analogWrite(PIN_MOTOR_2A, min(power_R, 255));
      analogWrite(PIN_MOTOR_2B, 0);
    } else {
      analogWrite(PIN_MOTOR_2A, 0);
      analogWrite(PIN_MOTOR_2B, min(-power_R, 255));    
    }

    if (input_3 == 1 && servotimer == 0) {
      servotimer++;
      analogWrite(PIN_SERVO, 180); //start swinging the hammer (servo range is [40, 180])
    } else if (servotimer > 0) {
      servotimer++; //advance the timer
      if (servotimer == rebound) {
        analogWrite(PIN_SERVO, 40); //bring the hammer back to vertical
      } else if (servotimer == hammer_cooldown) {
        servotimer = 0;
      }
    } else {
      analogWrite(PIN_SERVO, 40);
    }

      // You can query the axis and other properties as well. See Gamepad.h
      // For all the available functions.
    }
  }

  // The main loop must have some kind of "yield to lower priority task" event.
  // Otherwise the watchdog will get triggered.
  // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
  // Detailed info here:
  // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

  vTaskDelay(1);
  //delay(150);
}
