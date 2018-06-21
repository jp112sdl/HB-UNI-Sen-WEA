//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// some parts (BME280 measurement) from HB-UNI-Sensor1
// 2018-05-11 Tom Major (Creative Commons)
// 2018-05-21 jp112sdl (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------
// #define NDEBUG
// #define NSENSORS
// #define USE_OTA_BOOTLOADER

#define  EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <SPI.h>  // after including SPI Library - we can use LibSPI class
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>

#include <MultiChannelDevice.h>
#include <sensors/Bh1750.h>
#include "Sensors/Sens_Bme280.h"
#include "Sensors/Sens_VEML6070.h"

///////////////////////////////
// Lightning Detector AS3935
#include "PWFusion_AS3935.h"

#define AS3935_IRQ_PIN       3        // digital pins 2 and 3 are available for interrupt capability
#define AS3935_CS_PIN        7
#define AS3935_ADD           0x03     // x03 - standard PWF SEN-39001-R01 config
#define AS3935_CAPACITANCE   72       // <-- SET THIS VALUE TO THE NUMBER LISTED ON YOUR BOARD 

// defines for general chip settings
#define AS3935_INDOORS       0
#define AS3935_OUTDOORS      1
#define AS3935_DIST_DIS      0
#define AS3935_DIST_EN       1

PWF_AS3935 LightningDetector(AS3935_CS_PIN, AS3935_IRQ_PIN);
///////////////////////////////

#define LED_PIN             4
#define WINDCOUNTER_PIN     5
#define RAINCOUNTER_PIN     6
#define CONFIG_BUTTON_PIN   8
#define WINDDIRECTION_PIN   A2

#define SYSCLOCK_FACTOR     1.0 //only relevant when using sleep mode
#define BRIGHTNESS_FACTOR   1.2 //you have to multiply BH1750 raw value by 1.2

//                             N                      O                       S                         W
//entspricht Windrichtung in ° 0 , 22.5, 45  , 67.5, 90  ,112.5, 135, 157.5, 180 , 202.5, 225 , 247.5, 270 , 292.5, 315 , 337.5
const uint16_t WINDDIRS[] = { 523  , 570 ,  474 , 746 , 624  , 806 , 370, 407, 999 , 228 ,  215 , 773 , 279 , 304, 290  , 880 };
//(kleinste Werteabweichung / 2) - 1
#define WINDDIR_TOLERANCE   5
#define WINDSPEED_MEASUREINTERVAL_SECONDS 5

#define PEERS_PER_CHANNEL   2

using namespace as;

volatile uint32_t _rain_isr_counter = 0;
volatile uint16_t _wind_isr_counter = 0;
volatile uint8_t  _lightning_isr_counter = 0;

const struct DeviceInfo PROGMEM devinfo = {
  {0xF1, 0xD0, 0x02},        // Device ID
  "JPWEA00002",           	 // Device Serial
  {0xF1, 0xD0},            	 // Device Model
  0x10,                   	 // Firmware Version
  as::DeviceType::THSensor,  // Device Type
  {0x01, 0x01}             	 // Info Bytes
};

// Configure the used hardware
//typedef AvrSPI<10, 11, 12, 13> RadioSPI;
//typedef AskSin<StatusLed<LED_PIN>, NoBattery, Radio< AvrSPI<10, 11, 12, 13>, 2> > Hal;
typedef AskSin<StatusLed<LED_PIN>, NoBattery, Radio<LibSPI<10>, 2>> Hal;

Hal hal;

