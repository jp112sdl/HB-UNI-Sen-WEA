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
const uint16_t WINDDIRS[] = { 523  , 570 ,  474 , 746 , 624  , 806 , 370, 407, 999 , 228 ,  215 , 773 , 279 , 304, 290  , 880 };
#define WINDDIR_TOLERANCE   5
#define WINDDIRECTION_PIN   A2


void setup() {
  Serial.begin(57600);
  pinMode(A2, INPUT_PULLUP);
  // put your setup code here, to run once:

}

void loop() {
  delay(100);
  // put your main code here, to run repeatedly:

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

}
