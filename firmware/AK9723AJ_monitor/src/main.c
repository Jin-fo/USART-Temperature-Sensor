/**
 * @file main.c
 * @author Jin Yuan Chen
 * @date 4/24/2026
 *
 * @brief Main application for AK9723 sensor acquisition and SERLCD output
 * on AVR128DB48 using modular drivers (TWI, USART, utilities).
 *
 * @details
 * This file contains the main runtime loop for sensor acquisition,
 * configuration handling, and LCD output updates.
 *
 * @section include_config Include Path Configuration
 *
 * Header files are NOT referenced using relative paths.
 * Instead, include directories are provided via build system (-I flags).
 *
 * Required include paths:
 * - ../system   (globals, system state)
 * - ../drivers  (UART, TWI, LCD, sensors)
 * - ../utils    (helper functions)
 *
 * Build Configuration:
 * @code
 * Atmel Studio:
 * Toolchain ? AVR/GNU C Compiler ? Directories
 *
 * Makefile:
 * -I../system -I../drivers -I../utils
 * @endcode
 *
 * @section linker_config Linker Configuration
 *
 * AVR disables float printf by default to reduce binary size.
 *
 * Enable float printf support:
 * @code
 * -Wl,-u,vfprintf -lprintf_flt -lm
 * @endcode
 *
 * @par IDE Setting
 * AVR/GNU Linker ? Miscellaneous ? Other Linker Flags
 */

#include "global.h"
#include "serlcd.h"
#include "ak9723.h"
#include "comd.h"
#include "DAC_ramp.h"

//=================================================================//
//                        System Initialization                    //
//=================================================================//

/**
 * @brief Initializes system status indicator (POST LED test).
 *
 * Toggles PORTA PIN7 to indicate successful startup.
 */
void init_STATS(void)
{
    PORTA.DIR |= PIN7_bm;

    PORTA.OUT |= PIN7_bm;
    _delay_ms(500);
    PORTA.OUT &= ~PIN7_bm;
}

//=================================================================//
//                          Main Function                          //
//=================================================================//

/**
 * @brief Main application entry point.
 *
 * @details
 * Initializes peripherals and enters infinite control loop.
 *
 * Key operations:
 * - USART initialization
 * - TWI initialization
 * - Sensor initialization (AK9723)
 * - Configuration handling
 * - Read AK9723 sensor data 
 * - Updates LCD output value
 *
 * @note Runs indefinitely.
 */
int main(void)
{
    init_avr_USART3();
    init_avr_USART2();
	init_avr_TWI0();
	
	init_DAC0();	//task 4										
	
	init_STATS();
	init_SerLCD(139);	//task 1
    init_AK9723();
    sei();
	
    while (1)
    {
		const char test_stream[] = 
		"P1\r"
		"P2\r"
		"P1\r"
		"P0\r";
		for (int i = 0; test_stream[i] != '\0'; i++) {
			rx_char = test_stream[i];
			USART3_Comd_parse_fsm(p_state, rx_char);

        if (config_sensor)
        {
			init_AK9723();
			TWI0_AK9723_byte_write(AK_cntl_reg + reg_addr, reg_value); //reintialize 
			config_sensor = 0;
			_delay_ms(2);
        }
		
        switch (page_num) 
        {
            case 0:
                USART2_SerLCD_page0();
                break;

            case 1:
				TWI0_AK9723_start_meas();
                if (!(PORTA.IN & PIN4_bm)) {
                    TWI0_AK9723_read_meas();

                    if (get_AK9723_stats() & (3 << 1)) {
                        PORTA.OUT |= PIN7_bm;
                        _delay_ms(1000);
                        PORTA.OUT &= ~PIN7_bm;
						
                    }
					if (!toggle_LCD) {						//task 6
						ADC_compute_meas();
						
						USART2_SerLCD_page1();
					}
					DAC0_write(meas.at.IR1H, meas.at.IR1M); //task 5
                }
				
				_delay_ms(2);
                break;

            case 2:
                TWI0_AK9723_read_cntl();
                USART2_SerLCD_page2();
                break;

            default:
				init_AK9723();
                USART2_SerLCD_page0();
                break;
        }
		}
    }
}