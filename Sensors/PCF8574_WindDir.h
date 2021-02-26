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
      if (pcf8574.digitalRead(P0) == 0) return asIndex ? 0 : 0;
      if (pcf8574.digitalRead(P1) == 0) return asIndex ? 1 : 15;
      if (pcf8574.digitalRead(P2) == 0) return asIndex ? 2 : 30;
      if (pcf8574.digitalRead(P3) == 0) return asIndex ? 3 : 45;
      if (pcf8574.digitalRead(P4) == 0) return asIndex ? 4 : 60;
      if (pcf8574.digitalRead(P5) == 0) return asIndex ? 5 : 75;
      if (pcf8574.digitalRead(P6) == 0) return asIndex ? 6 : 90;
      if (pcf8574.digitalRead(P7) == 0) return asIndex ? 7 : 105;
      return 0;
    }   
    
};

}
#endif
