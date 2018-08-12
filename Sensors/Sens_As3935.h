//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-06-28 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENSORS_AS3935_h__
#define __SENSORS_AS3935_h__

#define  EI_NOTEXTERNAL
#include <EnableInterrupt.h>

#include <Sensors.h>
#include <SPI.h>

#include "../PWFusion_AS3935.h"
volatile uint8_t  _lightning_isr_counter = 0;

namespace as {

template <uint8_t _AS3935_IRQ_PIN = 3,uint8_t _AS3935_CS_PIN = 7>
class Sens_As3935 : public Sensor {
    ::PWF_AS3935 _lightningDetector;
    uint8_t _interrupt_src;
public:
  Sens_As3935 () : _lightningDetector(_AS3935_CS_PIN, _AS3935_IRQ_PIN) {}
    enum _AS3935_ENVIRONMENT {
        AS3935_ENVIRONMENT_OUTDOOR,
        AS3935_ENVIRONMENT_INDOOR
    };

  void init (uint8_t _AS3935_CAPACITANCE, uint8_t _AS3935_DIST_EN, uint8_t _AS3935_ENVIRONMENT, uint8_t _AS3935_MINSTRIKES, uint8_t _AS3935_WATCHDOGTHRESHOLD, uint8_t _AS3935_NOISEFLOORLEVEL, uint8_t _AS3935_SPIKEREJECTION) {
    if ( digitalPinToInterrupt(_AS3935_IRQ_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(_AS3935_IRQ_PIN, lightningISR, RISING); else attachInterrupt(digitalPinToInterrupt(_AS3935_IRQ_PIN), lightningISR, RISING);
    _lightningDetector.AS3935_Reset();
    _lightningDetector.AS3935_ManualCal(_AS3935_CAPACITANCE, _AS3935_DIST_EN, _AS3935_ENVIRONMENT);
    _lightningDetector.AS3935_SetMinStrikes(_AS3935_MINSTRIKES);
    _lightningDetector.AS3935_SetSpikeRejection(_AS3935_SPIKEREJECTION);
    _lightningDetector.AS3935_SetWatchdogThreshold(_AS3935_WATCHDOGTHRESHOLD);
    _lightningDetector.AS3935_SetNoiseFloorLvl(_AS3935_NOISEFLOORLEVEL);
    _lightningDetector.AS3935_PrintAllRegs();
    DPRINTLN("AS3935 Init done.");
    _present = true;
  }
    
  static void lightningISR() {
    _lightning_isr_counter = 1;
  }
    
  uint8_t GetInterruptSrc (__attribute__((unused)) bool async=false) {
    return ( present() == true ) ? _lightningDetector.AS3935_GetInterruptSrc() : 255;
  }
    
  uint8_t LightningDistKm (__attribute__((unused)) bool async=false) {
    return (present() == true) ? _lightningDetector.AS3935_GetLightningDistKm() : 255;
  }
    
  void PrintAllRegs() {
        _lightningDetector.AS3935_PrintAllRegs();
  }
    
  uint8_t LightningIsrCounter() { return _lightning_isr_counter; }
  bool ResetLightninIsrCounter() { _lightning_isr_counter = 0; return true; }

};

}

#endif
