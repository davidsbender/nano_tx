/**
 * @file iis2iclx.c
 * @brief IIS2ICLX inclination sensor interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 05.12.2022
 */

#include "iis2iclx.h"
#include "definitions.h"                // GPIO
#include "conversion.h"                 // Bit reversal
#include <stddef.h>
#include <math.h>

/**
 * IIS2ICLX inclination sensor register definitions
 */
#define IIS2ICLX_RW_BIT 				15
#define IIS2ICLX_ADDR_BIT 				8
#define IIS2ICLX_ADDR_MASK				0x7F

#define IIS2ICLX_DATA_BIT 				0
#define IIS2ICLX_DATA_MASK				0xFF

#define IIS2ICLX_ADDR_WHOAMI			0x0F
#define IIS2ICLX_ADDR_CTRL1_XL			0x10
#define IIS2ICLX_ADDR_STATUS 			0x1E
	#define IIS2ICLX_STATUS_XLDA_BIT		0
	#define IIS2ICLX_STATUS_XLDA_MASK		0x01
#define IIS2ICLX_ADDR_OUTX_L_A 			0x28
#define IIS2ICLX_ADDR_OUTX_H_A 			0x29
#define IIS2ICLX_ADDR_OUTY_L_A 			0x2A
#define IIS2ICLX_ADDR_OUTY_H_A 			0x2B

IIS2ICLX_DATA iis2iclxData;

void IIS2ICLX_Init(void)
{
    memset(&iis2iclxData, 0, sizeof(IIS2ICLX_DATA));
    
	// Configure IIS2ICLX for 12.5 Hz update rate, +/- 2 G acceleration range
	uint8_t data = (1 << 4) | (1 << 2);
	IIS2ICLX_Write(IIS2ICLX_ADDR_CTRL1_XL, data);

	// Read diagnostic data
	IIS2ICLX_ReadDiagnostic();
}

int IIS2ICLX_Write(int address, uint8_t data)
{
    // Remark: on the PIC32MX we no longer need busy delays to ensure IIS2ICLX
    // compliant timing (e.g. tNcsHigh >= 350 ns).
    // At 8 MHz and compile optimization level 0, NCS high time between
    // consecutive IIS2ICLX_Read() calls is 18.75 us.
    // This translates to 2.34 us at 64 MHz.
    // Better optimization levels have not been tested though.

	// Prepare command
	uint16_t command = (0 << IIS2ICLX_RW_BIT) | 
                  ((address & IIS2ICLX_ADDR_MASK) << IIS2ICLX_ADDR_BIT);
	command |= ((int)(data & IIS2ICLX_DATA_MASK)) << IIS2ICLX_DATA_BIT;
    command = byterev16(command);

	// Send command to chip
	GPIO_RC6_SPI_NCS_INCL_Clear();
    bool status = SPI1_Write(&command, sizeof(command));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RC6_SPI_NCS_INCL_Set();

	return (status)? 0: -1;
}

int IIS2ICLX_Read(int address, uint8_t* data)
{
    // Remark: on the PIC32MX we no longer need busy delays to ensure IIS2ICLX
    // compliant timing (e.g. tNcsHigh >= 350 ns).
    // At 8 MHz and compile optimization level 0, NCS high time between
    // consecutive IIS2ICLX_Read() calls is 18.75 us.
    // This translates to 2.34 us at 64 MHz.
    // Better optimization levels have not been tested though.
    
	// Prepare command and response buffers
	uint16_t command = (1 << IIS2ICLX_RW_BIT) |
                       ((address & IIS2ICLX_ADDR_MASK) << IIS2ICLX_ADDR_BIT);
    command = byterev16(command);
    uint16_t resp = 0;

	// Send command to chip
	GPIO_RC6_SPI_NCS_INCL_Clear();
    bool status = SPI1_WriteRead(&command, sizeof(command), &resp, sizeof(resp));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RC6_SPI_NCS_INCL_Set();

	// Extract data
    resp = byterev16(resp);
	if (data != NULL) {
		*data = (uint8_t)((resp >> IIS2ICLX_DATA_BIT) & IIS2ICLX_DATA_MASK);
	}

	return (status)? 0: -1;
}

int IIS2ICLX_ReadDiagnostic(void)
{
	uint8_t data = 0;

	int retval = IIS2ICLX_Read(IIS2ICLX_ADDR_WHOAMI, &data);
	iis2iclxData.id = (int)data;

	iis2iclxData.x = 0;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTX_L_A, &data);
	iis2iclxData.x |= (int)data;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTX_H_A, &data);
	iis2iclxData.x |= (int)((int16_t)(((int16_t)data) << 8));

	iis2iclxData.y = 0;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTY_L_A, &data);
	iis2iclxData.y |= (int)data;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTY_H_A, &data);
	iis2iclxData.y |= (int)((int16_t)(((int16_t)data) << 8));

	iis2iclxData.w = atan2(iis2iclxData.x, iis2iclxData.y);

	return retval;
}

int IIS2ICLX_ReadAcceleration(void)
{
	uint8_t data = 0;

	int retval = IIS2ICLX_Read(IIS2ICLX_ADDR_STATUS, &data);
	iis2iclxData.allCalls++;
	if ((data >> IIS2ICLX_STATUS_XLDA_BIT) & IIS2ICLX_STATUS_XLDA_MASK) {
		iis2iclxData.newDataCalls++;
        iis2iclxData.consecutiveNewDataCalls += iis2iclxData.newData;
		iis2iclxData.newData = 1;
	} else {
        iis2iclxData.newData = 0;
        return retval;
    }

	iis2iclxData.x = 0;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTX_L_A, &data);
	iis2iclxData.x |= (int)data;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTX_H_A, &data);
	iis2iclxData.x |= (int)((int16_t)(((int16_t)data) << 8));

	iis2iclxData.y = 0;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTY_L_A, &data);
	iis2iclxData.y |= (int)data;
	retval = IIS2ICLX_Read(IIS2ICLX_ADDR_OUTY_H_A, &data);
	iis2iclxData.y |= (int)((int16_t)(((int16_t)data) << 8));

	iis2iclxData.w = atan2(iis2iclxData.x, iis2iclxData.y);

	return retval;
}
