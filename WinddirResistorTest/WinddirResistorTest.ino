/*
  S  = 533
  SW = 418
  W  = 234
  NW = 131
  N  = 310
  NO = 151
  O  = 579
  SO = 754
*/
#include <EnableInterrupt.h>

const uint16_t WINDDIRS[] = { 806 , 371, 407, 999 , 228 ,  215 , 773 , 279,  304, 290  , 880, 523  , 570 ,  474 , 746 , 624   };
#define WINDDIR_TOLERANCE   5
#define WINDDIRECTION_PIN   A2
#define WINDCOUNTER_PIN     5
volatile int _windcounter = 0;
int lastWindcounter = 0;

void setup() {
  Serial.begin(57600);
  pinMode(WINDDIRECTION_PIN, INPUT_PULLUP);
  // put your setup code here, to run once:
  if ( digitalPinToInterrupt(WINDCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(WINDCOUNTER_PIN, windcounterISR, RISING); else attachInterrupt(digitalPinToInterrupt(WINDCOUNTER_PIN), windcounterISR, RISING);

}

void loop() {
  delay(100);

   int winddir = 0;     // Grad/3: 60° = 20; 0° = Norden
    uint8_t idxwdir = 0;

    uint16_t aVal = 0;
    for (int i = 0; i < 10; i++) {
      aVal += analogRead(WINDDIRECTION_PIN);
      delay(5);
    }
    aVal = aVal / 10;
    for (int i = 0; i < sizeof(WINDDIRS) / sizeof(uint16_t); i++) {
      if (aVal < WINDDIRS[i] + WINDDIR_TOLERANCE && aVal > WINDDIRS[i] - WINDDIR_TOLERANCE) {
        winddir = i * 7.5;
        idxwdir = i;
        break;
      }
    }
    Serial.println("A2 = "+String(aVal)+"; idx = "+String(idxwdir));
    
  if (lastWindcounter != _windcounter) {
    lastWindcounter = _windcounter;
    Serial.println("windcounter = " + String(_windcounter));
  }

}

void windcounterISR() {
  _windcounter++;
}
