#ifndef PWF_AS3935_h
#define PWF_AS3935_h

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <SPI.h>

class PWF_AS3935
{
  public:
    PWF_AS3935(uint8_t CSx, uint8_t IRQx);
    void AS3935_ManualCal(uint8_t capacitance, uint8_t disturber, uint8_t environment);
    void AS3935_DefInit(void);
    void AS3935_PowerUp(void);
    void AS3935_PowerDown(void);
    void AS3935_DisturberEn(void);
    void AS3935_DisturberDis(void);
    void AS3935_SetIRQ_Output_Source(uint8_t irq_select);
    void AS3935_SetTuningCaps(uint8_t cap_val);
    uint8_t AS3935_GetInterruptSrc(void);
    uint8_t AS3935_GetLightningDistKm(void);
    uint32_t AS3935_GetStrikeEnergyRaw(void);
    uint8_t AS3935_SetMinStrikes(uint8_t min_strk);
    void AS3935_ClearStatistics(void);
    void AS3935_SetOutdoors(void);
    void AS3935_SetIndoors(void);
    uint8_t AS3935_GetNoiseFloorLvl(void);
    void AS3935_SetNoiseFloorLvl(uint8_t nf_sel);
    uint8_t AS3935_GetWatchdogThreshold(void);
    void AS3935_SetWatchdogThreshold(uint8_t wdth);
    uint8_t AS3935_GetSpikeRejection(void);
    void AS3935_SetSpikeRejection(uint8_t srej);
    void AS3935_SetLCO_FDIV(uint8_t fdiv);
    void AS3935_PrintAllRegs(void);

  private:
    int8_t _cs, _irq;
    uint8_t _sing_reg_read(uint8_t RegAdd);
    void _sing_reg_write(uint8_t RegAdd, uint8_t DataMask, uint8_t RegData);
    void _AS3935_Reset(void);
    void _CalRCO(void);
};

#endif




