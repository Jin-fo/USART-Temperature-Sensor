/********************************************************************
 * File:        globals.h
 * Author:      Jin Yuan Chen
 * Created:     4/24/2026
 * Modified:    4/28/2026
 *
 * Notes:
 *  This header defines all globally shared variables used across
 *  the embedded firmware project.
 *
 * Description:
 *  Provides shared system state for:
 *   - Sensor configuration and control (AK9723 via TWI)
 *   - Display page selection (SERLCD via USART2)
 *   - Register-level communication interface
 *   - Processed sensor measurement values
 *
 *  These variables are declared as extern and defined in global.c
 *  to ensure single-instance linkage across modules.
 ********************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

//=================================================================//
//                          Standard Includes                       //
//=================================================================//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>

#define F_CPU 4000000
#include <util/delay.h>

//=================================================================//
//                        Compile-Time Constants                   //
//=================================================================//

/**
 * @brief Size of SERLCD information buffer.
 */
#define info_size 4

/**
 * @brief Number of control registers in AK9723.
 */
#define cntl_size 10
#define meas_size 11
//=================================================================//
//                     Control Register Structure                  //
//=================================================================//

/**
 * @brief AK9723 control register map.
 *
 * @details
 * Provides both:
 *  - Raw register array access (reg[])
 *  - Named field access (at struct)
 *
 * Used for:
 *  - Sensor configuration decoding
 *  - Debugging register states
 *  - Runtime control updates
 */
typedef union {

    /**
     * @brief Raw register access array.
     */
    uint8_t reg[cntl_size];

    /**
     * @brief Named control register fields.
     */
    struct {
        uint8_t c0;
        uint8_t c1;
        uint8_t c2;
        uint8_t c3;
        uint8_t c4;
        uint8_t c5;
        uint8_t c6;
        uint8_t c7;
        uint8_t c8;
        uint8_t c9;
    } at;

} cntl_t;

typedef union
{
	uint8_t reg[meas_size];

	struct
	{
		uint8_t ST1;

		uint8_t IR1L;
		uint8_t IR1M;
		uint8_t IR1H;

		uint8_t IR2L;
		uint8_t IR2M;
		uint8_t IR2H;

		uint8_t TEMPL;
		uint8_t TEMPH;

		uint8_t VFL;
		uint8_t VFH;

	} at;

} meas_t;

//=================================================================//
//                      System Control Variables                   //
//=================================================================//

/**
 * @brief Configuration flag for runtime register write operation.
 *
 * @details
 * When set to 1, triggers a TWI write operation to the AK9723
 * control register space.
 */
extern uint8_t config_sensor;
extern uint8_t toggle_LCD;
/**
 * @brief Active SERLCD display page selector.
 *
 * @details
 * Determines which display page is rendered:
 *  - 0: System identification page
 *  - 1: Sensor measurement page
 *  - 2: Control register debug page
 */
extern uint8_t page_num;

/**
 * @brief Target register address for sensor configuration.
 */
extern uint8_t reg_addr;

/**
 * @brief Value to be written to target register.
 */
extern uint8_t reg_value;

//=================================================================//
//                     Sensor Measurement Values                   //
//=================================================================//

/**
 * @brief Global control register snapshot.
 */
extern cntl_t cntl;

extern meas_t meas;

/**
 * @brief Infrared channel 1 voltage (mV).
 */
extern float ir1_mV;

/**
 * @brief Infrared channel 2 voltage (mV).
 */
extern float ir2_mV;

/**
 * @brief Forward voltage measurement (mV).
 */
extern float vf_mV;

#endif /* GLOBAL_H */