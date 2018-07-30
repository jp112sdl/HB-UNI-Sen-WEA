//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// some parts (BME280 measurement) from HB-UNI-Sensor1
// 2018-05-11 Tom Major (Creative Commons)
// 2018-05-21 jp112sdl (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------
// #define NDEBUG   // disable all serial debuf messages
// #define NSENSORS // if defined, only fake values are used

#define  EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <SPI.h>  // after including SPI Library - we can use LibSPI class
#include <AskSinPP.h>
#include <Register.h>

#include <MultiChannelDevice.h>
#include <sensors/Max44009.h>
#include <sensors/Veml6070.h>
#include "Sensors/Sens_Bme280.h"
#include "Sensors/Sens_As3935.h"

#define CONFIG_BUTTON_PIN                    8     // Anlerntaster-Pin

////////// REGENDETEKTOR
#define USE_RAINDETECTOR_STALLBIZ
//bei Verwendung der Regensensorplatine von stall.biz (https://www.stall.biz/produkt/regenmelder-sensorplatine)
#define RAINDETECTOR_STALLBIZ_SENS_PIN            A3   // Pin, an dem der Kondensator angeschlossen ist (hier wird der analoge Wert für die Regenerkennung ermittelt)
#define RAINDETECTOR_STALLBIZ_CRG_PIN             4    // Pin, an dem der Widerstand für die Kondensatoraufladung angeschlossen is
#define RAINDETECTOR_STALLBIZ_HEAT_PIN            9    // Pin, an dem der Transistor für die Heizung angeschlossen ist
#define RAINDETECTOR_STALLBIZ_RAIN_HI_THRESHOLD   750  // analoger Messwert, über dem 'Regen erkannt' angezeigt wird 
#define RAINDETECTOR_STALLBIZ_RAIN_LO_THRESHOLD   500  // analoger Messwert, unter dem 'Regen erkannt' nicht mehr angezeigt wird 
#define RAINDETECTOR_STALLBIZ_HEAT_HI_THRESHOLD   500  // analoger Messwert, über dem die Heizung aktiviert wird
#define RAINDETECTOR_STALLBIZ_HEAT_LO_THRESHOLD   400  // analoger Messwert, unter dem die Heizung wieder deaktiviert wird

//bei Verwendung eines Regensensors mit H/L-Pegel Ausgang
#define RAINDETECTOR_PIN                     9    // Pin, an dem der Regendetektor angeschlossen ist
#define RAINDETECTOR_PIN_LEVEL_ON_RAIN       LOW  // Pegel, wenn Regen erkannt wird

#define RAINDETECTOR_CHECK_INTERVAL          5     // alle x Sekunden auf Regen prüfen
/////////////////////////////////////////////////////////////////////

#define WINDSPEEDCOUNTER_PIN                 5     // Anemometer
#define RAINQUANTITYCOUNTER_PIN              6     // Regenmengenmesser

#define AS3935_IRQ_PIN                       3     // IRQ Pin des Blitzdetektors
#define AS3935_CS_PIN                        7     // CS Pin des Blitzdetektors

//                             N                      O                       S                         W
//entspricht Windrichtung in ° 0 , 22.5, 45  , 67.5, 90  ,112.5, 135, 157.5, 180 , 202.5, 225 , 247.5, 270 , 292.5, 315 , 337.5
const uint16_t WINDDIRS[] = { 33 , 71, 51 , 111, 93, 317, 292 , 781, 544, 650, 180, 197, 183, 703, 40 , 41 };
//(kleinste Werteabweichung / 2) - 1
#define WINDDIR_TOLERANCE                    3     // Messtoleranz
#define WINDDIRECTION_PIN                    A2    // Pin, an dem der Windrichtungsanzeiger angeschlossen ist


#define WINDSPEED_MEASUREINTERVAL_SECONDS    5     // Messintervall Wind / Böen
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//some static definitions
#define WINDSPEED_MAX              0x3FFF
#define GUSTSPEED_MAX              0x7FFF
#define RAINCOUNTER_MAX            0x7FFF
#define PEERS_PER_CHANNEL          4

