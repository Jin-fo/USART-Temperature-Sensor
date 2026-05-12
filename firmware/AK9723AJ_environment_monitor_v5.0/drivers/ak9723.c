/********************************************************************
 * File:        ak9723.c
 * Author:      Jin Yuan Chen
 * Created:     4/24/2026
 *
 * @file ak9723.c
 * @brief Low-level driver for AK9723 IR gas sensor using AVR TWI0.
 *
 * @details
 * This module implements sensor communication, configuration,
 * measurement acquisition, and data conversion for the AK9723
 * using the AVR128DB48 TWI (I2C) peripheral.
 *
 * Functional blocks:
 * - TWI0 initialization
 * - Sensor initialization
 * - Register read/write operations
 * - Burst measurement acquisition
 * - Raw data conversion to physical units
 ********************************************************************/

#include "global.h"
#include "ak9723.h"
#include "parse.h"

//=================================================================//
//                      TWI0 INITIALIZATION                        //
//=================================================================//

/**
 * @brief Initialize TWI0 peripheral for AK9723 communication.
 *
 * @details
 * Pseudo-code:
 * @code
 * 1. Set TWI baud rate
 * 2. Enable TWI master mode
 * 3. Enable debug run mode
 * 4. Clear status register
 * 5. Toggle GPIO to reset/stabilize sensor
 * @endcode
 */
void init_avr_TWI0()
{
    TWI0.MBAUD   = 0x0F;
    TWI0.MCTRLA  = TWI_ENABLE_bm;
    TWI0.DBGCTRL = TWI_DBGRUN_bm;
    TWI0.MSTATUS = 0x01;

    PORTA.DIR |= PIN5_bm;
    PORTA.OUT &= ~PIN5_bm;
    _delay_us(200);
    PORTA.OUT |= PIN5_bm;
}

//=================================================================//
//                     AK9723 INITIALIZATION                       //
//=================================================================//

/**
 * @brief Initialize AK9723 sensor configuration registers.
 *
 * @details
 * Pseudo-code:
 * @code
 * 1. Write control register 9 = 0xFF
 * 2. Write control register 8 = 0x00
 * 3. Write control register 5 = 0x02
 * 4. Configure interrupt pin as input
 * @endcode
 */
void init_AK9723()
{
    TWI0_AK9723_byte_write(AK_cntl_reg + 9, 0xFF);
    TWI0_AK9723_byte_write(AK_cntl_reg + 8, 0x00);
	
    PORTA.DIR &= ~PIN4_bm;
}

void TWI0_AK9723_start_meas() 
{
	TWI0_AK9723_byte_write(AK_cntl_reg + 5, 0x02);
}
//=================================================================//
//                     REGISTER ADDRESS WRITE                      //
//=================================================================//

/**
 * @brief Send register address to AK9723.
 *
 * @param raddr Register address
 *
 * @details
 * Pseudo-code:
 * @code
 * 1. Wait for bus ready
 * 2. Send slave write address
 * 3. Check ACK
 * 4. Send register address
 * 5. Wait for completion
 * @endcode
 */
void TWI0_AK9723_set_address(uint8_t raddr)
{
    while(!(TWI0.MSTATUS = 0x03));

    TWI0.MADDR = (AK_addr << 1);

    while(!(TWI0.MSTATUS & TWI_WIF_bm));

    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        TWI0.MCTRLB = 0x03;
        return;
    }

    TWI0.MDATA = raddr;

    while(!(TWI0.MSTATUS & TWI_WIF_bm));
}

//=================================================================//
//                    SENSOR MEASUREMENT READ                     //
//=================================================================//

/**
 * @brief Read full measurement frame from AK9723.
 *
 * @details
 * Pseudo-code:
 * @code
 * 1. Set measurement register address
 * 2. Switch to read mode
 * 3. Loop over measurement bytes:
 *      - Wait for data ready
 *      - Read byte into buffer
 *      - ACK until last byte
 *      - NACK last byte
 * @endcode
 */
