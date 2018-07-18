/* Copyright © 2014 Playing With Fusion, Inc.
  SOFTWARE LICENSE AGREEMENT: This code is released under the MIT License.

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
* **************************************************************************
  REVISION HISTORY:
  Author    Date    Comments
  J. Steinlage  2014Aug20 Original version
  J. Steinlage  2016Jul05   Fixed data write issue - now writing 'NewRegData'
                based on feedback from J Shuhy and A Jahnke

  jp112sdl 2018Jun21 modified for the use with AskSinPP (using SPI transactions)

*/
//#define  NDEBUG
#include <AskSinPP.h>
#include "PWFusion_AS3935.h"
#define _SPI_CLOCK_DIVIDER  SPI_CLOCK_DIV8
#define _SPI_DATA_MODE      SPI_MODE1
#define _SPI_BIT_ORDER      MSBFIRST

PWF_AS3935::PWF_AS3935(uint8_t CSx, uint8_t IRQx)
{
  _cs  = CSx;
  _irq = IRQx;

  // initalize the chip select pins
  pinMode(_cs, OUTPUT);
  pinMode(_irq, INPUT);

  digitalWrite(_cs, HIGH);	// set pin high to initialize/clear SPI bus
}

uint8_t PWF_AS3935::_sing_reg_read(uint8_t RegAdd)
{
  digitalWrite(_cs, LOW);						// set pin low to start talking to IC
  // next pack command byte, send AS3935.... structure is shown below
  //  MODE   Register Address/Direct Cmd
  // B15-14    B13------------------8
  // 15 is always 0; B14, 0: Write/Direct Command, 1: Read
  SPI.beginTransaction(SPISettings(_SPI_CLOCK_DIVIDER, _SPI_BIT_ORDER, _SPI_DATA_MODE));
  SPI.transfer((RegAdd & 0x3F) | 0x40);		// simple write, nothing to read back
  SPI.endTransaction();
  uint8_t RegData = SPI.transfer(0x00); 		// read register data from IC
  digitalWrite(_cs, HIGH);					// set pin high to end SPI session

  return RegData;
}

void PWF_AS3935::_sing_reg_write(uint8_t RegAdd, uint8_t DataMask, uint8_t RegData)
{
  // start by reading original register data (only modifying what we need to)
  uint8_t OrigRegData = _sing_reg_read(RegAdd);

  // calculate new register data... 'delete' old targeted data, replace with new data
  // note: 'DataMask' must be bits targeted for replacement
  // add'l note: this function does NOT shift values into the proper place... they need to be there already
  uint8_t NewRegData = ((OrigRegData & ~DataMask) | (RegData & DataMask));

  // now configure and write the updated register value
  digitalWrite(_cs, LOW);							// set pin low to start talking to IC
  // next pack command byte, send AS3935.... structure is shown below
  //  MODE   Register Address/Direct Cmd
  // B15-14    B13------------------8
  // 15 is always 0; B14, 0: Write/Direct Command, 1: Read
  SPI.beginTransaction(SPISettings(_SPI_CLOCK_DIVIDER, _SPI_BIT_ORDER, _SPI_DATA_MODE));
  SPI.transfer((RegAdd & 0x3F));					// simple write, nothing to read back
  SPI.transfer(NewRegData); 						// write register data to IC
  SPI.endTransaction();
  digitalWrite(_cs, HIGH);						// set pin high to end SPI session
}


void PWF_AS3935::AS3935_Reset()
{
  digitalWrite(_cs, LOW);        // begin transfer
  // run PRESET_DEFAULT Direct Command to set all registers in default state
  SPI.beginTransaction(SPISettings(_SPI_CLOCK_DIVIDER, _SPI_BIT_ORDER, _SPI_DATA_MODE));
  SPI.transfer(0x3C);			// send first byte (write, PRESET_DEFAULT register)
  SPI.transfer(0x96);			// send second byte (direct command)
  SPI.endTransaction();
  digitalWrite(_cs, HIGH);    // end transfer
  delay(2);					// wait 2ms to complete
}

void PWF_AS3935::_CalRCO()
{
  // run ALIB_RCO Direct Command to cal internal RCO
  digitalWrite(_cs, LOW);		// begin transfer
  SPI.beginTransaction(SPISettings(_SPI_CLOCK_DIVIDER, _SPI_BIT_ORDER, _SPI_DATA_MODE));
  SPI.transfer(0x3D);			// send first byte (write, CALIB_RCO register)
  SPI.transfer(0x96);			// send second byte (direct command)
  SPI.endTransaction();
  digitalWrite(_cs, HIGH);	// end transfer
  delay(3);					// wait 3ms to complete
}

void PWF_AS3935::AS3935_PowerUp(void)
{
  // power-up sequence based on datasheet, pg 23/27
  // register 0x00, PWD bit: 0 (clears PWD)
  _sing_reg_write(0x00, 0x01, 0x00);
  _CalRCO();							// run RCO cal cmd
  _sing_reg_write(0x08, 0x20, 0x20);	// set DISP_SRCO to 1
  delay(2);
  _sing_reg_write(0x08, 0x20, 0x00);	// set DISP_SRCO to 0

}