using namespace as;

volatile uint32_t _rainquantity_isr_counter = 0;
volatile uint16_t _wind_isr_counter = 0;

enum eventMessageSources {EVENT_SRC_RAINING, EVENT_SRC_HEATING, EVENT_SRC_GUST};

const struct DeviceInfo PROGMEM devinfo = {
  {0xF1, 0xD0, 0x02},        // Device ID
  "JPWEA00002",           	 // Device Serial
  {0xF1, 0xD0},            	 // Device Model
  0x13,                   	 // Firmware Version
  as::DeviceType::THSensor,  // Device Type
  {0x01, 0x01}             	 // Info Bytes
};

// Configure the used hardware
typedef AskSin<NoLed, NoBattery, Radio<LibSPI<10>, 2>> Hal;
Hal hal;

class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint16_t airPressure, uint8_t humidity, uint32_t brightness, bool israining, bool isheating, uint16_t raincounter,  uint16_t windspeed, uint8_t winddir, uint8_t winddirrange, uint16_t gustspeed, uint8_t uvindex, uint8_t lightningcounter, uint8_t lightningdistance) {
      Message::init(0x1a, msgcnt, 0x70, BIDI | RPTEN, (temp >> 8) & 0x7f, temp & 0xff);
      pload[0] = (airPressure >> 8) & 0xff;
      pload[1] = airPressure & 0xff;
      pload[2] = humidity;
      pload[3] = (brightness >>  16) & 0xff;
      pload[4] = (brightness >>  8) & 0xff;
      pload[5] = brightness & 0xff;
      pload[6] = ((raincounter >> 8) & 0xff) | (israining << 7);
      pload[7] = raincounter & 0xff;
      pload[8] = (windspeed >> 8) & 0xff | (winddirrange << 6);
      pload[9] = windspeed & 0xff;
      pload[10] = winddir;
      pload[11] = (gustspeed >> 8) & 0xff | (isheating << 7);
      pload[12] = gustspeed & 0xff;
      pload[13] = (uvindex & 0xff) | (lightningdistance << 4);
      pload[14] = lightningcounter & 0xff;
    }
};

class ExtraEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, bool israining, bool isheating, uint16_t gustspeed) {
      Message::init(0x0d, msgcnt, 0x53, BIDI | RPTEN, 0x41, (israining & 0xff) | (isheating << 1));
      pload[0] =  (gustspeed >> 8) & 0xff;
      pload[1] =  gustspeed & 0xff;
    }
};

DEFREGISTER(Reg0, MASTERID_REGS, DREG_TRANSMITTRYMAX, 0x20, 0x21, 0x22, 0x23)
class SensorList0 : public RegList0<Reg0> {
  public:
    SensorList0(uint16_t addr) : RegList0<Reg0>(addr) {}

    bool updIntervall (uint16_t value) const {
      return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff);
    }
    uint16_t updIntervall () const {
      return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0);
    }

    bool height (uint16_t value) const {
      return this->writeRegister(0x22, (value >> 8) & 0xff) && this->writeRegister(0x23, value & 0xff);
    }
    uint16_t height () const {
      return (this->readRegister(0x22, 0) << 8) + this->readRegister(0x23, 0);
    }

    void defaults () {
      clear();
      updIntervall(60);
      height(0);
      transmitDevTryMax(6);
    }
};

DEFREGISTER(Reg1, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08)
class SensorList1 : public RegList1<Reg1> {
  public:
    SensorList1 (uint16_t addr) : RegList1<Reg1>(addr) {}
    bool AnemometerRadius (uint8_t value) const {
      return this->writeRegister(0x01, value & 0xff);
    }
    uint8_t AnemometerRadius () const {
      return this->readRegister(0x01, 0);
    }

    bool AnemometerCalibrationFactor (uint16_t value) const {
      return this->writeRegister(0x02, (value >> 8) & 0xff) && this->writeRegister(0x03, value & 0xff);
    }
    uint16_t AnemometerCalibrationFactor () const {
      return (this->readRegister(0x02, 0) << 8) + this->readRegister(0x03, 0);
    }

