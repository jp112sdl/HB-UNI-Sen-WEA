//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-04-03 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_VEML6070_h__
#define __SENSORS_VEML6070_h__


#include <Sensors.h>
#include <Wire.h>
#include <Adafruit_VEML6070.h>

namespace as {

template <veml6070_integrationtime_t INTEGRATION_TIME=veml6070_integrationtime_t::VEML6070_1_T>
class Sens_Veml6070 : public Sensor {
  uint16_t  _uvvalue;
  uint8_t   _uvindex;
  ::Adafruit_VEML6070   _veml;
public:
  Sens_Veml6070 ()  {}
  void init () {
      _veml.begin(INTEGRATION_TIME);
      _present = true;
  }
  void measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _uvvalue = _veml.readUV();
      _uvindex = 0;
      if (_uvvalue >= 187)
        _uvindex = 1;
      if (_uvvalue >= 374)
        _uvindex = 2;
      if (_uvvalue >= 560)
        _uvindex = 3;
      if (_uvvalue >= 747)
        _uvindex = 4;
      if (_uvvalue >= 934)
        _uvindex = 5;
      if (_uvvalue >= 1120)
        _uvindex = 6;
      if (_uvvalue >= 1307)
        _uvindex = 7;
      if (_uvvalue >= 1494)
        _uvindex = 8;
      if (_uvvalue >= 1681)
        _uvindex = 9;
      if (_uvvalue >= 1868)
        _uvindex = 10;
      if (_uvvalue >= 2054)
        _uvindex = 11;  
    }
  }
  uint16_t UVValue ()  { return _uvvalue; }
  uint8_t UVIndex ()   { return _uvindex; }
};

}

#endif
