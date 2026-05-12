/********************************************************************
 * File:        serlcd.c
 * Author:      Jin Yuan Chen
 * Created:     4/24/2026
 * Modified:    4/28/2026
 *
 * @file serlcd.c
 * @brief USART2-based SERLCD driver for multi-page display output.
 *
 * @details
 * This module provides interrupt-driven USART2 communication with
 * a SERLCD display. It supports formatted multi-page output for:
 * - Sensor measurements
 * - Control register debugging
 * - System identification page
 ********************************************************************/

#include "global.h"
#include "serlcd.h"

//=================================================================//
//                     USART2 TX CONTROL FLAG                      //
//=================================================================//

/**
 * @brief Transmission ready flag.
 *
 * @details
 * Controls USART2 byte transmission flow.
 *
 * Pseudo-logic:
 * @code
 * 1. Set when USART is idle
 * 2. Cleared when transmission starts
 * 3. Set again in TX ISR after completion
 * @endcode
 */
volatile uint8_t uart_ready = 1;
volatile uint8_t spi_ready = 1;
volatile uint16_t def_baud = 1667;
/**
 * @brief USART2 Data Register Empty ISR.
 *
 * @details
 * Pseudo-logic:
 * @code
 * 1. Load next byte into TX register
 * 2. Mark transmission complete (uart_ready = 1)
 * 3. Disable DRE interrupt until next byte
 * @endcode
 */

void USART2_SerLCD_clear(void);

ISR(USART2_DRE_vect)
{
    USART2.TXDATAL = tx_char;
    uart_ready = 1;
    USART2.CTRLA &= ~USART_DREIE_bm;
}

ISR(SPI0_INT_vect)
{
	volatile uint8_t dummy = SPI0.DATA;  // clear interrupt flag
	spi_ready = 1;
}

//=================================================================//
//                     USART2 INITIALIZATION                       //
//=================================================================//

/**
 * @brief Initialize USART2 for SERLCD communication.
 *
 * @details
 * Pseudo-logic:
 * @code
 * 1. Configure port multiplexing
 * 2. Set baud rate
 * 3. Enable TX mode
 * 4. Set 8N1 frame format
 * 5. Enable TX interrupt
 * 6. Configure TX pin as output
 * @endcode
 */
void init_avr_USART2()
{
    PORTMUX.USARTROUTEA |= PIN4_bm;

    USART2.BAUD  = def_baud;
    USART2.CTRLB = 0x40;
    USART2.CTRLC = 0x03;
    PORTF.DIR |= PIN4_bm;
}

void init_SerLCD(uint16_t baud)
{
	if (def_baud == 1667 && baud == 138) {
		USART2_SerLCD_byte_write('|');
		USART2_SerLCD_byte_write('-');
		USART2_SerLCD_byte_write('|');
		USART2_SerLCD_byte_write(0x12);
		def_baud = 138;
		init_avr_USART2();
		
	} else if ((def_baud == 138) &&  (baud == 1667)) {
		USART2_SerLCD_byte_write('|');
		USART2_SerLCD_byte_write('-');
		USART2_SerLCD_byte_write('|');
		USART2_SerLCD_byte_write(0x0D);
		def_baud = 1667;
		init_avr_USART2();
	} else {
		USART2_SerLCD_byte_write('|');
		USART2_SerLCD_byte_write(0x08);
	}
}

//=================================================================//
//                        SERLCD PAGE 0                             //
//=================================================================//

/**
 * @brief System identification / splash page.
 *
 * @details
 * Pseudo-logic:
 * @code
 * 1. Send LCD control header
 * 2. Load static text into buffer
 * 3. Transmit buffer byte-by-byte
 * 4. Delay for display stability
 * @endcode
 */
void USART2_SerLCD_page0()
{
	USART2_SerLCD_clear();

    snprintf((char*)dsp1.buf.ln1, 21, " AK9723AJ Temp/C02  ");
    snprintf((char*)dsp1.buf.ln2, 21, " ---- Sensor ----   ");
    snprintf((char*)dsp1.buf.ln3, 21, "   Spring ESE 381   ");
    snprintf((char*)dsp1.buf.ln4, 21, "JIN Y. CHEN, ERIC WU");

    for (int i = 0; i < DISPLAY_SIZE + 4; i++)
    {
        if (dsp1.buffer[i] != '\0')
        {
           USART2_SerLCD_byte_write(dsp1.buffer[i]);
           _delay_ms(1);
        }
    }

    //_delay_ms(1000);
}