class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint16_t airPressure, uint8_t humidity, uint16_t brightness, uint16_t raincounter, uint16_t windspeed, uint8_t winddir, uint8_t winddirrange, uint16_t gustspeed, uint8_t uvindex, uint16_t lightningcounter, uint8_t lightningdistance) {
      Message::init(0x1a, msgcnt, 0x70, BCAST, (temp >> 8) & 0x7f, temp & 0xff);
      pload[0] = (airPressure >> 8) & 0xff;
      pload[1] = airPressure & 0xff;
      pload[2] = humidity;
      pload[3] = (brightness >>  8) & 0xff;
      pload[4] = brightness & 0xff;
      pload[5] = (raincounter >> 8) & 0xff;
      pload[6] = raincounter & 0xff;
      pload[7] = (windspeed >> 8) & 0xff | (winddirrange << 6);
      pload[8] = windspeed & 0xff;
      pload[9] = winddir;
      pload[10] = (gustspeed >> 8) & 0xff;
      pload[11] = gustspeed & 0xff;
      pload[12] = (uvindex & 0xff) | (lightningdistance << 4);
      pload[13] = (lightningcounter >> 8) & 0xff;
      pload[14] = lightningcounter & 0xff;
    }
};

DEFREGISTER(Reg0, MASTERID_REGS, 0x20, 0x21, 0x22, 0x23)
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
    }
};

DEFREGISTER(UReg1, 0x01, 0x02, 0x03)
class SensorList1 : public RegList1<UReg1> {
  public:
    SensorList1 (uint16_t addr) : RegList1<UReg1>(addr) {}
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

    void defaults () {
      clear();
      AnemometerRadius(65);
      AnemometerCalibrationFactor(10);
    }
};

class WeatherChannel : public Channel<Hal, SensorList1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {

    WeatherEventMsg msg;
    int16_t       temperature;
    uint16_t      airPressure;
    uint8_t       humidity;
    uint16_t      brightness;
    uint16_t      raincounter;
    uint16_t      windspeed;
    uint16_t      gustspeed;
    uint8_t       uvindex;
    uint16_t      lightningcounter;
    uint8_t       lightningdistance;

    uint8_t       winddir;
    uint8_t       idxoldwdir;
    uint8_t       winddirrange;
    bool          firstRun;
    uint8_t       short_interval_measure_count;

    uint8_t       anemometerRadius;
    uint8_t       anemometerCalibrationFactor;

    Sens_Bme280                 bme280;
    Sens_Veml6070<VEML6070_1_T> veml6070;
    Bh1750<>                    bh1750;

  public:
    WeatherChannel () : Channel(), Alarm(seconds2ticks(60)), firstRun(true), windspeed(0), uvindex(0), short_interval_measure_count(0), background_measure(*this), lightning_check(*this)  {}
    virtual ~WeatherChannel () {}

    class WindSpeedMeasureAlarm : public Alarm {
        WeatherChannel& chan;
      public:
        WindSpeedMeasureAlarm (WeatherChannel& c) : Alarm (seconds2ticks(WINDSPEED_MEASUREINTERVAL_SECONDS)), chan(c) {}
        virtual ~WindSpeedMeasureAlarm () {}

