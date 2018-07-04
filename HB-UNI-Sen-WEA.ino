//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// some parts (BME280 measurement) from HB-UNI-Sensor1
// 2018-05-11 Tom Major (Creative Commons)
// 2018-05-21 jp112sdl (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------
// #define USE_OTA_BOOTLOADER
// #define NDEBUG
// #define NSENSORS // if defined, only fake values are used

#define  EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <SPI.h>  // after including SPI Library - we can use LibSPI class
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>

#include <MultiChannelDevice.h>
#include <sensors/Bh1750.h>
//#include <sensors/Max44009.h>
#include "Sensors/Sens_Bme280.h"
#include "Sensors/Sens_Max44009.h"
#include "Sensors/Sens_VEML6070.h"
#include "Sensors/Sens_As3935.h"

#define LED_PIN             4
#define WINDCOUNTER_PIN     5
#define RAINCOUNTER_PIN     6
#define CONFIG_BUTTON_PIN   8
#define WINDDIRECTION_PIN   A2

#define USE_MAX44009
//#define USE_BH1750
#define BH1750_BRIGHTNESS_FACTOR   1.2 //you have to multiply BH1750 raw value by 1.2

//                             N                      O                       S                         W
//entspricht Windrichtung in ° 0 , 22.5, 45  , 67.5, 90  ,112.5, 135, 157.5, 180 , 202.5, 225 , 247.5, 270 , 292.5, 315 , 337.5
const uint16_t WINDDIRS[] = { 806 , 371, 407, 999 , 228 ,  215 , 773 , 279,  304, 290  , 880, 523  , 570 ,  474 , 746 , 624 };
//(kleinste Werteabweichung / 2) - 1
#define WINDDIR_TOLERANCE   5
#define WINDSPEED_MEASUREINTERVAL_SECONDS 5

#define PEERS_PER_CHANNEL   2

using namespace as;

volatile uint32_t _rain_isr_counter = 0;
volatile uint16_t _wind_isr_counter = 0;

const struct DeviceInfo PROGMEM devinfo = {
  {0xF1, 0xD0, 0x02},        // Device ID
  "JPWEA00002",           	 // Device Serial
  {0xF1, 0xD0},            	 // Device Model
  0x10,                   	 // Firmware Version
  as::DeviceType::THSensor,  // Device Type
  {0x01, 0x01}             	 // Info Bytes
};

// Configure the used hardware
typedef AskSin<NoLed, NoBattery, Radio<LibSPI<10>, 2>> Hal;
//typedef AskSin<StatusLed<LED_PIN>, NoBattery, Radio<LibSPI<10>, 2>> Hal;

Hal hal;

class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint16_t airPressure, uint8_t humidity, uint32_t brightness, uint16_t raincounter, uint16_t windspeed, uint8_t winddir, uint8_t winddirrange, uint16_t gustspeed, uint8_t uvindex, uint8_t lightningcounter, uint8_t lightningdistance) {
      Message::init(0x1a, msgcnt, 0x70, BCAST, (temp >> 8) & 0x7f, temp & 0xff);
      pload[0] = (airPressure >> 8) & 0xff;
      pload[1] = airPressure & 0xff;
      pload[2] = humidity;
      pload[3] = (brightness >>  16) & 0xff;
      pload[4] = (brightness >>  8) & 0xff;
      pload[5] = brightness & 0xff;
      pload[6] = (raincounter >> 8) & 0xff;
      pload[7] = raincounter & 0xff;
      pload[8] = (windspeed >> 8) & 0xff | (winddirrange << 6);
      pload[9] = windspeed & 0xff;
      pload[10] = winddir;
      pload[11] = (gustspeed >> 8) & 0xff;
      pload[12] = gustspeed & 0xff;
      pload[13] = (uvindex & 0xff) | (lightningdistance << 4);
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

DEFREGISTER(UReg1, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06)
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

    void defaults () {
      clear();
      AnemometerRadius(65);
      AnemometerCalibrationFactor(10);
      LightningDetectorCapacitor(80);
      LightningDetectorDisturberDetection(true);
      ExtraMessageOnGustThreshold(0);
    }
};