//=================================================================//
//                        SERLCD PAGE 1                             //
//=================================================================//

/**
 * @brief Real-time sensor measurement display.
 *
 * @details
 * Pseudo-logic:
 * @code
 * 1. Load IR1, IR2, VF values
 * 2. Format values into fixed-width strings
 * 3. Transfer buffer to SERLCD
 * 4. Delay for display refresh
 * @endcode
 */
void USART2_SerLCD_page1()		
{
	USART2_SerLCD_clear();
	//task 2
    snprintf((char*)dsp1.buf.ln1, 21, "IR1 = 0x%02X %02X %02X", meas.at.IR1H, meas.at.IR1M, meas.at.IR1L);
    snprintf((char*)dsp1.buf.ln2, 21, "IR1 = %7.2f mV     ", ir1_mV);
    snprintf((char*)dsp1.buf.ln3, 21, "IR2 = %7.2f mV     ", ir2_mV);
    snprintf((char*)dsp1.buf.ln4, 21, "VF  = %7.2f mV     ", vf_mV);

    for (int i = 0; i < DISPLAY_SIZE + 4; i++)
    {
        if (dsp1.buffer[i] != '\0')
        {
            USART2_SerLCD_byte_write(dsp1.buffer[i]);
            _delay_ms(1);
        }
    }

    _delay_ms(1000);
}

//=================================================================//
//                        SERLCD PAGE 2                             //
//=================================================================//

/**
 * @brief Control register debug page.
 *
 * @details
 * Pseudo-logic:
 * @code
 * 1. Read control register values (C0–C9)
 * 2. Format into display buffer
 * 3. Transmit buffer to LCD
 * 4. Delay for readability
 * @endcode
 */
void USART2_SerLCD_page2()
{

	USART2_SerLCD_clear();
	snprintf((char*)dsp2.buf.ln1, 21, "AK9723 cntl  , C0=%02X ", cntl.at.c0);

	snprintf((char*)dsp2.buf.ln2, 21,
			 "C1=%02X, C2=%02X, C3=%02X ",
			 cntl.at.c1, cntl.at.c2, cntl.at.c3);

	snprintf((char*)dsp2.buf.ln3, 21,
			 "C4=%02X, C5=%02X, C6=%02X ",
			 cntl.at.c4, cntl.at.c5, cntl.at.c6);

	snprintf((char*)dsp2.buf.ln4, 21,
			 "C7=%02X, C8=%02X, C9=%02X ",
			 cntl.at.c7, cntl.at.c8, cntl.at.c9);

    for (int i = 0; i < DISPLAY_SIZE + 4; i++)
    {
        if (dsp2.buffer[i] != '\0')
        {
            USART2_SerLCD_byte_write(dsp2.buffer[i]);
            _delay_ms(1);
        }
    }

    _delay_ms(1000);
}

//=================================================================//
//                     USART2 BYTE TRANSMISSION                    //
//=================================================================//

/**
 * @brief Send single character to SERLCD via USART2.
 *
 * @param c Character to transmit
 *
 * @details
 * Pseudo-logic:
 * @code
 * 1. Wait until USART is ready
 * 2. Load character into TX buffer
 * 3. Enable DRE interrupt
 * 4. ISR handles actual transmission
 * @endcode
 */
void USART2_SerLCD_byte_write(char c)
{
    while (!uart_ready);

    uart_ready = 0;
    tx_char = c;

    USART2.CTRLA |= USART_DREIE_bm;
}

void SPI_SerLCD_byte_write(char c)
{
	while (!spi_ready);

	spi_ready = 0;
	tx_char = c;

	SPI0.DATA = tx_char;
}

void USART2_SerLCD_clear() {
	USART2_SerLCD_byte_write('|');
    USART2_SerLCD_byte_write('-');
}