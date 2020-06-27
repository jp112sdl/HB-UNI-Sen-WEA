#ifndef VentusW132_h
#define VentusW132_h

#define  N_MASK 0b00001000
#define NO_MASK 0b00001100
#define  O_MASK 0b00000100
#define SO_MASK 0b00000110
#define  S_MASK 0b00000010
#define SW_MASK 0b00000011
#define  W_MASK 0b00000001
#define NW_MASK 0b00001001

namespace as {

template <uint8_t N_PIN, uint8_t O_PIN, uint8_t S_PIN, uint8_t W_PIN>
class VentusW132 {
  private:
    uint8_t pins[4];
  public:
    VentusW132 () {}

    void init () {
      pins[0] = N_PIN;
      pins[1] = O_PIN;
      pins[2] = S_PIN;
      pins[3] = W_PIN;
      
      for (uint8_t i = 0; i < 4; i++) 
        pinMode(pins[i], INPUT);

    }

    uint8_t readState() {
      uint8_t state = 0;
      for (uint8_t i = 0; i < 4; i++) {
        bool p = (pins[i] == A6 || pins[i] == A7) ? (analogRead(pins[i]) > 500) : digitalRead(pins[i]);
        state |= p << i;
        //DPRINT(F("read pin("));DDEC(pins[i]);DPRINT(") = ");DDECLN(p);
      }
      return state;
    }

    uint8_t winddirValue(bool asIndex = false) {
      uint8_t state = readState();
      //DPRINT(F("state="));DHEXLN(state);

      if (state ==  N_MASK) return asIndex ? 0 : 0;
      if (state == NO_MASK) return asIndex ? 1 : 15;
      if (state ==  O_MASK) return asIndex ? 2 : 30;
      if (state == SO_MASK) return asIndex ? 3 : 45;
      if (state ==  S_MASK) return asIndex ? 4 : 60;
      if (state == SW_MASK) return asIndex ? 5 : 75;
      if (state ==  W_MASK) return asIndex ? 6 : 90;
      if (state == NW_MASK) return asIndex ? 7 : 105;
      return 0;
    }    
};

}
#endif
