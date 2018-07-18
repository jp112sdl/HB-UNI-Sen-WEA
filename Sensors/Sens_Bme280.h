
#ifndef _SENS_BME280_H_
#define _SENS_BME280_H_

#include <Wire.h>
#include <Sensors.h>
#include <BME280I2C.h>
#include <EnvironmentCalculations.h>

namespace as {

class Sens_Bme280 : public Sensor {

  int16_t   _temperature;
  uint16_t  _pressure;
  uint16_t  _pressureNN;
  uint8_t   _humidity;
  uint16_t  _dewPoint;
  uint8_t   _humidityAbs;
;

  BME280I2C _bme280;    		// Default : forced mode, standby time = 1000 ms, Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off

public:

  Sens_Bme280 () {}

  void init () {

    Wire.begin(); //ToDo sync with further I2C sensor classes

    uint8_t i = 10;
    while( (!_bme280.begin()) && (i > 0) ) {
      delay(100);
      i--;
    }
  
    switch(_bme280.chipModel()) {
     case BME280::ChipModel_BME280:
       _present = true;
       DPRINTLN(F("BME280 sensor OK"));
       break;
     default:
       DPRINTLN(F("BME280 sensor NOT OK"));
    }
  }

  void measure (uint16_t height) {
    if (_present == true) {
      float temp(NAN), hum(NAN), pres(NAN), presNN(NAN);
      _bme280.read(pres, temp, hum, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);
      
      _temperature = (int16_t)(temp * 10);
      _pressure    = (uint16_t)(pres * 10);
      _pressureNN  = (uint16_t)(EnvironmentCalculations::EquivalentSeaLevelPressure(float(height), temp, pres) * 10);
      _humidity    = (uint8_t)hum;
      _humidityAbs = (uint8_t)(EnvironmentCalculations::AbsoluteHumidity(temp, hum, EnvironmentCalculations::TempUnit_Celsius));
      _dewPoint    = (int16_t)(EnvironmentCalculations::DewPoint(temp, hum, EnvironmentCalculations::TempUnit_Celsius) * 10);
      
      DPRINTLN(F("BME280:"));
      DPRINT(F("-T    : ")); DDECLN(_temperature);
      DPRINT(F("-P    : ")); DDECLN(_pressure);
      DPRINT(F("-P(NN): ")); DDECLN(_pressureNN);
      DPRINT(F("-H    : ")); DDECLN(_humidity);
      DPRINT(F("-H(Ab): ")); DDECLN(_humidityAbs);
      DPRINT(F("-DP   : ")); DDECLN(_dewPoint);
    }
  }
  
  int16_t  temperature () { return _temperature; }
  uint16_t pressure ()    { return _pressure; }
  uint16_t pressureNN ()  { return _pressureNN; }
  uint8_t  humidity ()    { return _humidity; }
  uint8_t  humidityAbs () { return _humidityAbs; }
  int16_t  dewPoint ()    { return _dewPoint; }

};

}

#endif
