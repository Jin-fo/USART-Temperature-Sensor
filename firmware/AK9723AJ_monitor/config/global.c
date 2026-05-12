/********************************************************************
 * File:        global.c
 * Author:      Jin Yuan Chen
 * Created:     4/24/2026
 * Modified:    4/28/2026
 *
 * Notes:
 *  This file defines global system variables used across the
 *  embedded firmware application.
 *
 * Description:
 *  Provides shared state variables for system configuration,
 *  display control, sensor data processing, and register-level
 *  communication with external peripherals such as AK9723.
 *
 *  These variables are accessed across multiple modules including
 *  USART, TWI (I2C), ADC processing, and display logic.
 ********************************************************************/

#include "global.h"

//=================================================================//
//                      System Control Variables                   //
//=================================================================//

/**
 * @brief Configuration flag for runtime register updates.
 *
 * @details
 * When set, indicates that a register write operation should be
 * executed for the AK9723 sensor via TWI interface.
 */
uint8_t config_sensor;

uint8_t toggle_LCD;

/**
 * @brief Active display page selector.
 *
 * @details
 * Controls SERLCD output mode:
 *  - Page 0: System splash / identification
 *  - Page 1: Sensor measurement display
 *  - Page 2: Control register display
 */
uint8_t page_num;

//=================================================================//
//                     AK9723 Register Interface                   //
//=================================================================//

/**
 * @brief Target register address for configuration write.
 */
uint8_t reg_addr;

/**
 * @brief Value to be written to target register.
 */
uint8_t reg_value;

//=================================================================//
//                     Control Structure State                     //
//=================================================================//

/**
 * @brief Control register structure for AK9723 sensor.
 *
 * @details
 * Holds decoded control register values used for configuration
 * monitoring and debugging.
 */
cntl_t cntl;
meas_t meas;

//=================================================================//
//                      Sensor Measurement Data                    //
//=================================================================//

/**
 * @brief Infrared channel 1 measurement in millivolts.
 */
float ir1_mV;

/**
 * @brief Infrared channel 2 measurement in millivolts.
 */
float ir2_mV;

/**
 * @brief Forward voltage measurement in millivolts.
 */
float vf_mV;