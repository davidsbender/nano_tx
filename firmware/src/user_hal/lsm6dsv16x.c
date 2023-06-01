// Note: we actually got an LSM6DSV16BX, not an LSM6DSV16X!

#include "lsm6dsv16x.h"
#include "definitions.h"                // GPIO
#include "conversion.h"                 // Bit reversal
#include <stddef.h>
#include <math.h>

/**
 * LSM6DSV16X accelerometer and gyroscope register definitions
 */
#define LSM6DSV16X_RW_BIT                   15
#define LSM6DSV16X_ADDR_BIT 				8
#define LSM6DSV16X_ADDR_MASK				0x7F

#define LSM6DSV16X_DATA_BIT 				0
#define LSM6DSV16X_DATA_MASK				0xFF

#define LSM6DSV16X_ADDR_WHOAMI              0x0F
#define LSM6DSV16X_ADDR_CTRL1               0x10
    #define LSM6DSV16X_CTRL1_BF_OP_MODE_XL      6,4
    #define LSM6DSV16X_CTRL1_BF_ODR_XL          3,0
#define LSM6DSV16X_ADDR_CTRL2               0x11
    #define LSM6DSV16X_CTRL2_BF_OP_MODE_G       6,4
    #define LSM6DSV16X_CTRL2_BF_ODR_G           3,0
//#define LSM6DSV16X_ADDR_CTRL3               0x12
//    #define LSM6DSV16X_CTRL3_BF_BDU             6,6
#define LSM6DSV16C_ADDR_OUTX_L_G            0x22
#define LSM6DSV16C_ADDR_OUTX_H_G            0x23
#define LSM6DSV16C_ADDR_OUTY_L_G            0x24
#define LSM6DSV16C_ADDR_OUTY_H_G            0x25
#define LSM6DSV16C_ADDR_OUTZ_L_G            0x26
#define LSM6DSV16C_ADDR_OUTZ_H_G            0x27
#define LSM6DSV16C_ADDR_OUTX_L_A            0x28
#define LSM6DSV16C_ADDR_OUTX_H_A            0x29
#define LSM6DSV16C_ADDR_OUTY_L_A            0x2A
#define LSM6DSV16C_ADDR_OUTY_H_A            0x2B
#define LSM6DSV16C_ADDR_OUTZ_L_A            0x2C
#define LSM6DSV16C_ADDR_OUTZ_H_A            0x2D

LSM6DSV16X_DATA lsm6dsv16xData = {0};

int LSM6DSV16X_Write(int address, uint8_t data);
int LSM6DSV16X_Read(int address, uint8_t* data);

int LSM6DSV16X_Init(void)
{
    // Configure accelerometer: high performance mode, 15 Hz update rate
    uint8_t data = 0;
    data |= SET_BF(LSM6DSV16X_CTRL1_BF_OP_MODE_XL, 0);
    data |= SET_BF(LSM6DSV16X_CTRL1_BF_ODR_XL, 3);
    int retval = LSM6DSV16X_Write(LSM6DSV16X_ADDR_CTRL1, data);
    
    // Configure gyroscope: high performance mode, 15 Hz update rate
    data = 0;
    data |= SET_BF(LSM6DSV16X_CTRL2_BF_OP_MODE_G, 0);
    data |= SET_BF(LSM6DSV16X_CTRL2_BF_ODR_G, 3);
    retval = LSM6DSV16X_Write(LSM6DSV16X_ADDR_CTRL2, data);
    
    // Read diagnostic information including ID
    LSM6DSV16X_ReadDiagnostic();
    
    return retval;
}

int LSM6DSV16X_Write(int address, uint8_t data)
{
	// Prepare command
	uint16_t command = (0 << LSM6DSV16X_RW_BIT) | 
                  ((address & LSM6DSV16X_ADDR_MASK) << LSM6DSV16X_ADDR_BIT);
	command |= ((int)(data & LSM6DSV16X_DATA_MASK)) << LSM6DSV16X_DATA_BIT;
    command = byterev16(command);

	// Send command to chip
	GPIO_RC4_SPI_NCS_ACC_Clear();
    bool status = SPI1_Write(&command, sizeof(command));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RC4_SPI_NCS_ACC_Set();

	return (status)? 0: -1;
}

int LSM6DSV16X_Read(int address, uint8_t* data)
{
	// Prepare command and response buffers
	uint16_t command = (1 << LSM6DSV16X_RW_BIT) |
                       ((address & LSM6DSV16X_ADDR_MASK) << LSM6DSV16X_ADDR_BIT);
    command = byterev16(command);
    uint16_t resp = 0;

	// Send command to chip
	GPIO_RC4_SPI_NCS_ACC_Clear();
    bool status = SPI1_WriteRead(&command, sizeof(command), &resp, sizeof(resp));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RC4_SPI_NCS_ACC_Set();

	// Extract data
    resp = byterev16(resp);
	if (data != NULL) {
		*data = (uint8_t)((resp >> LSM6DSV16X_DATA_BIT) & LSM6DSV16X_DATA_MASK);
	}

	return (status)? 0: -1;
}

int LSM6DSV16X_ReadDiagnostic(void)
{
	uint8_t data = 0;
	int retval = LSM6DSV16X_Read(LSM6DSV16X_ADDR_WHOAMI, &data);
	lsm6dsv16xData.id = (int)data;
    return retval;
}

int LSM6DSV16X_ReadTAG(void)
{
    int address = 0x20;
    uint8_t cmd = (1 << (LSM6DSV16X_RW_BIT - 8))
            | ((address & LSM6DSV16X_ADDR_MASK) << (LSM6DSV16X_ADDR_BIT - 8));
    uint8_t resp[15] = {0};

    // Send command to chip
	GPIO_RC4_SPI_NCS_ACC_Clear();
    bool status = SPI1_WriteRead(&cmd, sizeof(cmd), &resp, sizeof(resp));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RC4_SPI_NCS_ACC_Set();
    
    lsm6dsv16xData.t  =  (resp[1] << 0) |  (resp[2] << 8);
    lsm6dsv16xData.gx =  (resp[3] << 0) |  (resp[4] << 8);
    lsm6dsv16xData.gy =  (resp[5] << 0) |  (resp[6] << 8);
    lsm6dsv16xData.gz =  (resp[7] << 0) |  (resp[8] << 8);
    lsm6dsv16xData.ax =  (resp[9] << 0) | (resp[10] << 8);
    lsm6dsv16xData.ay = (resp[11] << 0) | (resp[12] << 8);
    lsm6dsv16xData.az = (resp[13] << 0) | (resp[14] << 8);
    
    return (status)? 0: -1;
}
