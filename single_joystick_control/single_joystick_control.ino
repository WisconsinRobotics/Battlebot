#include <Bluepad32.h>
#include <ezButton.h>
#include <ESP32Servo.h>

/*
A minimally modified version of the bluepad32 example that will drive the motors using one joystick control
*/

#define ONBOARD_LED 2


// maxGamePads
const uint8_t maxGamePads = 1;
GamepadPtr myGamepads[maxGamePads];
Servo hammerServo;

const uint8_t PIN_MOTOR_1A = 16;  //pwm pins to drive the left motor
const uint8_t PIN_MOTOR_1B = 17;

const uint8_t PIN_MOTOR_2A = 18;  //pwm pins to drive the right motor
const uint8_t PIN_MOTOR_2B = 19;

//  pinMode(12, INPUT_PULLUP);
//  pinMode(13, INPUT_PULLUP);
//  pinMode(14, INPUT_PULLUP);
//  pinMode(27, INPUT_PULLUP);

ezButton button1(12);
ezButton button2(13);
ezButton button3(14);
ezButton button4(27);



ezButton limitSwitches[] = { button1, button2, button3, button4 };

const uint8_t PIN_SERVO = 5;  //pwm pin to drive the servo

/*
uint servotimer = 0;                //timer for future servo implementation
const uint rebound = 1000;          //duration after which to raise the hammer
*/
const uint hammer_cooldown = 10000;  //duration after which to allow another hammer strike
int servo_status = 0; // 3 possible values: 0 - Resting/inital  1 - Downswing   2 - Upswing

//int ledPin1 = 21;
//int ledPin2 = 33;
//int ledPin3 = 26;
int lifeLEDs[] = {21,33,26}; // TODO: Find the right pins for these
int lives = sizeof(lifeLEDs) /sizeof(int);
//int lives = 3;

hw_timer_t *led_Timer = NULL; //https://deepbluembedded.com/esp32-timers-timer-interrupt-tutorial-arduino-ide/
hw_timer_t *hammer_Timer = NULL;

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedGamepad(GamepadPtr gp) {
  bool foundEmptySlot = false;
  for (int i = 0; i < maxGamePads; i++) {
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

  for (int i = 0; i < maxGamePads; i++) {
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


void IRAM_ATTR DebugLED() {
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
}

void IRAM_ATTR ServoTimeout() {
  //Serial.printf("Hammer Timeout Triggered\n");
  switch (servo_status){
    case 1: // When bottom reached
      servo_status = 2;
      break;
    case 2: // Reset for a hammer cooldown
      servo_status = 0;
      break;
    default: // Reset State, includes 0
      break;
  }

}


// Arduino setup function. Runs in CPU 1
void setup() {
  Serial.begin(115200);

  hammerServo.attach(PIN_SERVO);
  hammerServo.writeMicroseconds(890);


  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t *addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2],
                addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

  // "forgetBluetoothKeys()" should be called when the user performs a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();

  for (int i = 0; i < 4; i++) {
    limitSwitches[i].setDebounceTime(50);
  }
  for(int i = 0; i < sizeof(lifeLEDs) / sizeof(int); i++){
    pinMode(lifeLEDs[i],OUTPUT);
  }
  pinMode(ONBOARD_LED, OUTPUT);


  led_Timer = timerBegin(0, 8000, true); // Sets timer to increment ~0.1 ms
  timerAttachInterrupt(led_Timer, &DebugLED, true); // Function attached
  timerAlarmWrite(led_Timer, 10000, true); // Timer is set to call function every 1 second
  timerAlarmEnable(led_Timer);  // Start timer

  hammer_Timer = timerBegin(1, 8000, true); 
  timerAttachInterrupt(hammer_Timer, &ServoTimeout, true);
  timerAlarmWrite(hammer_Timer, hammer_cooldown, true); // False makes timer only run once
  timerAlarmEnable(hammer_Timer);  // Start timer

}

int pressed;
// Arduino loop function. Runs in CPU 1
void loop() {
  pressed = 0;
  for (int i = 0; i < 4; i++) {
    limitSwitches[i].loop();  // MUST call the loop() function first

    if (limitSwitches[i].isPressed()) {
      Serial.println("The limit switch: TOUCHED -> UNTOUCHED");
      pressed++;
    }
  }
  if (pressed >= 3){
    lives -= 1;
  }


  for(int i = 0; i < sizeof(lifeLEDs) / sizeof(int); i++){
    if(i < lives - 1){
      digitalWrite(lifeLEDs[i],HIGH);
    } else{
      digitalWrite(lifeLEDs[i],LOW);
    }
  }

  if (lives <= 0) {
    Serial.println("You Died.");
    delay(1500);
    lives = sizeof(lifeLEDs) / sizeof(int);
  }



  // This call fetches all the gamepad info from the NINA (ESP32) module.
  // Just call this function in your main loop.
  // The gamepads pointer (the ones received in the callbacks) gets updated
  // automatically.
  BP32.update();

  // It is safe to always do this before using the gamepad API.
  // This guarantees that the gamepad is valid and connected.
  for (int i = 0; i < maxGamePads; i++) {
    GamepadPtr myGamepad = myGamepads[i];

    if (myGamepad && myGamepad->isConnected()) {
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

      int input_Y = -(myGamepad->axisY() >> 1);  // y axis is flipped for some reason, so unflip it
      int input_X = myGamepad->axisX() >> 1;     // x axis is seemingly not flipped for some reason
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

      if (power_R > 0) {  //handle right motor driving
        analogWrite(PIN_MOTOR_2A, min(power_R, 255));
        analogWrite(PIN_MOTOR_2B, 0);
      } else {
        analogWrite(PIN_MOTOR_2A, 0);
        analogWrite(PIN_MOTOR_2B, min(-power_R, 255));
      }

      if (input_3 == 1 && servo_status == 0){
        servo_status = 1;
        Serial.printf("Swinging Hammer\n");

        timerRestart(hammer_Timer);
        hammerServo.writeMicroseconds(1600);
      } else if (servo_status >= 2){
        hammerServo.writeMicroseconds(860);

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
