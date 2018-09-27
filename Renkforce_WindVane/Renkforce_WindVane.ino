#include <SoftwareSerial.h>
#define PWM_OUT_PIN   PB4

SoftwareSerial wvSerial(PB3, PB5); // RX, TX

#define START_BYTE            0x2c

//uint8_t directionBytes1[4] = {0x24, 0x64, 0x2c, 0x6c };
//uint8_t directionBytes2[4] = {0x04, 0x0c, 0x01, 0x07 };

uint8_t directionBytes1[4] = {0x26, 0x66, 0x2e, 0x6e };
uint8_t directionBytes2[4] = {0x06, 0x0e, 0x01, 0x07 };

uint8_t receivedValues [4];
uint8_t idx = 0;
uint8_t pwmOutputValue = 0;
uint8_t lastPwmOutputValue = 0;

void setup() {
  wvSerial.begin(2400);
  //wvSerial.print("HELLO");
  pinMode(PWM_OUT_PIN, OUTPUT);
}

void loop() {
  if (wvSerial.available()) {
    uint8_t recv = wvSerial.read();
    if ((recv >> 2) == START_BYTE)  idx = 0;
    receivedValues[idx] = recv;
    //wvSerial.print(receivedValues[idx], HEX);
    //wvSerial.print(" : ");
    idx++;
    if (idx > 2) {
      idx = 0;
      //wvSerial.println("");
    }
  } else {
    if ((receivedValues[0] >> 2) == START_BYTE) {
      if ((receivedValues[1] == directionBytes1[0]) && (receivedValues[2] == directionBytes2[0])) pwmOutputValue = 0;     // N
      if ((receivedValues[1] == directionBytes1[0]) && (receivedValues[2] == directionBytes2[1])) pwmOutputValue = 15;    // NNO
      if ((receivedValues[1] == directionBytes1[0]) && (receivedValues[2] == directionBytes2[2])) pwmOutputValue = 30;    // NO
      if ((receivedValues[1] == directionBytes1[0]) && (receivedValues[2] == directionBytes2[3])) pwmOutputValue = 45;    // ONO

      if ((receivedValues[1] == directionBytes1[1]) && (receivedValues[2] == directionBytes2[0])) pwmOutputValue = 60;    // O
      if ((receivedValues[1] == directionBytes1[1]) && (receivedValues[2] == directionBytes2[1])) pwmOutputValue = 75;    // OSO
      if ((receivedValues[1] == directionBytes1[1]) && (receivedValues[2] == directionBytes2[2])) pwmOutputValue = 90;    // SO
      if ((receivedValues[1] == directionBytes1[1]) && (receivedValues[2] == directionBytes2[3])) pwmOutputValue = 105;   // SSO

      if ((receivedValues[1] == directionBytes1[2]) && (receivedValues[2] == directionBytes2[0])) pwmOutputValue = 120;   // S
      if ((receivedValues[1] == directionBytes1[2]) && (receivedValues[2] == directionBytes2[1])) pwmOutputValue = 135;   // SSW
      if ((receivedValues[1] == directionBytes1[2]) && (receivedValues[2] == directionBytes2[2])) pwmOutputValue = 150;   // SW
      if ((receivedValues[1] == directionBytes1[2]) && (receivedValues[2] == directionBytes2[3])) pwmOutputValue = 165;   // WSW

      if ((receivedValues[1] == directionBytes1[3]) && (receivedValues[2] == directionBytes2[0])) pwmOutputValue = 180;   // W
      if ((receivedValues[1] == directionBytes1[3]) && (receivedValues[2] == directionBytes2[1])) pwmOutputValue = 195;   // WNW
      if ((receivedValues[1] == directionBytes1[3]) && (receivedValues[2] == directionBytes2[2])) pwmOutputValue = 210;   // NW
      if ((receivedValues[1] == directionBytes1[3]) && (receivedValues[2] == directionBytes2[3])) pwmOutputValue = 225;   // NNW

      if (pwmOutputValue != lastPwmOutputValue) {
        analogWrite(PWM_OUT_PIN, pwmOutputValue);
        lastPwmOutputValue = pwmOutputValue;
        //wvSerial.println("pwmOutputValue = " + String(pwmOutputValue));
      }
    }
    idx = 0;
  }
  delay(100);
}
