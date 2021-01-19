#ifndef PCF8574_WindDir_h
#define PCF8574_WindDir_h

//https://github.com/xreef/PCF8574_library
#include <PCF8574.h>

namespace as {

template <uint8_t ADDRESS=0x38>
class PCF8574_WindDir {
  private:
  PCF8574 pcf8574;
  public:
    PCF8574_WindDir () : pcf8574(ADDRESS) {}

    void init () {
      for (uint8_t i = 0; i < 8; i++) 
        pcf8574.pinMode(i, INPUT);
        
      pcf8574.begin();  

    }

    uint8_t winddirValue(bool asIndex = false) {
      PCF8574::DigitalInput state = pcf8574.digitalReadAll();
      //DPRINT(F("state="));DHEXLN(state);

      if (state.p0) return asIndex ? 0 : 0;
      if (state.p1) return asIndex ? 1 : 15;
      if (state.p2) return asIndex ? 2 : 30;
      if (state.p3) return asIndex ? 3 : 45;
      if (state.p4) return asIndex ? 4 : 60;
      if (state.p5) return asIndex ? 5 : 75;
      if (state.p6) return asIndex ? 6 : 90;
      if (state.p7) return asIndex ? 7 : 105;
      return 0;
    }    
};

}
#endif