class WeatherChannel : public Channel<Hal, SensorList1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {

    WeatherEventMsg msg;
    int16_t       temperature;
    uint16_t      airPressure;
    uint8_t       humidity;
    uint32_t      brightness;
    uint16_t      raincounter;
    uint16_t      windspeed;
    uint16_t      gustspeed;
    uint8_t       uvindex;
    uint8_t       lightningcounter;
    uint8_t       lightningdistance;

    uint8_t       winddir;
    uint8_t       idxoldwdir;
    uint8_t       winddirrange;
    bool          initComplete;
    uint8_t       short_interval_measure_count;

    uint8_t       anemometerRadius;
    uint8_t       anemometerCalibrationFactor;
    uint8_t       extraMessageOnGustThreshold;

    Sens_Bme280                 bme280;
    Sens_Veml6070<VEML6070_1_T> veml6070;
#ifdef USE_BH1750
    Bh1750<>                    bh1750;
#endif
#ifdef USE_MAX44009
    MAX44009<>                    max44009;
#endif
  public:
    Sens_As3935<> as3935;

  public:
    WeatherChannel () : Channel(), Alarm(seconds2ticks(60)), initComplete(false), windspeed(0), uvindex(0), short_interval_measure_count(0), background_measure(*this), lightning_check(*this)  {}
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

