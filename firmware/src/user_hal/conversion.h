/**
 * @file conversion.h
 * @brief Bit and byte order (endianness) conversion
 * @author David Bender <d_bender@bluewin.ch>
 * @date 05.12.2022
 */

#include "definitions.h"

#define SET_BF(bf, value) SET_BF_EXP(bf, value)
#define SET_BF_EXP(msb, lsb, value) ((value & MASK_EXP(msb, lsb)) << lsb)
#define MASK_EXP(msb, lsb) (~(0xFFFFFFFF << (1 + msb - lsb)))

uint16_t bitrev16(uint16_t x);
uint16_t byterev16(uint16_t x);
