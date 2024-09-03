#include "BattleBotFunctions.h"
#include "ESP32Servo.h"
#include "ezButton.h"
#include "Bluepad32.h"

BattleBotFunctions::BattleBotFunctions(int PIN_MOTOR_1A, int PIN_MOTOR_1B, int PIN_MOTOR_2A, int PIN_MOTOR_2B) {
  _PIN_MOTOR_1A = PIN_MOTOR_1A;
  _PIN_MOTOR_1B = PIN_MOTOR_1B;
  _PIN_MOTOR_2A = PIN_MOTOR_2A;
  _PIN_MOTOR_2B = PIN_MOTOR_2B;
}

void BattleBotFunctions::begin(){

  pinMode(_PIN_MOTOR_1A, OUTPUT);
  pinMode(_PIN_MOTOR_1B, OUTPUT);
  pinMode(_PIN_MOTOR_2A, OUTPUT);
  pinMode(_PIN_MOTOR_2B, OUTPUT);

}
/**
if any of the limit switches are triggered, decrease the amount of lives
**/
int BattleBotFunctions::detectHit(ezButton limitSwitches[], int lifeLEDs[], int lives) {
  
  int pressed = 0;
  for (int i = 0; i < 4; i++) {
    limitSwitches[i].loop();  // MUST call the loop() function first

    if (limitSwitches[i].isPressed()) {
      Serial.println("The limit switch: TOUCHED -> UNTOUCHED");
      pressed++;
    }
  }
  if (pressed >= 1){
    lives -= 1;
  }

  for(int i = 0; i < 3; i++){
    if(i < lives - 1){
      digitalWrite(lifeLEDs[i],HIGH);
    } else{
      digitalWrite(lifeLEDs[i],LOW);
    }
  }

  if (lives <= 0) {
    Serial.println("You Died.");
    
      analogWrite(_PIN_MOTOR_1A, 0);
      analogWrite(_PIN_MOTOR_2A, 0);
      analogWrite(_PIN_MOTOR_1B, 0);
      analogWrite(_PIN_MOTOR_2B, 0);
      //hammerServo.writeMicroseconds(890);
      lives = 3;//sizeof(lifeLEDs) / sizeof(int);
      delay(1500);
  }
  return lives;
}

/**
Drives the motors based on the input from the left stick
**/
void BattleBotFunctions::driveMotors(int power_L, int power_R) {

  if (power_L > 0) {
        analogWrite(_PIN_MOTOR_1A, min(power_L, 255));
        analogWrite(_PIN_MOTOR_1B, 0);
      } else {
        analogWrite(_PIN_MOTOR_1A, 0);
        analogWrite(_PIN_MOTOR_1B, min(-power_L, 255));
      }

      if (power_R > 0) {  //handle right motor driving
        analogWrite(_PIN_MOTOR_2A, min(power_R, 255));
        analogWrite(_PIN_MOTOR_2B, 0);
      } else {
        analogWrite(_PIN_MOTOR_2A, 0);
        analogWrite(_PIN_MOTOR_2B, min(-power_R, 255));
      }
}

/**
Swings the hammer using the servo, delays, and brings hammer back to vertical
**/
void BattleBotFunctions::swingHammer(int *servo_status, int input, Servo *hammerServo, hw_timer_t *hammer_Timer) {
  //Serial.printf("input %d \t servo_status %d", input, *servo_status);
  if (input == 1 && *servo_status == 0){
        *servo_status = 1;
        Serial.printf("Swinging Hammer\n");

        timerRestart(hammer_Timer);
        Serial.printf("%ld",timerRead(hammer_Timer));

        hammerServo->writeMicroseconds(1600);
        delay(1000);
      } 
  else if (*servo_status >= 2){
        hammerServo->writeMicroseconds(890);
      }
}

