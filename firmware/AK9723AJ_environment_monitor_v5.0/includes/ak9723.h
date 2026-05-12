#ifndef AK9723_H
#define AK9723_H

#include <stdint.h>

#define AK_addr     0x65
#define AK_info_reg 0x00
#define AK_cntl_reg 0x0F
#define AK_meas_reg 0x04

void init_avr_TWI0(void);
void init_AK9723(void);
void TWI0_AK9723_start_meas(void);
void TWI0_AK9723_byte_write(uint8_t reg, uint8_t value);
void TWI0_AK9723_read_meas(void);
void TWI0_AK9723_read_cntl(void);
void ADC_compute_meas(void);
uint8_t get_AK9723_stats(void);

#endif