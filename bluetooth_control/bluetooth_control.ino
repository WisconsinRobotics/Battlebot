#include "BluetoothSerial.h"
#define servoPin 9
#include <Arduino.h>

// Create a new servo object:
BluetoothSerial SerialBT;

// Create a variable to store the servo position:
int angle = 0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()){
    analogWrite(servoPin, 5);
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      Serial.write("Message Found");
    }
    Serial.write(incomingChar);  
    analogWrite(servoPin, 0);
  }

    // Tell the servo to go to a particular angle:
  //myservo.write(90);
  delay(1000);

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