    bool LightningDetectorCapacitor (uint8_t value) const {
      return this->writeRegister(0x04, value & 0xff);
    }
    uint8_t LightningDetectorCapacitor () const {
      return this->readRegister(0x04, 0);
    }

    bool LightningDetectorDisturberDetection () const {
      return this->readBit(0x05, 0, true);
    }
    bool LightningDetectorDisturberDetection (bool v) const {
      return this->writeBit(0x05, 0, v);
    }

    bool ExtraMessageOnGustThreshold (uint8_t value) const {
      return this->writeRegister(0x06, value & 0xff);
    }
    uint8_t ExtraMessageOnGustThreshold () const {
      return this->readRegister(0x06, 0);
    }

    bool StormUpperThreshold (uint8_t value) const {
      return this->writeRegister(0x07, value & 0xff);
    }
    uint8_t StormUpperThreshold () const {
      return this->readRegister(0x07, 0);
    }

    bool StormLowerThreshold (uint8_t value) const {
      return this->writeRegister(0x08, value & 0xff);
    }
    uint8_t StormLowerThreshold () const {
      return this->readRegister(0x08, 0);
    }

    void defaults () {
      clear();
      AnemometerRadius(65);
      AnemometerCalibrationFactor(10);
      LightningDetectorCapacitor(80);
      LightningDetectorDisturberDetection(true);
      ExtraMessageOnGustThreshold(0);
      StormUpperThreshold(0);
      StormLowerThreshold(0);
    }
};

class WeatherChannel : public Channel<Hal, SensorList1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {
    int16_t       temperature;
    uint16_t      airPressure;
    uint8_t       humidity;
    uint32_t      brightness;
    bool          israining;
    bool          isheating;
    uint16_t      raincounter;
    uint16_t      windspeed;
    uint16_t      gustspeed;
    uint8_t       uvindex;
    uint8_t       lightningcounter;
    uint8_t       lightningdistance;

    uint8_t       winddir;
    uint8_t       winddirrange;

    bool          initComplete;
    bool          initLightningDetectorDone;
    uint8_t       short_interval_measure_count;
    uint8_t       israining_alarm_count;

    Sens_Bme280                 bme280;
    Veml6070<VEML6070_1_T>      veml6070;
    MAX44009<>                  max44009;

  public:
    Sens_As3935<AS3935_IRQ_PIN, AS3935_CS_PIN> as3935;

  public:
    WeatherChannel () : Channel(), Alarm(seconds2ticks(60)), israining_alarm_count(0), israining(false), initLightningDetectorDone(false), initComplete(false), windspeed(0), uvindex(0), short_interval_measure_count(0), wind_and_uv_measure(*this), lightning_and_raining_check(*this)  {}
    virtual ~WeatherChannel () {}

    class WindSpeedAndUVMeasureAlarm : public Alarm {
        WeatherChannel& chan;
      public:
        WindSpeedAndUVMeasureAlarm (WeatherChannel& c) : Alarm (seconds2ticks(WINDSPEED_MEASUREINTERVAL_SECONDS)), chan(c) {}
        virtual ~WindSpeedAndUVMeasureAlarm () {}