    void processMessage() {
      measure_winddirection();
      measure_thpb();
      measure_rain();

      if (initComplete) {
        windspeed = windspeed / short_interval_measure_count;
        uvindex = uvindex / short_interval_measure_count;
      }

      DPRINT(F("GUSTSPEED gustspeed    : ")); DDECLN(gustspeed);
      DPRINT(F("WINDSPEED windspeed    : ")); DDECLN(windspeed);
      DPRINT(F("UV Index               : ")); DDECLN(uvindex);

      msg.init(device().nextcount(), temperature, airPressure, humidity, brightness, raincounter, windspeed, winddir, winddirrange, gustspeed, uvindex, lightningcounter, lightningdistance);
      device().sendPeerEvent(msg, *this);

      uint16_t updCycle = this->device().getList0().updIntervall();
      tick = seconds2ticks(updCycle);

      initComplete = true;
      windspeed = 0;
      gustspeed = 0;
      uvindex = 0;
      short_interval_measure_count = 0;
      sysclock.add(*this);
    }

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      processMessage();
    }

    void sendExtraMessageOnGustThreshold () {
      DPRINTLN("SENDING EXTRA MESSAGE");
      sysclock.cancel(*this);
      processMessage();
    }

    void measure_windspeed() {
#ifdef NSENSORS
      _wind_isr_counter = random(20);
#endif
      //V = 2 * R * Pi * N
      //  int kmph =  3.141593 * 2 * ((float)anemometerRadius / 100)   * ((float)_wind_isr_counter / (float)WINDSPEED_MEASUREINTERVAL_SECONDS)        * 3.6 * ((float)anemometerCalibrationFactor / 10);
      int kmph = ((226L * anemometerRadius * anemometerCalibrationFactor * _wind_isr_counter) / WINDSPEED_MEASUREINTERVAL_SECONDS) / 10000;
      if (extraMessageOnGustThreshold > 0 && kmph > (extraMessageOnGustThreshold * 10)) {
        sendExtraMessageOnGustThreshold();
      }
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
      if (!initComplete) {
        _rain_isr_counter = 0;
      } else {
        raincounter = _rain_isr_counter;
      }
      DDECLN(_rain_isr_counter);
    }

    void measure_lightning() {
#ifdef NSENSORS
      lightningcounter = random(255);
      lightningdistance = random(15);
#else
      uint8_t lightning_dist_km = 0;
      if (as3935.LightningIsrCounter() > 0) {
        switch (as3935.GetInterruptSrc()) {
          case 0:
            DPRINTLN("LD IRQ SRC NOT EXPECTED");
            break;
          case 1:
            lightning_dist_km = as3935.LightningDistKm();
            DPRINT("LD LIGHTNING IN ");
            DDEC(lightning_dist_km);
            DPRINTLN(" km");
            lightningcounter++;
            lightningdistance = lightning_dist_km;
            break;
          case 2:
            DPRINTLN("LD DIST DETECTED");
            break;
          case 3:
            DPRINTLN("LD NOISE HIGH");
            break;
        }
        as3935.ResetLightninIsrCounter();
      }
#endif
    }

    void measure_thpb() {
      uint16_t height = this->device().getList0().height();
#ifdef NSENSORS
      airPressure = 9000 + random(2000);   // 1024 hPa +x
      humidity    = 66 + random(7);     // 66% +x
      temperature = 150 + random(50);   // 15C +x
      brightness = 1700000 + random(10000);   // 67000 Lux +x
      DPRINT(F("        airPressure    : ")); DDECLN(airPressure);
      DPRINT(F("        humidity       : ")); DDECLN(humidity);
      DPRINT(F("        temperature    : ")); DDECLN(temperature);
      DPRINT(F("        brightness     : ")); DDECLN(brightness);
#else
      bme280.measure(height);
      temperature = bme280.temperature();
      airPressure = bme280.pressureNN();
      humidity    = bme280.humidity();

#ifdef USE_BH1750
      bh1750.measure();
      brightness = bh1750.brightness() * 10 * BH1750_BRIGHTNESS_FACTOR;
#endif
#ifdef USE_MAX44009
      max44009.measure();
      brightness = max44009.brightness();
#endif
      DPRINT(F("BRIGHTNESS             : ")  ); DDECLN(brightness);
#endif
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      tick = seconds2ticks(3);	// first message in 3 sec.
#ifndef NSENSORS
#ifdef USE_BH1750
      bh1750.init();
#endif
#ifdef USE_MAX44009
      max44009.init();
#endif
      bme280.init();
      veml6070.init();
#endif
      sysclock.add(*this);
      sysclock.add(background_measure);
      sysclock.add(lightning_check);
    }

    void configChanged() {
      anemometerRadius = this->getList1().AnemometerRadius();
      anemometerCalibrationFactor = this->getList1().AnemometerCalibrationFactor();
      extraMessageOnGustThreshold = this->getList1().ExtraMessageOnGustThreshold();
      DPRINTLN("* Config changed       : List1");
      DPRINTLN(F("* ANEMOMETER           : "));
      DPRINT(F("*  - RADIUS            : ")); DDECLN(anemometerRadius);
      DPRINT(F("*  - CALIBRATIONFACTOR : ")); DDECLN(anemometerCalibrationFactor);
      DPRINT(F("*  - GUST MSG THRESHOLD: ")); DDECLN(extraMessageOnGustThreshold);
      DPRINTLN(F("* LIGHTNINGDETECTOR    : "));
      DPRINT(F("*  - CAPACITOR         : ")); DDECLN(this->getList1().LightningDetectorCapacitor());
      DPRINT(F("*  - DISTURB.DETECTION : ")); DDECLN(this->getList1().LightningDetectorDisturberDetection());
#ifndef NSENSORS
      as3935.init(this->getList1().LightningDetectorCapacitor(), this->getList1().LightningDetectorDisturberDetection());
#endif
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
#ifdef NDEBUG
  Serial.begin(57600);
#else
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
#endif
  sdev.init(hal);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.initDone();

  pinMode(RAINCOUNTER_PIN, INPUT_PULLUP);
  pinMode(WINDCOUNTER_PIN, INPUT_PULLUP);
  pinMode(WINDDIRECTION_PIN, INPUT_PULLUP);

  if ( digitalPinToInterrupt(RAINCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(RAINCOUNTER_PIN, raincounterISR, RISING); else attachInterrupt(digitalPinToInterrupt(RAINCOUNTER_PIN), raincounterISR, RISING);
  if ( digitalPinToInterrupt(WINDCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(WINDCOUNTER_PIN, windcounterISR, RISING); else attachInterrupt(digitalPinToInterrupt(WINDCOUNTER_PIN), windcounterISR, RISING);
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



