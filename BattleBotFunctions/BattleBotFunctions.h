#include <Arduino.h>
#include <ESP32Servo.h>
#include <ezButton.h>
#include <Bluepad32.h>
class BattleBotFunctions
{
  public:
    BattleBotFunctions(int PIN_MOTOR_1A, int PIN_MOTOR_1B, int PIN_MOTOR_2A, int PIN_MOTOR_2B);
    void begin();
    int detectHit(ezButton limitSwitches[], int lifeLEDs[], int lives);
    void driveMotors(int power_L, int power_R);
    void swingHammer(int *servo_status, int input, Servo *hammerServo, hw_timer_t *hammer_Timer);
  private:
    int _PIN_MOTOR_1A;  //pwm pins to drive the left motor
    int _PIN_MOTOR_1B;
    int _PIN_MOTOR_2A;  //pwm pins to drive the right motor
    int _PIN_MOTOR_2B;
};

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
          */