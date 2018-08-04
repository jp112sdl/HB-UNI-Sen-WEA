#include <EnableInterrupt.h>

const uint16_t WINDDIRS[] = {  200, 160, 412, 126, 148, 57, 72, 52, 113, 95, 320, 295, 785, 550, 655, 183   };
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
  for (uint8_t i = 0; i <= 0xf; i++) {
    aVal += analogRead(WINDDIRECTION_PIN);
  }
  aVal = aVal >> 4;

  uint8_t WINDDIR_TOLERANCE = 2;
  if ((aVal > 100) && (aVal < 250)) WINDDIR_TOLERANCE = 5;
  if (aVal >= 250) WINDDIR_TOLERANCE = 10;

  for (uint8_t i = 0; i < sizeof(WINDDIRS) / sizeof(uint16_t); i++) {
    if (aVal < WINDDIRS[i] + WINDDIR_TOLERANCE && aVal > WINDDIRS[i] - WINDDIR_TOLERANCE) {
      idxwdir = i;
      winddir = (idxwdir * 15 + 2 / 2) / 2;
      break;
    }
  }
  Serial.println("A2 = " + String(aVal) + "; idx = " + String(idxwdir) + "; Tolerance = " + String(WINDDIR_TOLERANCE));

  if (lastWindcounter != _windcounter) {
    lastWindcounter = _windcounter;
    Serial.println("windcounter = " + String(_windcounter));
  }

}

void windcounterISR() {
  _windcounter++;
}
