//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// some parts (BME280 measurement) from HB-UNI-Sensor1
// 2018-05-11 Tom Major (Creative Commons)
// 2018-05-21 jp112sdl (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------
// #define NDEBUG
// #define USE_OTA_BOOTLOADER

#define  EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>
#include <MultiChannelDevice.h>
#include <sensors/Bh1750.h>

#define LED_PIN             4
#define WINDCOUNTER_PIN     5
#define RAINCOUNTER_PIN     6
#define CONFIG_BUTTON_PIN   8
#define WINDDIRECTION_PIN   A2

#define SYSCLOCK_FACTOR     1.0
#define BRIGHTNESS_FACTOR   1.2

//                             N                      O                       S                         W
//entspricht Windrichtung in ° 0 , 22.5, 45  , 67.5, 90  ,112.5, 135, 157.5, 180 , 202.5, 225 , 247.5, 270 , 292.5, 315 , 337.5
const uint16_t WINDDIRS[] = { 523  , 570 ,  474 , 746 , 624  , 806 , 370, 407, 999 , 228 ,  215 , 773 , 279 , 304, 290  , 880 };
//(kleinste Werteabweichung / 2) - 1
#define WINDDIR_TOLERANCE   5

#define PEERS_PER_CHANNEL   6

#include "Sensors/Sens_Bme280.h"

using namespace as;

volatile uint32_t _raincounter = 0;
volatile uint16_t _windcounter = 0;


const struct DeviceInfo PROGMEM devinfo = {
  {0xF1, 0xD0, 0x01},        // Device ID
  "JPWEA00001",           	 // Device Serial
  {0xF1, 0xD0},            	 // Device Model
  0x10,                   	 // Firmware Version
  as::DeviceType::THSensor,  // Device Type
  {0x01, 0x01}             	 // Info Bytes
};

// Configure the used hardware
typedef AvrSPI<10, 11, 12, 13> RadioSPI;
typedef AskSin<StatusLed<LED_PIN>, NoBattery, Radio<RadioSPI, 2> > Hal;
Hal hal;

class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint16_t airPressure, uint8_t humidity, uint16_t brightness, uint16_t raincounter, uint16_t windspeed, uint8_t winddir, uint8_t winddirrange) {
      Message::init(0x15, msgcnt, 0x70, (msgcnt % 20 == 1) ? BIDI : BCAST, (temp >> 8) & 0x7f, temp & 0xff);
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
    uint8_t       winddir;
    uint8_t       idxoldwdir;
    uint8_t       winddirrange;
    bool          firstRun;

    Sens_Bme280  bme280;
    Bh1750<>     bh1750;

  public:
    WeatherChannel () : Channel(), Alarm(seconds2ticks(60)), firstRun(true) {}
    virtual ~WeatherChannel () {}

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      measure_windspeed();
      measure_winddirection();
      measure_thpb();
      measure_rain();

      msg.init(device().nextcount(), temperature, airPressure, humidity, brightness, raincounter, windspeed, winddir, winddirrange);
      device().sendPeerEvent(msg, *this);

      uint16_t updCycle = this->device().getList0().updIntervall();
      tick = seconds2ticks(updCycle * SYSCLOCK_FACTOR);
      clock.add(*this);
      firstRun = false;
    }

    void measure_rain() {
      DPRINT(F("RAINCOUNTER            : "));
      if (firstRun) {
        //manchmal wird bei Initialisierung die ISR ausgelöst, das setzen wir hier zurück
        _raincounter = 0;
        DPRINTLN(F("first run - nothing"));
      } else {
        raincounter = _raincounter;
        DDECLN(raincounter);
      }
    }

    void measure_windspeed() {
      windspeed = 0;
      float Umdrehungen = (_windcounter * 1.0) / (device().getList0().updIntervall() * SYSCLOCK_FACTOR);
      //V = 2 * R * Pi * N
      float kmph =  3.141593 * 2 * this->getList1().AnemometerRadius() * Umdrehungen * 3.6 * (float)(this->getList1().AnemometerCalibrationFactor() / 10.0);
      windspeed = kmph;
      DPRINT(F("WINDSPEED _windcounter : ")); DDECLN(_windcounter);
      DPRINT(F("WINDSPEED windspeed    : ")); DDECLN(windspeed);
      _windcounter = 0;
    }

    void measure_winddirection() {
      //Windrichtung Grad/3: 60° = 20; 0° = Norden
      winddir = 0;
      uint8_t idxwdir = 0;
      uint16_t aVal = 0;
      for (uint8_t i = 0; i <= 0xf; i++) {
        aVal += analogRead(WINDDIRECTION_PIN);
      }
      aVal = aVal >> 4;

      for (uint8_t i = 0; i < sizeof(WINDDIRS) / sizeof(uint16_t); i++) {
        if (aVal < WINDDIRS[i] + WINDDIR_TOLERANCE && aVal > WINDDIRS[i] - WINDDIR_TOLERANCE) {
          winddir = i * 7.5;
          idxwdir = i;
          break;
        }
      }

      //Schwankungsbreite
      winddirrange = 3; // 0  - 3 (0, 22,5, 45, 67,5°)
      int idxdiff = abs(idxwdir - idxoldwdir);

      if (idxdiff <= 3) winddirrange = idxdiff;
      if (idxwdir <= 2 && idxoldwdir >= 13) winddirrange = (sizeof(WINDDIRS) / sizeof(uint16_t)) - idxdiff;
      if (winddirrange > 3) winddirrange = 3;

      idxoldwdir = idxwdir;

      DPRINT(F("WINDDIR aVal           : ")); DDEC(aVal); DPRINT(F(" i = ")); DDECLN(idxwdir);
      DPRINT(F("WINDDIR winddir/3      : ")); DDECLN(winddir);
      DPRINT(F("WINDDIR winddirrange   : ")); DDECLN(winddirrange);
    }

    // here we do the measurement
    void measure_thpb() {
      uint16_t height = this->device().getList0().height();
      bme280.measure(height);
      temperature = bme280.temperature();
      airPressure = bme280.pressureNN();
      humidity    = bme280.humidity();

      bh1750.measure();
      brightness = bh1750.brightness() * BRIGHTNESS_FACTOR;
      DPRINT(F("BRIGHTNESS             : ")  ); DDECLN(brightness);
    }

    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      tick = seconds2ticks(3);	        // first message in 3 sec.
      bh1750.init();
      bme280.init();

      sysclock.add(*this);
    }

    void configChanged() {
      DPRINTLN("* Config changed       : List1");
      DPRINTLN(F("* ANEMOMETER           : "));
      DPRINT(F("*  - RADIUS            : ")); DDECLN(this->getList1().AnemometerRadius());
      DPRINT(F("*  - CALIBRATIONFACTOR : ")); DDECLN(this->getList1().AnemometerCalibrationFactor());
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
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    hal.activity.savePower<Idle<>>(hal);
  }
}

void raincounterISR() {
  _raincounter++;
}

void windcounterISR() {
  _windcounter++;
}

