//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-01-03 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef SENSORS_SENS_AS5600_H_
#define SENSORS_SENS_AS5600_H_

#include <Wire.h>

#define AS5600ADDRESS       0x36
#define ZMCOADDRESS         0x00
#define ZPOSADDRESSMSB      0x01
#define ZPOSADDRESSLSB      0x02
#define MPOSADDRESSMSB      0x03
#define MPOSADDRESSLSB      0x04
#define MANGADDRESSMSB      0x05
#define MANGADDRESSLSB      0x06
#define CONFADDRESSMSB      0x07
#define CONFADDRESSLSB      0x08
#define RAWANGLEADDRESSMSB  0x0C
#define RAWANGLEADDRESSLSB  0x0D
#define ANGLEADDRESSMSB     0x0E
#define ANGLEADDRESSLSB     0x0F
#define STATUSADDRESS       0x0B
#define AGCADDRESS          0x1A
#define MAGNITUDEADDRESSMSB 0x1B
#define MAGNITUDEADDRESSLSB 0x1C
#define BURNADDRESS         0xFF

namespace as {

class Sens_As5600  {
private:
  int16_t   _angle;
  bool      _present;

  uint8_t _getReg(uint8_t reg)
  {
    Wire.beginTransmission(AS5600ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();

    Wire.requestFrom(AS5600ADDRESS, 1);

    uint8_t _msb = 0;
    if(Wire.available() <=1) {
      _msb = Wire.read();
    }

    return _msb;
  }

  uint16_t _getRegs(uint8_t regMSB, uint8_t regLSB)
  {
    uint8_t _lsb = 0;
    uint8_t  _msb = 0;

    Wire.beginTransmission(AS5600ADDRESS);
    Wire.write(regMSB);
    Wire.endTransmission();
    delay(10);

    Wire.requestFrom(AS5600ADDRESS, 1);

    if(Wire.available() <=1) {
      _msb = Wire.read();
    }

    Wire.requestFrom(AS5600ADDRESS, 1);

    Wire.beginTransmission(AS5600ADDRESS);
    Wire.write(regLSB);
    Wire.endTransmission();

    if(Wire.available() <=1) {
      _lsb = Wire.read();
    }

    return (_lsb) + (_msb & 0b00001111) * 256;
  }
public:

  Sens_As5600 () : _angle(0), _present(false) {}

  void init () {
    Wire.begin();

    uint8_t status = _getReg(STATUSADDRESS) & 0b00111000;
    if (status == 32) {
      _present=true;
      DPRINTLN(F("AS5600 OK."));
    } else {
      DPRINTLN(F("AS5600 FAILURE."));
    }
  }

  void measure () {
    if (_present) {
      uint16_t raw = _getRegs(RAWANGLEADDRESSMSB, RAWANGLEADDRESSLSB);
      _angle = map(raw, 0, 4096, 0, 359);
    }
  }

  int16_t  angle () { return _angle; }
};

}



#endif /* SENSORS_SENS_AS5600_H_ */
