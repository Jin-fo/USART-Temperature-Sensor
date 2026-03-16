#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#define F_CPU 4000000
#include <util/delay.h>   // delay functions

#define DISPLAY_SIZE 80   // 4 lines × 20 characters LCD

// Required linker flags for float printf
// AVR/GNU Linker->Miscellaneous->Other Linker Flags:
// -Wl,-u,vfprintf -lprintf_flt -lm

// Display buffer structure
// Allows access either as a raw buffer or as 4 text lines
typedef union {
	uint8_t buffer[DISPLAY_SIZE + 4];   // full display buffer
	struct {
		uint8_t ln1[21];   // line1 (20 chars + null)
		uint8_t ln2[21];   // line2
		uint8_t ln3[21];   // line3
		uint8_t ln4[21];   // line4
	} buf;
} dsp_t;

dsp_t dsp;   // global display buffer


// Union to access temperature data as
// a 16-bit value or two 8-bit registers
typedef union {
	uint16_t byte16;   // 16-bit temperature data
	struct {
		uint8_t byt_L;	 // low byte
		uint8_t byt_H;	 // high byte
	} loc;
} reg16_t;


reg16_t reg;  // variable storing LM75 temperature register

// signed temperature after adjusting LM75 data format
int16_t signed_byte16;	// adjusted signed({byt_H, byt_L} >> 5)
void TWI0_init(void);
uint16_t TWI0_LM75_read(uint8_t saddr);
uint16_t Sign_adjust(uint16_t Byt_HL);
void update_SerLCD(void);
void write_twi0_SerLCD(uint8_t saddr, uint8_t data);
float float_Centigrade(uint16_t temp16);
void clear_display_buffs(void);
float float_Fahrenheit(uint16_t temp16);

int main(void) {

	TWI0_init();   // initialize TWI (I2C) module

	while(1) {
		clear_display_buffs();
		// read temperature register from LM75 sensor
		reg.byte16 = TWI0_LM75_read(0x91);
		// convert raw data into signed integer temperature
		signed_byte16 = Sign_adjust(reg.byte16);
		// update LCD with new temperature value
		update_SerLCD();
	}
}

void TWI0_init(void) {

	TWI0.MBAUD = 0x0f;         // set TWI clock speed

	TWI0.MCTRLA |= (TWI_ENABLE_bm);  // enable TWI master mode

	TWI0.MSTATUS |= 0x01;      // force TWI bus state to idle
}	


// Read two bytes from LM75 temperature register
uint16_t TWI0_LM75_read(uint8_t saddr) {
	while((TWI0.MSTATUS & 0x03) != 0x01);
	TWI0.MADDR = saddr; // send slave address (LM75)
	while(!(TWI0.MSTATUS & 0x80)); // wait until address transfer complete
	if (!(TWI0.MSTATUS & TWI_RXACK_bm)) { // check if slave acknowledged
		while(!(TWI0.MSTATUS & TWI_RIF_bm)); // wait until first byte received
		reg.loc.byt_H = TWI0.MDATA; // read MSB of temperature
		TWI0.MCTRLB = 0x02;
		while(!(TWI0.MSTATUS & TWI_RIF_bm)); // wait until second byte received
		reg.loc.byt_L = TWI0.MDATA; // read LSB
		TWI0.MCTRLB = 0x07;
	}
	// combine high and low bytes into a single 16-bit value
	return ((uint16_t)reg.loc.byt_H << 8) | ((uint16_t)reg.loc.byt_L);
}


// Convert LM75 raw data into signed integer temperature
uint16_t Sign_adjust(uint16_t Byt_HL) {
	if (Byt_HL & 0x8000) {
		return (int16_t)((~(Byt_HL >> 8) + 0x0001) | 0x8000);
		} else {
		return (uint16_t)(Byt_HL) >> 8;
	}
}


// Update LCD display with temperature data
void update_SerLCD(void){

	// convert LM75 raw value to Celsius float
	float C_temp32 = float_Centigrade(reg.byte16);
	float F_temp32 = float_Fahrenheit(reg.byte16);
	// first line: sensor title
	snprintf((char*)dsp.buf.ln1, 41, "LM75 Temp Sensor");

	// start of binary display
	snprintf((char*)dsp.buf.ln2, 21, ">0b");

	// convert signed temperature to 16-bit binary string
	for (int i = 0; i < 16; i++) {

		// check each bit and print '1' or '0'
		dsp.buf.ln2[i+3] = (signed_byte16 & (1 << (15-i))) ? '1' : '0';
	}

	dsp.buf.ln2[20] = '\0';   // terminate string

	// third line: temperature in Celsius
	snprintf((char*)dsp.buf.ln3, 41, "Centigrade:%6.2fC", C_temp32);

	// fourth line blank
	snprintf((char*)dsp.buf.ln4, 41, "Fahrenheit:%6.2fC", F_temp32);  // display Fahrenheit value

	// send display buffer to LCD through I2C
	for (int i = 0; i < DISPLAY_SIZE+4; i++) {

		if (dsp.buffer[i] != '\0') {
			write_twi0_SerLCD((0x72 << 1), dsp.buffer[i]); // send character
			_delay_ms(1);   // small delay for LCD processing
		}
	}

	_delay_ms(1);  // refresh delay
}


// send one byte to SerLCD via TWI/I2C
void write_twi0_SerLCD(uint8_t saddr, uint8_t data) {

	while(!(TWI0.MSTATUS & 0x03));	// wait until master is idle

	TWI0.MADDR = saddr;  // transmit slave address

	while(!(TWI0.MSTATUS & 0x40));  // wait until address transmission completes

	if (!(TWI0.MSTATUS & 0x10)) {   // check if ACK received

		TWI0.MDATA = data;  // load data byte to transmit

		while(!(TWI0.MSTATUS & 0x40)); // wait until data transmission completes
		TWI0.MCTRLB = 0x07;
		return;
	}
	else {

		return;  // exit if device did not acknowledge
	}
}

// Convert LM75 raw temperature into Celsius float value
float float_Centigrade(uint16_t temp16) {

	float fraction, whole;

	// LM75 resolution = 0.5°C
	fraction = (float)((temp16 >> 8) & 0x01) * 0.5;

	// check negative temperature
	if (temp16 & (1 << 15)) {

		// convert two's complement negative value
		whole = (float)(~((int16_t)temp16 >> 8) + 0x01);

		return -(whole + fraction);
	}

	// positive temperature
	whole = (float)(temp16 >> 8);

	return (whole + fraction);
}
// Convert Celsius to Fahrenheit
float float_Fahrenheit(uint16_t temp16) {

	return ((float_Centigrade(temp16) * 1.8) + 32.0);
}


void clear_display_buffs(void) {
	for (int i = 0; i < DISPLAY_SIZE + 4; i++) {
		dsp.buffer[i] = ' ';
	}
	write_twi0_SerLCD((0x72 << 1), 0xFE); //FE
	_delay_ms(1);
	write_twi0_SerLCD((0x72 << 1), 0x80); //80
}