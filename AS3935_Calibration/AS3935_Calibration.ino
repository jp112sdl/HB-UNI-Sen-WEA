#include <SPI.h>
#define IRQpin 3
#define CSpin  7

#define AS3935_AFE_GB    0x00, 0x3E
#define AS3935_PWD    0x00, 0x01
#define AS3935_NF_LEV   0x01, 0x70
#define AS3935_WDTH   0x01, 0x0F
#define AS3935_CL_STAT    0x02, 0x40
#define AS3935_MIN_NUM_LIGH 0x02, 0x30
#define AS3935_SREJ   0x02, 0x0F
#define AS3935_LCO_FDIV 0x03, 0xC0
#define AS3935_MASK_DIST  0x03, 0x20
#define AS3935_INT    0x03, 0x0F
#define AS3935_DISTANCE 0x07, 0x3F
#define AS3935_DISP_LCO 0x08, 0x80
#define AS3935_DISP_SRCO  0x08, 0x40
#define AS3935_DISP_TRCO  0x08, 0x20
#define AS3935_TUN_CAP    0x08, 0x0F
#define AS3935_ENERGY_1   0x04, 0xFF
#define AS3935_ENERGY_2   0x05, 0xFF
#define AS3935_ENERGY_3   0x06, 0x1F

// other constants
#define AS3935_AFE_INDOOR 0x12
#define AS3935_AFE_OUTDOOR  0x0E

byte SPITransfer2(byte high, byte low)
{
  digitalWrite(CSpin, LOW);
  SPI.transfer(high);
  byte regval = SPI.transfer(low);
  digitalWrite(CSpin, HIGH);
  return regval;
}

byte _rawRegisterRead(byte reg)
{
  return SPITransfer2((reg & 0x3F) | 0x40, 0);
}

byte _ffsz(byte mask)
{
  byte i = 0;
  if (mask)
    for (i = 1; ~mask & 1; i++)
      mask >>= 1;
  return i;
}

void registerWrite(byte reg, byte mask, byte data)
{
  byte regval = _rawRegisterRead(reg);
  regval &= ~(mask);
  if (mask)
    regval |= (data << (_ffsz(mask) - 1));
  else
    regval |= data;
  SPITransfer2(reg & 0x3F, regval);
}

byte registerRead(byte reg, byte mask)
{
  byte regval = _rawRegisterRead(reg);
  regval = regval & mask;
  if (mask)
    regval >>= (_ffsz(mask) - 1);
  return regval;
}

void powerUp()
{
  registerWrite(AS3935_PWD, 0);
  SPITransfer2(0x3D, 0x96);
  delay(3);

  // Modify REG0x08[5] = 1
  registerWrite(AS3935_DISP_TRCO, 1);
  delay(2);
  // Modify REG0x08[5] = 0
  registerWrite(AS3935_DISP_TRCO, 0);
}

void Areset()
{
  SPITransfer2(0x3C, 0x96);
  delay(2);
  powerUp();
  delay(2);
}

int tuneAntenna(byte tuneCapacitor)
{
  unsigned long setUpTime;
  int currentcount = 0;
  int currIrq, prevIrq;

  registerWrite(AS3935_LCO_FDIV, 0);
  registerWrite(AS3935_DISP_LCO, 1);

  registerWrite(AS3935_TUN_CAP, tuneCapacitor);


  delay(2);
  prevIrq = digitalRead(IRQpin);
  setUpTime = millis() + 100;
  while ((long)(millis() - setUpTime) < 0)
  {
    currIrq = digitalRead(IRQpin);

    if (currIrq > prevIrq)
    {
      currentcount++;
    }
    prevIrq = currIrq;
  }

  registerWrite(AS3935_TUN_CAP, tuneCapacitor);
  delay(2);
  registerWrite(AS3935_DISP_LCO, 0);
  powerUp();

  return currentcount;
}
void setup() {
  digitalWrite(CSpin, HIGH);
  pinMode(CSpin, OUTPUT);
  pinMode(IRQpin, INPUT);
  Serial.begin(57600);
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  Serial.println("GO");
  Areset();

  delay(50);
  Serial.println();
  for (byte i = 0; i <= 0x0F; i++) {
    int frequency = tuneAntenna(i);
    Serial.print("tune antenna to capacitor ");
    Serial.print(i);
    Serial.print("\t gives frequency: ");
    Serial.print(frequency);
    Serial.print(" = ");
    long fullFreq = (long) frequency * 160; // multiply with clock-divider, and 10 (because measurement is for 100ms)
    Serial.print(fullFreq, DEC);
    Serial.println(" Hz");
    delay(100);
  }

}

void loop() { }
