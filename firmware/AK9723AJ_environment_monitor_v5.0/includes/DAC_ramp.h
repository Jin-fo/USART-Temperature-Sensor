#ifndef DAC_ramp_H
#define DAC_ramp_H

#include <stdint.h>

void init_DAC0(void);
void DAC0_write(uint8_t, uint8_t);

#endif