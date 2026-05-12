/********************************************************************
 * File:        parse.h
 * Author:      Jin Yuan Chen
 * Created:     4/24/2026
 * Modified:    4/28/2026
 *
 * Notes:
 *  This header provides utility function declarations for
 *  converting raw multi-byte sensor data into signed integers.
 *
 * Description:
 *  Declares parsing interfaces used across ADC and TWI sensor
 *  processing pipelines. These functions reconstruct signed
 *  values from 16-bit and 24-bit raw sensor outputs.
 ********************************************************************/

#ifndef PARSE_H
#define PARSE_H

//=================================================================//
//                          Standard Includes                       //
//=================================================================//

#include <stdint.h>

//=================================================================//
//                     24-bit Data Parsing API                     //
//=================================================================//

/**
 * @brief Convert 3-byte raw data into signed 32-bit integer.
 */
int32_t parse24(uint8_t L, uint8_t M, uint8_t H);

//=================================================================//
//                     16-bit Data Parsing API                     //
//=================================================================//

/**
 * @brief Convert 2-byte raw data into signed 16-bit integer.
 */
int16_t parse16(uint8_t L, uint8_t H);

#endif /* PARSE_H */