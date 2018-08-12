#define RAINDETECTOR_CRG_PIN 4
#define RAINDETECTOR_DISCRG_PIN A3
void setup() {
  Serial.begin(57600);
  pinMode(RAINDETECTOR_CRG_PIN, OUTPUT);
  pinMode(RAINDETECTOR_DISCRG_PIN, INPUT);
}

void loop() {

  digitalWrite(RAINDETECTOR_CRG_PIN, HIGH);
  _delay_ms(2);
  digitalWrite(RAINDETECTOR_CRG_PIN, LOW);
  int rdVal = analogRead(RAINDETECTOR_DISCRG_PIN);
  Serial.println("a = " + String(rdVal));
  delay(1000);
}