        void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
          chan.measure_windspeed();
          chan.measure_uvindex();
          tick = (seconds2ticks(WINDSPEED_MEASUREINTERVAL_SECONDS));
          clock.add(*this);
          chan.short_interval_measure_count++;
        }
    } background_measure;

    class LightningReceiveAlarm : public Alarm {
        WeatherChannel& chan;
      public:
        LightningReceiveAlarm (WeatherChannel& c) : Alarm (seconds2ticks(1)), chan(c) {}
        virtual ~LightningReceiveAlarm () {}

        void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
          chan.measure_lightning();
          tick = (seconds2ticks(1));
          clock.add(*this);
        }
    } lightning_check;

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      measure_winddirection();
      measure_thpb();
      measure_rain();

      if (!firstRun) {
        windspeed = windspeed / short_interval_measure_count;
        uvindex = uvindex / short_interval_measure_count;
      }

      DPRINT(F("GUSTSPEED gustspeed    : ")); DDECLN(gustspeed);
      DPRINT(F("WINDSPEED windspeed    : ")); DDECLN(windspeed);
      DPRINT(F("UV Index               : ")); DDECLN(uvindex);

      msg.init(device().nextcount(), temperature, airPressure, humidity, brightness, raincounter, windspeed, winddir, winddirrange, gustspeed, uvindex, lightningcounter, lightningdistance);

      device().sendPeerEvent(msg, *this);

      uint16_t updCycle = this->device().getList0().updIntervall();
      tick = seconds2ticks(updCycle * SYSCLOCK_FACTOR);
      clock.add(*this);
      firstRun = false;
      windspeed = 0;
      gustspeed = 0;
      uvindex = 0;
      short_interval_measure_count = 0;
    }

    void measure_windspeed() {
#ifdef NSENSORS
      _wind_isr_counter = random(20);
#endif
      //V = 2 * R * Pi * N
      int   kmph =  3.141593 * 2 * (anemometerRadius / 100.0) * (_wind_isr_counter / (WINDSPEED_MEASUREINTERVAL_SECONDS * SYSCLOCK_FACTOR)) * 3.6 * (anemometerCalibrationFactor / 10.0);
      if (kmph > gustspeed) {
        gustspeed = kmph;
      }
      windspeed += kmph;
      _wind_isr_counter = 0;
    }

    void measure_uvindex() {
#ifdef NSENSORS
      uvindex += random(11);
#else
      veml6070.measure();
      uvindex += veml6070.UVIndex();
#endif
    }

    void measure_winddirection() {
      //Windrichtung Grad/3: 60° = 20; 0° = Norden
      winddir = 0;
      uint8_t idxwdir = 0;
#ifdef NSENSORS
      idxwdir = random(15);
      winddir = idxwdir * 7.5;
#else
      uint16_t aVal = 0;
      for (uint8_t i = 0; i <= 0xf; i++) {
        aVal += analogRead(WINDDIRECTION_PIN);
      }
      aVal = aVal >> 4;

      for (uint8_t i = 0; i < sizeof(WINDDIRS) / sizeof(uint16_t); i++) {
        if (aVal < WINDDIRS[i] + WINDDIR_TOLERANCE && aVal > WINDDIRS[i] - WINDDIR_TOLERANCE) {
          idxwdir = i;
          winddir = i * 7.5;
          break;
        }
      }
      DPRINT(F("WINDDIR aVal           : ")); DDEC(aVal); DPRINT(F(" i = ")); DDECLN(idxwdir);
#endif

      //Schwankungsbreite
      winddirrange = 3; // 0  - 3 (0, 22,5, 45, 67,5°)
      int idxdiff = abs(idxwdir - idxoldwdir);

      if (idxdiff <= 3) winddirrange = idxdiff;
      if (idxwdir <= 2 && idxoldwdir >= 13) winddirrange = (sizeof(WINDDIRS) / sizeof(uint16_t)) - idxdiff;
      if (winddirrange > 3) winddirrange = 3;

      idxoldwdir = idxwdir;

      DPRINT(F("WINDDIR winddir/3      : ")); DDECLN(winddir);
      DPRINT(F("WINDDIR winddirrange   : ")); DDECLN(winddirrange);
    }

    void measure_rain() {
      DPRINT(F("RAINCOUNTER            : "));
      if (firstRun) {
        //manchmal wird bei Initialisierung die ISR ausgelöst, das setzen wir hier zurück
        _rain_isr_counter = 0;
        DPRINTLN(F("first run - nothing"));
      } else {
        raincounter = _rain_isr_counter;
        DDECLN(raincounter);
      }
    }

    void measure_lightning() {
#ifdef NSENSORS
      lightningcounter = random(65534);
      lightningdistance = random(15);
#else
      if (_lightning_isr_counter > 0) {
        uint8_t int_src = LightningDetector.AS3935_GetInterruptSrc();
        if (0 == int_src) {
          DPRINTLN(F("interrupt source result not expected"));
        }
        else if (1 == int_src) {
          uint8_t lightning_dist_km = LightningDetector.AS3935_GetLightningDistKm();
          DPRINT(F("Lightning detected in "));
          DDEC(lightning_dist_km);
          DPRINTLN(F(" kilometers"));
          lightningcounter++;
          lightningdistance = lightning_dist_km / 3;
        }
        else if (2 == int_src) {
          DPRINTLN(F("Disturber detected"));
        }
        else if (3 == int_src) {
          DPRINTLN(F("Noise high"));
        }
        //LightningDetector.AS3935_PrintAllRegs(); // for debug...
        _lightning_isr_counter = 0;
      }
#endif
    }

    void measure_thpb() {
      uint16_t height = this->device().getList0().height();
#ifdef NSENSORS
      airPressure = 9000 + random(2000);   // 1024 hPa +x
      humidity    = 66 + random(7);     // 66% +x
      temperature = 150 + random(50);   // 15C +x
      brightness = 67000 + random(1000);   // 67000 Lux +x
      DPRINT(F("        airPressure    : ")); DDECLN(airPressure);
      DPRINT(F("        humidity       : ")); DDECLN(humidity);
      DPRINT(F("        temperature    : ")); DDECLN(temperature);
      DPRINT(F("        brightness     : ")); DDECLN(brightness);
#else
      bme280.measure(height);
      temperature = bme280.temperature();
      airPressure = bme280.pressureNN();
      humidity    = bme280.humidity();

      bh1750.measure();
      brightness = bh1750.brightness() * BRIGHTNESS_FACTOR;
      DPRINT(F("BRIGHTNESS             : ")  ); DDECLN(brightness);
#endif
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      tick = seconds2ticks(3);	// first message in 3 sec.
#ifndef NSENSORS
      bh1750.init();
      bme280.init();
      veml6070.init();
      LightningDetector.AS3935_DefInit();
      LightningDetector.AS3935_ManualCal(AS3935_CAPACITANCE, AS3935_OUTDOORS, AS3935_DIST_EN);
      //LightningDetector.AS3935_PrintAllRegs();
      _lightning_isr_counter = 0;
#endif
      sysclock.add(*this);
      sysclock.add(background_measure);
      sysclock.add(lightning_check);
    }

    void configChanged() {
      anemometerRadius = this->getList1().AnemometerRadius();
      anemometerCalibrationFactor = this->getList1().AnemometerCalibrationFactor();
      DPRINTLN("* Config changed       : List1");
      DPRINTLN(F("* ANEMOMETER           : "));
      DPRINT(F("*  - RADIUS            : ")); DDECLN(anemometerRadius);
      DPRINT(F("*  - CALIBRATIONFACTOR : ")); DDECLN(anemometerCalibrationFactor);
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
      DPRINT(F("* ALTITUDE             : ")); DDECLN(this->getList0().height());
    }
};

