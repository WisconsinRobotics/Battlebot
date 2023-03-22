#include "BluetoothSerial.h"
#include <Arduino.h>
#define servoPin 9
#define pwm1Pin 2
#define pwm2Pin 18

// Setting PWM properties
const int freq = 1000;
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int resolution = 8;
int dutyCycle = 0;

// Create a new servo object:
BluetoothSerial SerialBT;

// Create a variable to store the servo position:
int angle = 0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

void setup() {
  // sets the pins as outputs:
  pinMode(pwm1Pin, OUTPUT);
  pinMode(pwm2Pin, OUTPUT);
  
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel1, freq, resolution);
  ledcSetup(pwmChannel2, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(pwm1Pin, pwmChannel1);
  ledcAttachPin(pwm2Pin, pwmChannel2);

  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()){
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      Serial.write("Message Found");
    }
    Serial.write(incomingChar);  
  }
  delay(100);

  while (dutyCycle <= 255){
    ledcWrite(pwmChannel1, dutyCycle);
    ledcWrite(pwmChannel2, dutyCycle);
    Serial.print("Forward with duty cycle: ");
    Serial.println(dutyCycle);
    dutyCycle = dutyCycle + 5;
    delay(500);
  }
  dutyCycle = 0;

    // Tell the servo to go to a particular angle:
  //myservo.write(90);

  // Sweep from 0 to 180 degrees:
  /*
  for (angle = 0; angle <= 180; angle += 1) {
    myservo.write(angle);
    delay(20);
  }

  // And back from 180 to 0 degrees:
  for (angle = 180; angle >= 0; angle -= 1) {
    myservo.write(angle);
    delay(20);
  }
  delay(100);
   */
}

