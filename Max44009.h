#ifndef MAX44009_H
#define MAX44009_H
//
//    FILE: Max44009.h
//  AUTHOR: Rob dot Tillaart at gmail dot com
// VERSION: 0.1.9
// PURPOSE: library for MAX44009 lux sensor Arduino
// HISTORY: See Max440099.cpp
//
// Released to the public domain
//

#include "Wire.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// REGISTERS
#define MAX44009_CONFIGURATION      0x02
#define MAX44009_LUX_READING_HIGH   0x03
#define MAX44009_LUX_READING_LOW    0x04

// CONFIGURATION MASKS
#define MAX44009_CFG_CONTINUOUS     0x80
#define MAX44009_CFG_MANUAL         0x40
#define MAX44009_CFG_CDR            0x08
#define MAX44009_CFG_TIMER          0x07


class Max44009
{
public:
    Max44009(const uint8_t address);

    uint32_t        getLux();
    int             getError();

    // check datasheet for detailed behavior
    void    setConfiguration(uint8_t);
    uint8_t getConfiguration();
    void    setAutomaticMode();
    void    setContinuousMode();
    // CDR = Current Divisor Ratio
    // CDR = 1 ==> only 1/8th is measured
    // TIM = Time Integration Measurement (table)
    // 000  800ms
    // 001  400ms
    // 010  200ms
    // 011  100ms
    // 100   50ms
    // 101   25ms
    // 110   12.5ms
    // 111    6.25ms
    void    setManualMode(uint8_t CDR, uint8_t TIM);

private:

    uint8_t read(uint8_t reg);
    void    write(uint8_t, uint8_t);

    uint8_t _address;
    uint8_t _data;
    int     _error;
};
#endif
