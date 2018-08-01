#include <EnableInterrupt.h>
#define WINDCOUNTER_PIN 5

//Wind Speed
float radius = 0.065; //metres from center pin to middle of cup
int seconds = 0;
int revolutions = 0; //counted by interrupt
int rps = -1; // read revs per second (5 second interval)
int mps = 0;  //wind speed metre/sec



void setup() {
  Serial.begin(57600);
  pinMode(WINDCOUNTER_PIN, INPUT_PULLUP);
  if ( digitalPinToInterrupt(WINDCOUNTER_PIN) == NOT_AN_INTERRUPT ) enableInterrupt(WINDCOUNTER_PIN, rps_fan, FALLING); else attachInterrupt(digitalPinToInterrupt(WINDCOUNTER_PIN), rps_fan, FALLING);
  // Initialize Timer1 for a 1 second interrupt
  // Thanks to http://www.engblaze.com/ for this section, see their Interrupt Tutorial
  cli();          // disable global interrupts
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B
  // set compare match register to desired timer count:
  OCR1A = 15624;
  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  // enable global interrupts:
  sei();
} // setup()

//Routine Driven by Interrupt, trap 1 second interrupts, and output every 5 seconds
ISR(TIMER1_COMPA_vect) {
  seconds++;
  if (seconds == 5) { //make 5 for each output
    seconds = 0;
    rps = revolutions;
    revolutions = 0;
  }
}

// executed every time the interrupt 0 (pin2) gets low, ie one rev of cups.
void rps_fan() {
  revolutions++;
}//end of interrupt routine


void loop() {

  while (true) {
   
    if (rps != -1) { //Update every 5 seconds, this will be equal to reading frequency (Hz)x5.
      float kmph = rps * 3.1414 * 2 * radius * 12 * 60 / 1000;

      Serial.println("km/h = " + String(kmph));
      Serial.println("revs = "+String(revolutions));
    }

    delay(1000);
  }
} 

