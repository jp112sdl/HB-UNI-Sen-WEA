#define RAINDETECTOR_CRG_PIN       4  // D4
#define RAINDETECTOR_DISCRG_PIN   A3  // A3
#define RAINDETECTOR_HEATING_PIN   9  // D9

void setup() {
  Serial.begin(57600);
  Serial.println("Raindetector Test");
  digitalWrite(RAINDETECTOR_HEATING_PIN, LOW);
  pinMode(RAINDETECTOR_HEATING_PIN, OUTPUT);
  pinMode(RAINDETECTOR_CRG_PIN, OUTPUT);
  pinMode(RAINDETECTOR_DISCRG_PIN, INPUT);
}

void loop() {
  static unsigned int count = 0;

  if ((++count % 10) == 0) {
    /* toggle heating every 10 cycles */
    if (digitalRead(RAINDETECTOR_HEATING_PIN) == LOW) {
      digitalWrite(RAINDETECTOR_HEATING_PIN, HIGH);
      Serial.println("Heating switched on! " + String(count));
    } else {
      digitalWrite(RAINDETECTOR_HEATING_PIN, LOW);
      Serial.println("Heating switched off! " + String(count));
    }
  }
  
  digitalWrite(RAINDETECTOR_CRG_PIN, HIGH);
  _delay_ms(2);
  digitalWrite(RAINDETECTOR_CRG_PIN, LOW);
  int rdVal = analogRead(RAINDETECTOR_DISCRG_PIN);
  Serial.println("Raindetector: " + String(rdVal));
  delay(1000);
}
