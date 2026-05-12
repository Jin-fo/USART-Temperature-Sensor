#ifndef SERLCD_H
#define SERLCD_H

#include <stdint.h>

#define LCD_addr 0x62
#define DISPLAY_SIZE 80

typedef union
{
	uint8_t buffer[DISPLAY_SIZE + 4];

	struct
	{
		uint8_t ln1[21];
		uint8_t ln2[21];
		uint8_t ln3[21];
		uint8_t ln4[21];
	} buf;

} dsp_t;

dsp_t dsp1;
dsp_t dsp2;

void init_avr_USART2(void);
void init_SerLCD(uint16_t);
void USART2_SerLCD_page0(void);
void USART2_SerLCD_page1(void);
void USART2_SerLCD_page2(void);
void USART2_SerLCD_byte_write(char c);

volatile uint8_t tx_char;

#endif /* SERLCD_H */