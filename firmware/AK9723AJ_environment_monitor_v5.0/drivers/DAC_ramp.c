/*
 * DAC_ramp.c
 *
 * Created: 5/5/2026 8:55:01 PM
 *  Author: Jin
 */ 

#include "global.h"
#include "DAC_ramp.h"

//task 4
void init_DAC0(void)
{
	VREF.DAC0REF = (0x03 | VREF_ALWAYSON_bm);
	PORTA.DIRCLR = PIN6_bm;
	DAC0.CTRLA = (DAC_ENABLE_bm | DAC_OUTEN_bm);
	DAC0.DATA = 0;
}

void DAC0_write(uint8_t dac_val_H, uint8_t dac_val_L)
{
	
	DAC0.DATAL = (dac_val_L & 0xC0);
	DAC0.DATAH = dac_val_H;
}

