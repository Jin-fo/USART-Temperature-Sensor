#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#define F_CPU 4000000
#include <util/delay.h>

#define DISPLAY_SIZE 80
//AVR/GNU Linker->Miscellaneous->Other Linker Flags:
//-Wl,-u,vfprintf -lprintf_flt -lm
typedef union {
	uint8_t buffer[DISPLAY_SIZE + 4];
	struct {
		uint8_t ln1[21];
		uint8_t ln2[21];
		uint8_t ln3[21];
		uint8_t ln4[21];
	} buf;
} dsp_t;

void init_spi0(void);
void update_SerLCD(void);
uint16_t read_spi0_LM74 (void);
float float_Centigrade(uint16_t temp16);
float float_Fahrenheit(uint16_t temp16);
void write_spi0_SerLCD (unsigned char data);
void clear_display_buffs(void);

dsp_t dsp;

int main(void) {
	init_spi0();

	while (1) {
		clear_display_buffs();
		update_SerLCD();
		_delay_ms(500);
	}
}

void init_spi0(void) {
	SPI0.CTRLA |= (SPI_MASTER_bm | SPI_ENABLE_bm);
	SPI0.CTRLB |= 0x00;
	PORTA.DIR |= (PIN4_bm | PIN6_bm | PIN3_bm | PIN7_bm);
	PORTA.OUTSET |= (PIN7_bm | PIN3_bm);
}

void update_SerLCD(void) {
	uint16_t temp16 = read_spi0_LM74();
	float C_temp32 = float_Centigrade(temp16);
	float F_temp32 = float_Fahrenheit(temp16);

	snprintf((char*)dsp.buf.ln1, 21, "LM74 Temp Sensor    ");
	snprintf((char*)dsp.buf.ln2, 21, ">0b");

	for (int i = 0; i < 16; i++) {
		dsp.buf.ln2[i+3] = (temp16 & (1 << (15-i))) ? '1' : '0';
	}
	dsp.buf.ln2[20] = '\0';
		
	snprintf((char*)dsp.buf.ln3, 21, "Centigrade:%6.2fC", C_temp32);
	snprintf((char*)dsp.buf.ln4, 21, "Fahrenheit:%6.2fF", F_temp32);
		
	for (int i = 0; i < DISPLAY_SIZE+4; i++) {
		if (dsp.buffer[i] != '\0') {
			write_spi0_SerLCD(dsp.buffer[i]);
			_delay_ms(1);
		} 
		
	}
}

uint16_t read_spi0_LM74 (void) {
	uint8_t high_byte, low_byte;

	PORTA.OUTCLR = PIN3_bm;
	SPI0.DATA = 0x00;
	while (!(SPI0.INTFLAGS & SPI_IF_bm));
	high_byte = SPI0.DATA;
	
	SPI0.DATA = 0x00;
	while (!(SPI0.INTFLAGS & SPI_IF_bm));
	low_byte = SPI0.DATA;
	PORTA.OUTSET = PIN3_bm;

	return ((uint16_t)high_byte << 8) | low_byte;
}

float float_Centigrade(uint16_t temp16) {
	float fraction, whole;

	fraction = (float)((temp16 >> 3) & 0x0f) * 0.0625;

	if (temp16 & (1 << 15)) {
		whole = (float)(~((int16_t)temp16 >> 7) + 0x01);
		return (whole + fraction) * -1.0;
	}

	whole = (float)(temp16 >> 7);
	return (whole + fraction);
}

float float_Fahrenheit(uint16_t temp16) {
	return ((float_Centigrade(temp16) * 1.8) + 32.0);
}

void write_spi0_SerLCD (unsigned char data) {
	PORTA.OUTCLR = PIN7_bm;
	SPI0.DATA = data;
	while (!(SPI0.INTFLAGS & SPI_IF_bm));
	PORTA.OUTSET = PIN7_bm;
}

void clear_display_buffs(void) {
	for (int i = 0; i < DISPLAY_SIZE+4; i++) {
		dsp.buffer[i] = ' ';
	}
	
	write_spi0_SerLCD('|');
	_delay_ms(1);
	write_spi0_SerLCD('-');
}