void PWF_AS3935::AS3935_DisturberEn(void)
{
  // register 0x03, PWD bit: 5 (sets MASK_DIST)
  _sing_reg_write(0x03, 0x20, 0x00);
  DPRINTLN("LD DIST DETECT EN");
}

void PWF_AS3935::AS3935_DisturberDis(void)
{
  // register 0x03, PWD bit: 5 (sets MASK_DIST)
  _sing_reg_write(0x03, 0x20, 0x20);
  DPRINTLN("LD: DIST DETECT DIS");
}

void PWF_AS3935::AS3935_SetIRQ_Output_Source(uint8_t irq_select)
{
  // set interrupt source - what to displlay on IRQ pin
  // reg 0x08, bits 5 (TRCO), 6 (SRCO), 7 (LCO)
  // only one should be set at once, I think
  // 0 = NONE, 1 = TRCO, 2 = SRCO, 3 = LCO

  if (1 == irq_select)
  {
    _sing_reg_write(0x08, 0xE0, 0x20);			// set only TRCO bit
  }
  else if (2 == irq_select)
  {
    _sing_reg_write(0x08, 0xE0, 0x40);			// set only SRCO bit
  }
  else if (3 == irq_select)
  {
    _sing_reg_write(0x08, 0xE0, 0x80);			// set only LCO bit
  }
  else // assume 0
  {
    _sing_reg_write(0x08, 0xE0, 0x00);			// clear IRQ pin display bits
  }

}

void PWF_AS3935::AS3935_SetTuningCaps(uint8_t cap_val)
{
  // Assume only numbers divisible by 8 (because that's all the chip supports)
  if (120 < cap_val)	// cap_value out of range, assume highest capacitance
  {
    _sing_reg_write(0x08, 0x0F, 0x0F);			// set capacitance bits to maximum
  }
  else
  {
    _sing_reg_write(0x08, 0x0F, (cap_val >> 3));	// set capacitance bits
  }
  //DPRINT("LD CAP SET TO 0x");
  //DPRINTLN((_sing_reg_read(0x08) & 0x0F));
}

uint8_t PWF_AS3935::AS3935_GetInterruptSrc(void)
{
  // definition of interrupt data on table 18 of datasheet
  // for this function:
  // 0 = unknown src, 1 = lightning detected, 2 = disturber, 3 = Noise level too high
  delay(10);						// wait 10ms before reading (2ms per pg 22 of datasheet)
  uint8_t int_src = (_sing_reg_read(0x03) & 0x0F);	// read register, get rid of non-interrupt data
  if (0x08 == int_src)
  {
    return 1;					// lightning caused interrupt
  }
  else if (0x04 == int_src)
  {
    return 2;					// disturber detected
  }
  else if (0x01 == int_src)
  {
    return 3;					// Noise level too high
  }
  else {
    return 0; // interrupt result not expected
  }

}

uint8_t PWF_AS3935::AS3935_GetLightningDistKm(void)
{
  uint8_t strike_dist = ((_sing_reg_read(0x07)) & 0x3F);	// read register, get rid of non-distance data
  return strike_dist;
}

uint32_t PWF_AS3935::AS3935_GetStrikeEnergyRaw(void)
{
  uint32_t nrgy_raw = ((_sing_reg_read(0x06) & 0x1F) << 8);	// MMSB, shift 8  bits left, make room for MSB
  nrgy_raw |= _sing_reg_read(0x05);							// read MSB
  nrgy_raw <<= 8;												// shift 8 bits left, make room for LSB
  nrgy_raw |= _sing_reg_read(0x04);							// read LSB, add to others

  return nrgy_raw;
}

uint8_t PWF_AS3935::AS3935_SetMinStrikes(uint8_t min_strk)
{
  // This function sets min strikes to the closest available number, rounding to the floor,
  // where necessary, then returns the physical value that was set. Options are 1, 5, 9 or 16 strikes.
  // see pg 22 of the datasheet for more info (#strikes in 17 min)
  if (5 > min_strk)
  {
    _sing_reg_write(0x02, 0x30, 0x00);
    return 1;
  }
  else if (9 > min_strk)
  {
    _sing_reg_write(0x02, 0x30, 0x10);
    return 5;
  }
  else if (16 > min_strk)
  {
    _sing_reg_write(0x02, 0x30, 0x20);
    return 9;
  }
  else
  {
    _sing_reg_write(0x02, 0x30, 0x30);
    return 16;
  }
}

void PWF_AS3935::AS3935_SetIndoors(void)
{
  // AFE settings addres 0x00, bits 5:1 (10010, based on datasheet, pg 19, table 15)
  // this is the default setting at power-up (AS3935 datasheet, table 9)
  _sing_reg_write(0x00, 0x3E, 0x24);
}

