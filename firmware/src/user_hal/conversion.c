/**
 * @file conversion.c
 * @brief Bit and byte order (endianness) conversion
 * @author David Bender <d_bender@bluewin.ch>
 * @date 05.12.2022
 */

#include "conversion.h"

/**
 * Bit reversal algorithm from http://aggregate.org/MAGIC/#Bit%20Reversal .
 * @param[in]   x       Original 16 bits value.
 * @return              Bit reversed 16 bits value.
 */
uint16_t bitrev16(uint16_t x)
{
	x = (((x & 0xaaaa) >> 1) | ((x & 0x5555) << 1));
	x = (((x & 0xcccc) >> 2) | ((x & 0x3333) << 2));
	x = (((x & 0xf0f0) >> 4) | ((x & 0x0f0f) << 4));
	return ((x >> 8) | (x << 8));
}

/**
 * Byte (endianness) reversal.
 * @param[in]   x       Original 16 bits value.
 * @return              Byte (or endianness) reversed 16 bits value.
 */
uint16_t byterev16(uint16_t x)
{
    return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}
