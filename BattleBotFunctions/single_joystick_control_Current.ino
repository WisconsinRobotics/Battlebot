#include <Bluepad32.h>
#include <ezButton.h>
#include <ESP32Servo.h>
#include <BattleBotFunctions.h>


/*
A minimally modified version of the bluepad32 example that will drive the motors using one joystick control
*/

#define ONBOARD_LED 2


// maxGamePads
const uint8_t maxGamePads = 1;
GamepadPtr myGamepads[maxGamePads];
Servo hammerServo;
const int PIN_MOTOR_1A = 16;  //pwm pins to drive the left motor
const int PIN_MOTOR_1B = 17;
const int PIN_MOTOR_2A = 18;  //pwm pins to drive the right motor
const int PIN_MOTOR_2B = 19;

BattleBotFunctions botFunctions = BattleBotFunctions(PIN_MOTOR_1A, PIN_MOTOR_1B, PIN_MOTOR_2A, PIN_MOTOR_2B);

ezButton button1(12);
ezButton button2(13);
ezButton button3(14);
ezButton button4(27);

ezButton limitSwitches[] = { button1, button2, button3, button4 };

const uint8_t PIN_SERVO = 5;  //pwm pin to drive the servo

const uint hammer_cooldown = 10000;  //duration after which to allow another hammer strike
int servo_status = 0;

int lifeLEDs[] = {21,33,26}; // TODO: Find the right pins for these
int lives = 3; //(int) (sizeof(lifeLEDs)/sizeof(int));

hw_timer_t *led_Timer = NULL;
hw_timer_t *hammer_Timer = NULL;

void IRAM_ATTR DebugLED() {
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
}

void IRAM_ATTR ServoTimeout() {
  //Serial.printf("%d", servo_status);
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

// Arduino setup function. Runs in CPU 1
void setup() {
  Serial.begin(115200);
  // sets motor pins to output pins

  hammerServo.attach(PIN_SERVO);


  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();

  // setup lifeLEDs and limit switches for hit detection
  for (int i = 0; i < 4; i++) {
    limitSwitches[i].setDebounceTime(50);
  }
  for(int i = 0; i < lives; i++){
    pinMode(lifeLEDs[i],OUTPUT);
  }

  led_Timer = timerBegin(0, 8000, true); // Sets timer to increment ~0.1 ms
  timerAttachInterrupt(led_Timer, &DebugLED, true); // Function attached
  timerAlarmWrite(led_Timer, 10000, true); // Timer is set to call function every 1 second
  timerAlarmEnable(led_Timer);  // Start timer

  hammer_Timer = timerBegin(1, 8000, true); 
  timerAttachInterrupt(hammer_Timer, &ServoTimeout, true);
  timerAlarmWrite(hammer_Timer, hammer_cooldown, true); // False makes timer only run once
  timerAlarmEnable(hammer_Timer);  // Start timer

  botFunctions.begin();
  pinMode(ONBOARD_LED, OUTPUT);
}


// Arduino loop function. Runs in CPU 1
void loop() {
 
  
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
      
      // Serial.printf("Gamepad is connected\n");

      int input_Y = -(myGamepad->axisY() >> 1);  // y axis is flipped for some reason, so unflip it
      int input_X = myGamepad->axisX() >> 1;     // x axis is seemingly not flipped for some reason
      int input_3 = myGamepad->l2() | myGamepad->r2();

      if (abs(input_Y) < 10) input_Y = 0;
      if (abs(input_X) < 10) input_X = 0;

      analogWriteFrequency(300);
      int power_L = input_Y + input_X;
      int power_R = input_Y - input_X;
       Serial.printf("lpower: %d, rpower: %d\n", power_L, power_R);

      botFunctions.driveMotors(power_L, power_R);
      
      lives = botFunctions.detectHit(limitSwitches, lifeLEDs, lives);
      botFunctions.swingHammer(&servo_status, input_3, &hammerServo, hammer_Timer);
      Serial.println("Lives: ");
      Serial.println(lives);
      
      
      // See Gamepad.h For all the available functions.
    }
  }
  // The main loop must have some kind of "yield to lower priority task" event.
  // Otherwise the watchdog will get triggered.
  // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
  // Detailed info here:
  // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

  vTaskDelay(1);
}
