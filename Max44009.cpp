//
//    FILE: Max44009.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.9
// PURPOSE: library for MAX44009 lux sensor Arduino
//     URL: https://github.com/RobTillaart/Arduino/tree/master/libraries
//
// Released to the public domain
//
// JP112SDL MODIFIED FOR HB-UNI-Sen-WEA
//
// 0.1.9  - 2018-07-01 issue #108 Fix shift math
//          (thanks Roland vandecook)
// 0.1.8  - 2018-05-13 issue #105 Fix read register
//          (thanks morxGrillmeister)
// 0.1.7  - 2018-04-02 issue #98 extend constructor for ESP8266
// 0.1.6  - 2017-07-26 revert double to float
// 0.1.5  - updated history
// 0.1.4  - added setAutomaticMode() to max44009.h (thanks debsahu)
// 0.1.03 - added configuration
// 0.1.02 - added threshold code
// 0.1.01 - added interrupt code
// 0.1.00 - initial version

#include "Max44009.h"


Max44009::Max44009(const uint8_t address)
{
  _address = address;
  Wire.begin();
  // TWBR = 12; // Wire.setClock(400000);
  _data = 0;
  _error = 0;
}

uint32_t Max44009::getLux(void)
{
  uint8_t dhi = read(MAX44009_LUX_READING_HIGH);
  uint8_t dlo = read(MAX44009_LUX_READING_LOW);
  uint8_t e = dhi >> 4;
  uint32_t m = ((dhi & 0x0F) << 4) + (dlo & 0x0F);
  m <<= e;
  uint32_t val = m * 45;
  return val;
}

int Max44009::getError()
{
  int e = _error;
  _error = 0;
  return e;
}

void Max44009::setConfiguration(const uint8_t value)
{
  write(MAX44009_CONFIGURATION, value);
}

uint8_t Max44009::getConfiguration()
{
  return read(MAX44009_CONFIGURATION);
}

void Max44009::setAutomaticMode()
{
  uint8_t config = read(MAX44009_CONFIGURATION);
  config &= ~MAX44009_CFG_CONTINUOUS; // off
  config &= ~MAX44009_CFG_MANUAL;     // off
  write(MAX44009_CONFIGURATION, config);
}

void Max44009::setContinuousMode()
{
  uint8_t config = read(MAX44009_CONFIGURATION);
  config |= MAX44009_CFG_CONTINUOUS; // on
  config &= ~MAX44009_CFG_MANUAL;    // off
  write(MAX44009_CONFIGURATION, config);
}

void Max44009::setManualMode(uint8_t CDR, uint8_t TIM)
{
  if (CDR != 0) CDR = 1;
  if (TIM > 7) TIM = 7;
  uint8_t config = read(MAX44009_CONFIGURATION);
  config &= ~MAX44009_CFG_CONTINUOUS; // off
  config |= MAX44009_CFG_MANUAL;      // on
  config &= 0xF0; // clear CDR & TIM bits
  config |= CDR << 3 | TIM;
  write(MAX44009_CONFIGURATION, config);
}

///////////////////////////////////////////////////////////
//
// PRIVATE
//
uint8_t Max44009::read(uint8_t reg)
{
  Wire.beginTransmission(_address);
  Wire.write(reg);
  _error = Wire.endTransmission();
  if (_error != 0)
  {
    return _data; // last value
  }
  if (Wire.requestFrom(_address, (uint8_t) 1) != 1)
  {
    _error = 10;
    return _data; // last value
  }
#if (ARDUINO <  100)
  _data = Wire.receive();
#else
  _data = Wire.read();
#endif
  return _data;
}

void Max44009::write(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value);
  _error = Wire.endTransmission();
}

// --- END OF FILE ---