void TWI0_AK9723_read_meas()
{
    int n = meas_size;

    TWI0_AK9723_set_address(AK_meas_reg);

    TWI0.MADDR = (AK_addr << 1 | 0x01);

    for (int i = 0; i < n; i++)
    {
        while(!(TWI0.MSTATUS & TWI_RIF_bm));
        meas.reg[i] = TWI0.MDATA;

        if (i == (n - 1)) {
            TWI0.MCTRLB = 0x07;
            break;
        }

        TWI0.MCTRLB = 0x02;
    }
}

//=================================================================//
//                    CONTROL REGISTER READ                       //
//=================================================================//

/**
 * @brief Read AK9723 control register block.
 *
 * @details
 * Pseudo-code:
 * @code
 * 1. Send read request
 * 2. Loop over control register bytes:
 *      - Wait for data ready
 *      - Store into control buffer
 *      - ACK until last byte
 *      - NACK last byte
 * @endcode
 */
void TWI0_AK9723_read_cntl()
{
	TWI0_AK9723_set_address(0x0F);
    int n = cntl_size;

    TWI0.MADDR = (AK_addr << 1) | 0X01;

    while(!(TWI0.MSTATUS & TWI_RIF_bm));

    if (!(TWI0.MSTATUS & TWI_RXACK_bm))
    {
        for (int i = 0; i < n; i++)
        {
            while(!(TWI0.MSTATUS & TWI_RIF_bm));
            cntl.reg[i] = TWI0.MDATA;

            if (i == (n - 1)) {
                TWI0.MCTRLB = 0x07;
                break;
            }

            TWI0.MCTRLB = 0x02;
        }
    }
    else {
        TWI0.MCTRLB = 0x03;
        return;
    }
}

//=================================================================//
//                     SENSOR REGISTER WRITE                      //
//=================================================================//

/**
 * @brief Write a single byte to AK9723 register.
 *
 * @param raddr Register address
 * @param data  Data byte
 *
 * @details
 * Pseudo-code:
 * @code
 * 1. Send register address
 * 2. Write data byte
 * 3. Wait for transmission complete
 * 4. Stop condition
 * @endcode
 */
void TWI0_AK9723_byte_write(uint8_t raddr, uint8_t data)
{
    TWI0_AK9723_set_address(raddr);

    TWI0.MDATA = data;

    while(!(TWI0.MSTATUS & TWI_WIF_bm));

    TWI0.MCTRLB = 0x03;
}

//=================================================================//
//                     SENSOR DATA CONVERSION                     //
//=================================================================//

/**
 * @brief Convert raw sensor values into physical units.
 *
 * @details
 * Pseudo-code:
 * @code
 * 1. Extract raw IR1, IR2, VF values
 * 2. Convert using parse functions
 * 3. Apply scaling factors:
 *    - IR1, IR2: ADC scaling
 *    - VF: offset + gain
 * @endcode
 */
void ADC_compute_meas()
{
    int32_t ir1_val = parse24(meas.at.IR1L, meas.at.IR1M, meas.at.IR1H);
    int32_t ir2_val = parse24(meas.at.IR2L, meas.at.IR2M, meas.at.IR2H);
    int16_t vf_val  = parse16(meas.at.VFL, meas.at.VFH);

    ir1_mV = ir1_val * -0.0000894f;
    ir2_mV = ir2_val *  0.0000894f;
    vf_mV  = vf_val  *  0.04577f + 1400.0f;
}

//=================================================================//
//                     SENSOR STATUS ACCESS                       //
//=================================================================//

/**
 * @brief Get AK9723 status register.
 *
 * @return Status byte (ST1)
 *
 * @details
 * Pseudo-code:
 * @code
 * return measurement_buffer.ST1
 * @endcode
 */
uint8_t get_AK9723_stats()
{
    return meas.at.ST1;
}