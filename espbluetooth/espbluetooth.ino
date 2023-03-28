//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

//#define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
//const char *pin = "1234"; // Change this to more secure PIN.

String device_name = "ESP32-BT-Slave";
uint8_t buf[2];

const uint8_t PIN_MOTOR_1A = 2;
const uint8_t PIN_MOTOR_1B = 4;

const uint8_t PIN_MOTOR_2A = 18;
const uint8_t PIN_MOTOR_2B = 19;

const uint8_t DEADBAND_L1 = 111, DEADBAND_L2 = 144; //lower and upper bounds for left motor/joystick deadband
const uint8_t DEADBAND_R1 = 111, DEADBAND_R2 = 144; //lower and upper bounds for right motor/joystick deadband

uint8_t input_1 = 128;
uint8_t input_2 = 128;

uint servotimer = 0; //timer for future servo implementation

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    SerialBT.readBytes(buf, 2);
    Serial.print(int(buf[0]));
    Serial.print("\t");
    Serial.println(int(buf[1]));
    input_1 = buf[0];
    input_2 = buf[1];
  }
  if (input_1 > DEADBAND_L2) {
    analogWrite(PIN_MOTOR_1A, map(input_1, DEADBAND_L2, 255, 0, 255));
    digitalWrite(PIN_MOTOR_1B, 0);
  } else if (input_1 < DEADBAND_L1) {
    digitalWrite(PIN_MOTOR_1A, 0);
    analogWrite(PIN_MOTOR_1B, map(input_1, 0, DEADBAND_L1, 255, 0));    
  } else {
    analogWrite(PIN_MOTOR_1A, 0);
    analogWrite(PIN_MOTOR_1B, 0);
  }

  if (input_2 > DEADBAND_R2) {
    analogWrite(PIN_MOTOR_2A, map(input_2, DEADBAND_R2, 255, 0, 255));
    digitalWrite(PIN_MOTOR_2B, 0);
  } else if (input_2 < DEADBAND_R1) {
    digitalWrite(PIN_MOTOR_2A, 0);
    analogWrite(PIN_MOTOR_2B, map(input_2, 0, DEADBAND_R1, 255, 0));    
  } else {
    analogWrite(PIN_MOTOR_2A, 0);
    analogWrite(PIN_MOTOR_2B, 0);
  }
  
}
