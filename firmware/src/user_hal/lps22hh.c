#include "lps22hh.h"
#include "definitions.h"                // GPIO
#include "conversion.h"                 // Bit reversal
#include <stddef.h>
#include <math.h>

/**
 * LPS22HH accelerometer and gyroscope register definitions
 */
#define LPS22HH_RW_BIT 				15
#define LPS22HH_ADDR_BIT 			8
#define LPS22HH_ADDR_MASK			0x7F

#define LPS22HH_DATA_BIT 			0
#define LPS22HH_DATA_MASK			0xFF

#define LPS22HH_ADDR_WHOAMI			0x0F
#define LPS22HH_ADDR_CTRL2          0x11
    #define LPS22HH_CTRL2_ONE_SHOT_BIT  0
    #define LPS22HH_CTRL2_ONE_SHOT_MASK 0x01
    #define LPS22HH_CTRL2_LOW_NOISE_BIT  1
    #define LPS22HH_CTRL2_LOW_NOISE_MASK 0x01
#define LPS22HH_ADDR_STATUS         0x27
	#define LPS22HH_STATUS_P_DA_BIT         0
	#define LPS22HH_STATUS_P_DA_MASK        0x01
#define LPS22HH_ADDR_PRESS_OUT_XL   0x28
#define LPS22HH_ADDR_PRESS_OUT_L    0x29
#define LPS22HH_ADDR_PRESS_OUT_H    0x2A
#define LPS22HH_ADDR_TEMP_OUT_L     0x2B
#define LPS22HH_ADDR_TEMP_OUT_H     0x2C

LPS22HH_DATA lps22hhData = {0};

int LPS22HH_Write(int address, uint8_t data);
int LPS22HH_Read(int address, uint8_t* data);

void LPS22HH_Init(void)
{
    memset(&lps22hhData, 0, sizeof(LPS22HH_DATA));
    
    uint8_t data = 0;
	data |= LPS22HH_CTRL2_ONE_SHOT_MASK << LPS22HH_CTRL2_ONE_SHOT_BIT;
    data |= LPS22HH_CTRL2_LOW_NOISE_MASK << LPS22HH_CTRL2_LOW_NOISE_BIT;
	LPS22HH_Write(LPS22HH_ADDR_CTRL2, data);
}

int LPS22HH_Write(int address, uint8_t data)
{
	// Prepare command
	uint16_t command = (0 << LPS22HH_RW_BIT) | 
                  ((address & LPS22HH_ADDR_MASK) << LPS22HH_ADDR_BIT);
	command |= ((int)(data & LPS22HH_DATA_MASK)) << LPS22HH_DATA_BIT;
    command = byterev16(command);

	// Send command to chip
	GPIO_RA1_SPI_NCS_APS_Clear();
    bool status = SPI1_Write(&command, sizeof(command));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RA1_SPI_NCS_APS_Set();

	return (status)? 0: -1;
}

int LPS22HH_Read(int address, uint8_t* data)
{
	// Prepare command and response buffers
	uint16_t command = (1 << LPS22HH_RW_BIT) |
                       ((address & LPS22HH_ADDR_MASK) << LPS22HH_ADDR_BIT);
    command = byterev16(command);
    uint16_t resp = 0;

	// Send command to chip
	GPIO_RA1_SPI_NCS_APS_Clear();
    bool status = SPI1_WriteRead(&command, sizeof(command), &resp, sizeof(resp));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RA1_SPI_NCS_APS_Set();

	// Extract data
    resp = byterev16(resp);
	if (data != NULL) {
		*data = (uint8_t)((resp >> LPS22HH_DATA_BIT) & LPS22HH_DATA_MASK);
	}

	return (status)? 0: -1;
}

int LPS22HH_ReadDiagnostic(void)
{
	uint8_t data = 0;
	int retval = LPS22HH_Read(LPS22HH_ADDR_WHOAMI, &data);
	lps22hhData.id = (int)data;
    return retval;
}

int LPS22HH_ReadPressure(void)
{
	uint8_t data = 0;
	int retval = LPS22HH_Read(LPS22HH_ADDR_STATUS, &data);
	if ((data >> LPS22HH_STATUS_P_DA_BIT) & LPS22HH_STATUS_P_DA_MASK) {
        lps22hhData.p = 0;
        retval = LPS22HH_Read(LPS22HH_ADDR_PRESS_OUT_H, &data);
        lps22hhData.p |= ((uint32_t)data) << 16;
        retval = LPS22HH_Read(LPS22HH_ADDR_PRESS_OUT_L, &data);
        lps22hhData.p |= ((uint32_t)data) << 8;
        retval = LPS22HH_Read(LPS22HH_ADDR_PRESS_OUT_XL, &data);
        lps22hhData.p |= ((uint32_t)data) << 0;
        
        lps22hhData.t = 0;
        retval = LPS22HH_Read(LPS22HH_ADDR_TEMP_OUT_H, &data);
        lps22hhData.t |= ((uint32_t)data) << 8;
        retval = LPS22HH_Read(LPS22HH_ADDR_TEMP_OUT_L, &data);
        lps22hhData.t |= ((uint32_t)data) << 0;
        
        data = 0;
        data |= LPS22HH_CTRL2_ONE_SHOT_MASK << LPS22HH_CTRL2_ONE_SHOT_BIT;
        data |= LPS22HH_CTRL2_LOW_NOISE_MASK << LPS22HH_CTRL2_LOW_NOISE_BIT;
        LPS22HH_Write(LPS22HH_ADDR_CTRL2, data);
	}
	return retval;
}