        void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
          chan.measure_windspeed();
          chan.measure_uvindex();
          tick = (seconds2ticks(WINDSPEED_MEASUREINTERVAL_SECONDS));
          clock.add(*this);
          chan.short_interval_measure_count++;
        }
    } wind_and_uv_measure;

    class LightningAndRainingAlarm : public Alarm {
        WeatherChannel& chan;
      public:
        LightningAndRainingAlarm (WeatherChannel& c) : Alarm (seconds2ticks(1)), chan(c) {}
        virtual ~LightningAndRainingAlarm () {}

        void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
          chan.measure_lightning();
          chan.measure_israining();
          tick = (seconds2ticks(1));
          clock.add(*this);
        }
    } lightning_and_raining_check;


    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      measure_winddirection();
      measure_thpb();
      measure_rainquantity();

      if (initComplete) {
        windspeed = windspeed / short_interval_measure_count;
        if (windspeed > WINDSPEED_MAX) windspeed = WINDSPEED_MAX;
        uvindex = uvindex / short_interval_measure_count;
      }

      //DPRINT(F("GUSTSPEED     : ")); DDECLN(gustspeed);
      //DPRINT(F("WINDSPEED     : ")); DDECLN(windspeed);
      //DPRINT(F("UV Index      : ")); DDECLN(uvindex);
      WeatherEventMsg& msg = (WeatherEventMsg&)device().message();
      uint8_t msgcnt = device().nextcount();
      msg.init(msgcnt, temperature, airPressure, humidity, brightness, israining, isheating, raincounter, windspeed, winddir, winddirrange, gustspeed, uvindex, lightningcounter, lightningdistance);
      if (msgcnt % 20 == 1) {
        device().sendPeerEvent(msg, *this);
      } else {
        device().broadcastPeerEvent(msg, *this);
      }
      uint16_t updCycle = this->device().getList0().updIntervall();
      tick = seconds2ticks(updCycle);

      initComplete = true;
      windspeed = 0;
      gustspeed = 0;
      uvindex = 0;
      short_interval_measure_count = 0;
      sysclock.add(*this);
    }

    void sendExtraMessage (uint8_t t) {
      DPRINT(F("SENDING EXTRA MESSAGE ")); DDECLN(t);
      ExtraEventMsg& extramsg = (ExtraEventMsg&)device().message();
      extramsg.init(device().nextcount(), israining, isheating, gustspeed);
      device().sendPeerEvent(extramsg, *this);
    }

    void measure_windspeed() {
#ifdef NSENSORS
      _wind_isr_counter = random(20);
#endif
      //V = 2 * R * Pi * N
      //  int kmph =  3.141593 * 2 * ((float)anemometerRadius / 100)   * ((float)_wind_isr_counter / (float)WINDSPEED_MEASUREINTERVAL_SECONDS)        * 3.6 * ((float)anemometerCalibrationFactor / 10);
      int kmph = ((226L * this->getList1().AnemometerRadius() * this->getList1().AnemometerCalibrationFactor() * _wind_isr_counter) / WINDSPEED_MEASUREINTERVAL_SECONDS) / 10000;
      if (kmph > gustspeed) {
        gustspeed = (kmph > GUSTSPEED_MAX) ? GUSTSPEED_MAX : kmph;
      }

      if (this->getList1().ExtraMessageOnGustThreshold() > 0 && kmph > (this->getList1().ExtraMessageOnGustThreshold() * 10)) {
        sendExtraMessage(EVENT_SRC_GUST);
      }

      //DPRINT(F("WIND kmph     : ")); DDECLN(kmph);
      static uint8_t STORM_PEER_VALUE_Last = 0;
      static uint8_t STORM_PEER_VALUE = 0;
      if (peers() > 0) {
        if (this->getList1().StormUpperThreshold() > 0) {
          if (kmph >= this->getList1().StormUpperThreshold() || kmph <= this->getList1().StormLowerThreshold()) {
            static uint8_t evcnt = 0;
            SensorEventMsg& rmsg = (SensorEventMsg&)device().message();

            if (kmph >= this->getList1().StormUpperThreshold()) STORM_PEER_VALUE = 200;
            if (kmph <= this->getList1().StormLowerThreshold()) STORM_PEER_VALUE = 100;

            if (STORM_PEER_VALUE != STORM_PEER_VALUE_Last) {
              rmsg.init(device().nextcount(), number(), evcnt++, STORM_PEER_VALUE, false , false);
              device().sendPeerEvent(rmsg, *this);
            }
            STORM_PEER_VALUE_Last = STORM_PEER_VALUE;
          }
        }
      }

      windspeed += kmph;
      _wind_isr_counter = 0;
    }

    void measure_israining() {
      static bool wasraining = false;

      if (initComplete) {
        if (israining_alarm_count >= RAINDETECTOR_CHECK_INTERVAL) {
#ifdef USE_RAINDETECTOR_STALLBIZ
          digitalWrite(RAINDETECTOR_STALLBIZ_CRG_PIN, HIGH);
          _delay_ms(2);
          digitalWrite(RAINDETECTOR_STALLBIZ_CRG_PIN, LOW);
          int rdVal = analogRead(RAINDETECTOR_STALLBIZ_SENS_PIN);
          DPRINT(F("RD aVal       : ")); DDECLN(rdVal);

          if (rdVal > RAINDETECTOR_STALLBIZ_RAIN_HI_THRESHOLD) {
            israining = true;
          }
          if (rdVal < (RAINDETECTOR_STALLBIZ_RAIN_LO_THRESHOLD)) {
            israining = false;
          }
          // Heizung einschalten, wenn Messwert > RAINDETECTOR_STALLBIZ_HEAT_THRESHOLD
          static bool mustheat = false;
          if (rdVal > RAINDETECTOR_STALLBIZ_HEAT_HI_THRESHOLD) {
            mustheat = true;
          }
          if (rdVal < (RAINDETECTOR_STALLBIZ_HEAT_LO_THRESHOLD)) {
            mustheat = false;
          }
          // Taubildung bei +/- 2°C Temperatur um den Taupunkt
          bool dewfall = bme280.present() ? (abs(bme280.temperature() - bme280.dewPoint()) < 20) : false;
          // Heizung schalten
          raindetector_heater(mustheat || dewfall);
#else
          israining = (digitalRead(RAINDETECTOR_PIN) == RAINDETECTOR_PIN_LEVEL_ON_RAIN);
#endif

          DPRINT(F("RD israining  : ")); DDECLN(israining);

          if (wasraining != israining) {
            sendExtraMessage(EVENT_SRC_RAINING);
            if (peers() > 0) {
              static uint8_t evcnt = 0;
              SensorEventMsg& rmsg = (SensorEventMsg&)device().message();
              rmsg.init(device().nextcount(), number(), evcnt++, israining ? 200 : 0, true , false);
              device().sendPeerEvent(rmsg, *this);
            }
          }
          wasraining = israining;
          israining_alarm_count = 0;
        }
        israining_alarm_count++;
      }
    }

    void raindetector_heater(bool State) {
#ifdef USE_RAINDETECTOR_STALLBIZ
      DPRINT(F("RD HEAT       : ")); DDECLN(State);
      isheating = State;
      digitalWrite(RAINDETECTOR_STALLBIZ_HEAT_PIN, State);
      static bool washeating = false;
      if (washeating != State) {
        sendExtraMessage(EVENT_SRC_HEATING);
      }
      washeating = State;
#endif
    }

    void measure_uvindex() {
#ifdef NSENSORS
      uvindex += random(11);
#else
      veml6070.measure();
      //DPRINT(F("UV readUV     : ")); DDECLN(veml6070.UVValue());
      uvindex += veml6070.UVIndex();
#endif
    }

    void measure_winddirection() {
      //Windrichtung Grad/3: 60° = 20; 0° = Norden
      winddir = 0;
      uint8_t idxwdir = 0;
#ifdef NSENSORS
      idxwdir = random(15);
      winddir = (idxwdir * 15 + 2 / 2) / 2;
#else
      uint16_t aVal = 0;
      for (uint8_t i = 0; i <= 0xf; i++) {
        aVal += analogRead(WINDDIRECTION_PIN);
      }
      aVal = aVal >> 4;

      for (uint8_t i = 0; i < sizeof(WINDDIRS) / sizeof(uint16_t); i++) {
        if (aVal < WINDDIRS[i] + WINDDIR_TOLERANCE && aVal > WINDDIRS[i] - WINDDIR_TOLERANCE) {
          idxwdir = i;
          winddir = (idxwdir * 15 + 2 / 2) / 2;
          break;
        }
      }
      //DPRINT(F("WINDDIR aVal  : ")); DDEC(aVal); DPRINT(F(" i = ")); DDECLN(idxwdir);
#endif

      //Schwankungsbreite
      static uint8_t idxoldwdir = 0;
      winddirrange = 3; // 0  - 3 (0, 22,5, 45, 67,5°)
      int idxdiff = abs(idxwdir - idxoldwdir);

      if (idxdiff <= 3) winddirrange = idxdiff;
      if (idxwdir <= 2 && idxoldwdir >= 13) winddirrange = (sizeof(WINDDIRS) / sizeof(uint16_t)) - idxdiff;
      if (winddirrange > 3) winddirrange = 3;

      idxoldwdir = idxwdir;

      //DPRINT(F("WINDDIR dir/3 : ")); DDECLN(winddir);
      //DPRINT(F("WINDDIR range : ")); DDECLN(winddirrange);
    }

    void measure_rainquantity() {
#ifdef NSENSORS
      _rainquantity_isr_counter++;
#endif
      if (!initComplete) {
        _rainquantity_isr_counter = 0;
        //DPRINTLN(F("RAINCOUNTER   : initalize"));
      } else {
        if (_rainquantity_isr_counter > RAINCOUNTER_MAX) {
          _rainquantity_isr_counter = 1;
        }
        raincounter = _rainquantity_isr_counter;
      }
      DPRINT(F("RAINCOUNTER   : ")); DDECLN(_rainquantity_isr_counter);
    }

    void measure_lightning() {
#ifdef NSENSORS
      lightningcounter++;
      lightningdistance = random(15);
#else
      if (!initLightningDetectorDone) {
        as3935.init(this->getList1().LightningDetectorCapacitor(), this->getList1().LightningDetectorDisturberDetection(), ::Sens_As3935<>::AS3935_ENVIRONMENT_OUTDOOR);
        initLightningDetectorDone = true;
      } else {
        uint8_t lightning_dist_km = 0;
        if (as3935.LightningIsrCounter() > 0) {
          switch (as3935.GetInterruptSrc()) {
            case 0:
              DPRINTLN(F("LD IRQ SRC NOT EXPECTED"));
              break;
            case 1:
              lightning_dist_km = as3935.LightningDistKm();
              DPRINT(F("LD LIGHTNING IN ")); DDEC(lightning_dist_km); DPRINTLN(" km");
              lightningcounter++;
              // Wenn Zähler überläuft (255 + 1), dann 1 statt 0
              if (lightningcounter == 0) lightningcounter = 1;
              lightningdistance = (lightning_dist_km + 1) / 3;
              break;
            case 2:
              DPRINTLN(F("LD DIST"));
              break;
            case 3:
              DPRINTLN(F("LD NOISE"));
              break;
          }
          as3935.ResetLightninIsrCounter();
        }
      }
#endif
      //DPRINT(F("LD CNT        : ")); DDECLN(lightningcounter);
      //DPRINT(F("LD DIST       : ")); DDECLN(lightningdistance);
    }

    void measure_thpb() {
      uint16_t height = this->device().getList0().height();
#ifdef NSENSORS
      airPressure = 9000 + random(2000);   // 1024 hPa +x
      humidity    = 66 + random(7);     // 66% +x
      temperature = 150 + random(50);   // 15C +x
      brightness = 1700000 + random(10000);   // 67000 Lux +x
      //DPRINT(F("        airPressure    : ")); DDECLN(airPressure);
      //DPRINT(F("        humidity       : ")); DDECLN(humidity);
      //DPRINT(F("        temperature    : ")); DDECLN(temperature);
      //DPRINT(F("        brightness     : ")); DDECLN(brightness);
#else
      bme280.measure(height);
      temperature = bme280.temperature();
      airPressure = bme280.pressureNN();
      humidity    = bme280.humidity();

      max44009.measure();
      brightness = max44009.brightness();
      //DPRINT(F("BRIGHTNESS    : ")  ); DDECLN(brightness);
#endif
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      tick = seconds2ticks(3);	// first message in 3 sec.
#ifndef NSENSORS
      max44009.init();
      bme280.init();
      veml6070.init();
#endif
      sysclock.add(*this);
      sysclock.add(wind_and_uv_measure);
      sysclock.add(lightning_and_raining_check);
    }

    void configChanged() {
      DPRINTLN("* Config changed       : List1");
      //DPRINTLN(F("* ANEMOMETER           : "));
      //DPRINT(F("*  - RADIUS            : ")); DDECLN(this->getList1().AnemometerRadius());
      //DPRINT(F("*  - CALIBRATIONFACTOR : ")); DDECLN(this->getList1().AnemometerCalibrationFactor());
      //DPRINT(F("*  - GUST MSG THRESHOLD: ")); DDECLN(this->getList1().ExtraMessageOnGustThreshold());
      //DPRINTLN(F("* LIGHTNINGDETECTOR    : "));
      //DPRINT(F("*  - CAPACITOR         : ")); DDECLN(this->getList1().LightningDetectorCapacitor());
      //DPRINT(F("*  - DISTURB.DETECTION : ")); DDECLN(this->getList1().LightningDetectorDisturberDetection());
      //DPRINT(F("PEERSETTING UPPER  = ")); DDECLN(this->getList1().StormUpperThreshold());
      //DPRINT(F("PEERSETTING LOWER  = ")); DDECLN(this->getList1().StormLowerThreshold());
      initLightningDetectorDone = false;
    }

    uint8_t status () const {
      return 0;
    }

    uint8_t flags () const {
      return 0;
    }
};

