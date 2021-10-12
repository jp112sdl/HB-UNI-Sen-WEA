#ifndef _SENS_BMP280_H_
#define _SENS_BMP280_H_

#include <Wire.h>
#include <Sensors.h>
#include <Adafruit_BMP280.h>
namespace as {

class Sens_Bmp280 : public Sensor {

  int16_t   _temperature;
  uint16_t  _pressure;

protected:
 Adafruit_BMP280 bmp;
 Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
 Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
public:

  Sens_Bmp280 () : _temperature(0), _pressure(0) {}

  void init () {  
    if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  }

  void measure () {
    if (_present == true) {
     sensors_event_t temp_event, pressure_event;
     bmp_temp->getEvent(&temp_event);
     bmp_pressure->getEvent(&pressure_event);      
      _temperature = (int16_t)(temp_event.temperature * 10);
      _pressure    = (uint16_t)(pressure_event.pressure * 10);
      
      DPRINTLN(F("BMP280:"));
      DPRINT(F("-T    : ")); DDECLN(_temperature);
      DPRINT(F("-P    : ")); DDECLN(_pressure);
    }
  }
  
  int16_t  temperature () { return _temperature; }
  uint16_t pressure ()    { return _pressure; }
};

}

#endif