SensChannelDevice sdev(devinfo, 0x20);
ConfigButton<SensChannelDevice> cfgBtn(sdev);

void setup () {
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.initDone();
  pinMode(RAINCOUNTER_PIN, INPUT_PULLUP);
  pinMode(WINDCOUNTER_PIN, INPUT_PULLUP);
  pinMode(WINDDIRECTION_PIN, INPUT_PULLUP);

  if ( digitalPinToInterrupt(RAINCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(RAINCOUNTER_PIN, raincounterISR, RISING); else attachInterrupt(digitalPinToInterrupt(RAINCOUNTER_PIN), raincounterISR, RISING);
  if ( digitalPinToInterrupt(WINDCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(WINDCOUNTER_PIN, windcounterISR, RISING); else attachInterrupt(digitalPinToInterrupt(WINDCOUNTER_PIN), windcounterISR, RISING);
  if ( digitalPinToInterrupt(AS3935_IRQ_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(AS3935_IRQ_PIN, lightningISR, RISING); else attachInterrupt(digitalPinToInterrupt(AS3935_IRQ_PIN), lightningISR, RISING);
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    hal.activity.savePower<Idle<>>(hal);
  }
}

void raincounterISR() {
  _rain_isr_counter++;
}

void windcounterISR() {
  _wind_isr_counter++;
}

void lightningISR() {
  _lightning_isr_counter = 1;
}