void PWF_AS3935::AS3935_SetOutdoors(void)
{
  // AFE settings addres 0x00, bits 5:1 (01110, based on datasheet, pg 19, table 15)
  _sing_reg_write(0x00, 0x3E, 0x1C);
}

void PWF_AS3935::AS3935_ClearStatistics(void)
{
  // clear is accomplished by toggling CL_STAT bit 'high-low-high' (then set low to move on)
  _sing_reg_write(0x02, 0x40, 0x40);			// high
  _sing_reg_write(0x02, 0x40, 0x00);			// low
  _sing_reg_write(0x02, 0x40, 0x40);			// high
}

uint8_t PWF_AS3935::AS3935_GetNoiseFloorLvl(void)
{
  // NF settings addres 0x01, bits 6:4
  // default setting of 010 at startup (datasheet, table 9)
  uint8_t reg_raw = _sing_reg_read(0x01);		// read register 0x01
  return ((reg_raw & 0x70) >> 4);				// should return value from 0-7, see table 16 for info
}

void PWF_AS3935::AS3935_SetNoiseFloorLvl(uint8_t nf_sel)
{
  // NF settings addres 0x01, bits 6:4
  // default setting of 010 at startup (datasheet, table 9)
  if (7 >= nf_sel)								// nf_sel within expected range
  {
    _sing_reg_write(0x01, 0x70, ((nf_sel & 0x07) << 4));
  }
  else
  { // out of range, set to default (power-up value 010)
    _sing_reg_write(0x01, 0x70, 0x20);
  }
}

uint8_t PWF_AS3935::AS3935_GetWatchdogThreshold(void)
{
  // This function is used to read WDTH. It is used to increase robustness to disturbers,
  // though will make detection less efficient (see page 19, Fig 20 of datasheet)
  // WDTH register: add 0x01, bits 3:0
  // default value of 0001
  // values should only be between 0x00 and 0x0F (0 and 7)
  uint8_t reg_raw = _sing_reg_read(0x01);
  return (reg_raw & 0x0F);
}

void PWF_AS3935::AS3935_SetWatchdogThreshold(uint8_t wdth)
{
  // This function is used to modify WDTH. It is used to increase robustness to disturbers,
  // though will make detection less efficient (see page 19, Fig 20 of datasheet)
  // WDTH register: add 0x01, bits 3:0
  // default value of 0001
  // values should only be between 0x00 and 0x0F (0 and 7)
  _sing_reg_write(0x01, 0x0F, (wdth & 0x0F));
}

uint8_t PWF_AS3935::AS3935_GetSpikeRejection(void)
{
  // This function is used to read SREJ (spike rejection). Similar to the Watchdog threshold,
  // it is used to make the system more robust to disturbers, though will make general detection
  // less efficient (see page 20-21, especially Fig 21 of datasheet)
  // SREJ register: add 0x02, bits 3:0
  // default value of 0010
  // values should only be between 0x00 and 0x0F (0 and 7)
  uint8_t reg_raw = _sing_reg_read(0x02);
  return (reg_raw & 0x0F);
}

void PWF_AS3935::AS3935_SetSpikeRejection(uint8_t srej)
{
  // This function is used to modify SREJ (spike rejection). Similar to the Watchdog threshold,
  // it is used to make the system more robust to disturbers, though will make general detection
  // less efficient (see page 20-21, especially Fig 21 of datasheet)
  // WDTH register: add 0x02, bits 3:0
  // default value of 0010
  // values should only be between 0x00 and 0x0F (0 and 7)
  _sing_reg_write(0x02, 0x0F, (srej & 0x0F));
}

void PWF_AS3935::AS3935_SetLCO_FDIV(uint8_t fdiv)
{
  // This function sets LCO_FDIV register. This is useful in the tuning of the antenna
  // LCO_FDIV register: add 0x03, bits 7:6
  // default value: 00
  // set 0, 1, 2 or 3 for ratios of 16, 32, 64 and 128, respectively.
  // See pg 23, Table 20 for more info.
  _sing_reg_write(0x03, 0xC0, ((fdiv & 0x03) << 5));
}

void PWF_AS3935::AS3935_PrintAllRegs(void)
{
  DPRINT("LD REGS [ ");
  DPRINT(_sing_reg_read(0x00));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x01));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x02));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x03));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x04));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x05));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x06));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x07));
  DPRINT(" | ");
  DPRINT(_sing_reg_read(0x08));
  DPRINTLN(" ]");
}

void PWF_AS3935::AS3935_ManualCal(uint8_t capacitance, uint8_t disturber, uint8_t environment)
{
  AS3935_SetIRQ_Output_Source(0);

  delay(2);

  AS3935_SetTuningCaps(capacitance);
  delay(3);
  
  AS3935_PowerUp();
  
  if (0 == environment) {
    AS3935_SetOutdoors();
  } else {
    AS3935_SetIndoors();
  }

  // disturber cal
  if (0 == disturber) {
    AS3935_DisturberDis();
  } else {
    AS3935_DisturberEn();
  }

  DPRINTLN("LD CAL COMPLETE");
}