class SensChannelDevice : public MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> {
  public:
    typedef MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> TSDevice;
    SensChannelDevice(const DeviceInfo& info, uint16_t addr) : TSDevice(info, addr) {}
    virtual ~SensChannelDevice () {}

    virtual void configChanged () {
      TSDevice::configChanged();
      DPRINTLN("* Config Changed       : List0");
      DPRINT(F("* SENDEINTERVALL       : ")); DDECLN(this->getList0().updIntervall());
      //DPRINT(F("* ALTITUDE             : ")); DDECLN(this->getList0().height());
      //DPRINT(F("* TRANSMITTRYMAX       : ")); DDECLN(this->getList0().transmitDevTryMax());
    }
};

SensChannelDevice sdev(devinfo, 0x20);
ConfigButton<SensChannelDevice> cfgBtn(sdev);

void setup () {
#ifdef NDEBUG
  Serial.begin(57600);
#else
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
#endif
  sdev.init(hal);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.initDone();

  pinMode(RAINQUANTITYCOUNTER_PIN, INPUT_PULLUP);
  pinMode(WINDSPEEDCOUNTER_PIN, INPUT_PULLUP);
  pinMode(WINDDIRECTION_PIN, INPUT_PULLUP);

#ifdef USE_RAINDETECTOR_STALLBIZ
  pinMode(RAINDETECTOR_STALLBIZ_CRG_PIN, OUTPUT);
  pinMode(RAINDETECTOR_STALLBIZ_HEAT_PIN, OUTPUT);
  pinMode(RAINDETECTOR_STALLBIZ_SENS_PIN, INPUT);
#else
  pinMode(RAINDETECTOR_PIN, INPUT_PULLUP);
#endif

  if ( digitalPinToInterrupt(RAINQUANTITYCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(RAINQUANTITYCOUNTER_PIN, rainquantitycounterISR, RISING); else attachInterrupt(digitalPinToInterrupt(RAINQUANTITYCOUNTER_PIN), rainquantitycounterISR, RISING);
  if ( digitalPinToInterrupt(WINDSPEEDCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(WINDSPEEDCOUNTER_PIN, windspeedcounterISR, RISING); else attachInterrupt(digitalPinToInterrupt(WINDSPEEDCOUNTER_PIN), windspeedcounterISR, RISING);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    hal.activity.savePower<Idle<false, true>>(hal);
  }
}

void rainquantitycounterISR() {
  _rainquantity_isr_counter++;
}

void windspeedcounterISR() {
  _wind_isr_counter++;
}



