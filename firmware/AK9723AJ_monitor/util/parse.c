/********************************************************************
 * File:        parse.c
 * Author:      Jin Yuan Chen
 * Created:     4/24/2026
 * Modified:    4/28/2026
 *
 * Notes:
 *  This module provides utility functions for parsing raw
 *  multi-byte sensor data into signed integer formats.
 *
 * Description:
 *  Implements conversion routines for:
 *   - 24-bit signed sensor data reconstruction
 *   - 16-bit signed sensor data reconstruction
 *
 *  These functions are typically used in ADC/TWI sensor pipelines
 *  such as AK9723 measurement decoding.
 ********************************************************************/

#include "parse.h"

//=================================================================//
//                      24-bit Signed Parsing                      //
//=================================================================//

/**
 * @brief Convert 3-byte raw data into signed 32-bit integer.
 *
 * @param L Low byte of the data.
 * @param M Middle byte of the data.
 * @param H High byte of the data.
 *
 * @return int32_t Signed 32-bit reconstructed value.
 *
 * @details
 * Combines three 8-bit values into a 24-bit signed integer.
 * Performs sign extension if bit 23 is set.
 */
int32_t parse24(uint8_t L, uint8_t M, uint8_t H)
{
	int32_t val = ((int32_t)H << 16) |
	((int32_t)M << 8) |
	((int32_t)L);

	if (val & 0x800000)
	val |= 0xFF000000;

	return val;
}

//=================================================================//
//                      16-bit Signed Parsing                      //
//=================================================================//

/**
 * @brief Convert 2-byte raw data into signed 16-bit integer.
 *
 * @param L Low byte of the data.
 * @param H High byte of the data.
 *
 * @return int16_t Signed 16-bit reconstructed value.
 *
 * @details
 * Combines two 8-bit values into a 16-bit signed integer
 * using little-endian ordering.
 */
int16_t parse16(uint8_t L, uint8_t H)
{
    return (int16_t)((H << 8) | L);
}