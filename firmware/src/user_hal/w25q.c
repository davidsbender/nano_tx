/**
 * @file w25q.c
 * @brief W25Q128JV NOR flash memory interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 22.12.2022
 */

#include "definitions.h"                // GPIO
#include "w25q.h"

W25Q_DATA w25qData;

void W25Q_Init(void)
{
    w25qData.busy = false;
}

int W25Q_EnableReset(void)
{
	// Prepare command buffer
	uint8_t command[1] = {0x66};

	// Send command to chip and read response
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    bool status = SPI1_WriteRead(command, sizeof(command), NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

int W25Q_Reset(void)
{
	// Prepare command buffer
	uint8_t command[1] = {0x99};

	// Send command to chip and read response
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    bool status = SPI1_WriteRead(command, sizeof(command), NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

int W25Q_ReadStatusRegister(int reg, uint8_t* resp)
{
    // Prepare command buffer
    uint8_t commandByte = 0x05;
     if (reg == 2) {
        commandByte = 0x35;
    } else if (reg == 3) {
        commandByte = 0x15;
    }
    // Dummy bytes must repeat the command, the datasheet does not point this
    // out but the W25Q does not return status register data otherwise
    uint8_t cmd[2] = {commandByte, commandByte};
    
    // Prepare response buffer
    uint8_t rdata[2] = {0};
    
	// Send command to chip and read response
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    bool status = SPI1_WriteRead(cmd, sizeof(cmd), rdata, sizeof(rdata));
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RB7_SPI_NCS_FLASH_Set();
    
    // Extract response byte from read data
    if (status) {
        *resp = rdata[1];
    }

	return (status)? 0: -1;
}

int W25Q_IsBusy(bool* busy)
{
    *busy = w25qData.busy;
    if (w25qData.busy == false) {
        return 0;
    }
    
    uint8_t statusRegister1 = 0;
    int status = W25Q_ReadStatusRegister(1, &statusRegister1);
    if (status == 0) {
        w25qData.busy = (statusRegister1 & 0x1)? true: false;
        *busy = w25qData.busy;
    }
    return status;
}

int W25Q_ReadId(uint8_t* resp6)
{
	// Prepare command buffer
	uint8_t command[6] = {0x90, 0x00, 0x00, 0x00, 0x00, 0x00};

	// Send command to chip and read response
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    bool status = SPI1_WriteRead(command, 6, resp6, 6);
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

int W25Q_ReadJedecId(uint8_t* resp4)
{
	// Prepare command buffer
	uint8_t command[6] = {0x9F, 0x00, 0x00, 0x00};

	// Send command to chip and read response
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    bool status = SPI1_WriteRead(command, 4, resp4, 4);
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

int W25Q_WriteEnable(void)
{
	// Prepare command buffer
	uint8_t command[1] = {0x06};

	// Send command to chip and read response
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    bool status = SPI1_WriteRead(command, sizeof(command), NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

int W25Q_ReadData(uint32_t addr, void* data, uint32_t len)
{
    // Abort and return if W25Q is busy
    bool busy = true;
    int retval = W25Q_IsBusy(&busy);
    if ((retval != 0) || (busy == true)) {
        return -1;
    }
    
	// Prepare command buffer
	uint8_t command[4] = {0x03,
                          (addr >> 16) & 0xFF,
                          (addr >> 8) & 0xFF,
                          (addr >> 0) & 0xFF};
    
    // Select chip
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    
    // Send command and address to chip
    bool status = SPI1_WriteRead(command, sizeof(command), NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}
    
    // Receive data from chip
    status &= SPI1_WriteRead(NULL, 0, data, len);
	while (SPI1_IsTransmitterBusy()) {
	}
    
    // Deselect chip
	GPIO_RB7_SPI_NCS_FLASH_Set();
    
    return (status)? 0: -1;
}

int W25Q_PageProgram(uint32_t addr, void* data, uint32_t len)
{
    // Abort and return if W25Q is busy
    bool busy = true;
    int retval = W25Q_IsBusy(&busy);
    if ((retval != 0) || (busy == true)) {
        return -1;
    }
    
    // Mark W25Q busy
    w25qData.busy = true;
    
	// Prepare command buffer
	uint8_t command[4] = {0x02,
                          (addr >> 16) & 0xFF,
                          (addr >> 8) & 0xFF,
                          (addr >> 0) & 0xFF};
    
    // Select chip
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    
    // Send command and address to chip
    bool status = SPI1_WriteRead(command, sizeof(command), NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}
    
    // Send data to chip
    status &= SPI1_WriteRead(data, len, NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}
    
    // Deselect chip
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

int W25Q_SectorErase(uint32_t addr)
{
    // Abort and return if W25Q is busy
    bool busy = true;
    int retval = W25Q_IsBusy(&busy);
    if ((retval != 0) || (busy == true)) {
        return -1;
    }
    
    // Mark W25Q busy
    w25qData.busy = true;
    
	// Prepare command buffer
	uint8_t command[4] = {0x20,
                          (addr >> 16) & 0xFF,
                          (addr >> 8) & 0xFF,
                          (addr >> 0) & 0xFF};
    
    // Select chip
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    
    // Send command and address to chip
    bool status = SPI1_WriteRead(command, sizeof(command), NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}

    // Deselect chip
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

int W25Q_ChipErase(void)
{
    // Abort and return if W25Q is busy
    bool busy = true;
    int retval = W25Q_IsBusy(&busy);
    if ((retval != 0) || (busy == true)) {
        return -1;
    }
    
    // Mark W25Q busy
    w25qData.busy = true;
    
	// Prepare command buffer
	uint8_t command = 0xC7;
    
    // Select chip
	GPIO_RB7_SPI_NCS_FLASH_Clear();
    
    // Send command and address to chip
    bool status = SPI1_WriteRead(&command, sizeof(command), NULL, 0);
	while (SPI1_IsTransmitterBusy()) {
	}

    // Deselect chip
	GPIO_RB7_SPI_NCS_FLASH_Set();

	return (status)? 0: -1;
}

uint32_t W25Q_getPageIndex(uint32_t addr)
{
    return addr / W25Q_PAGE_SIZE;
}

uint32_t W25Q_getPageAddress(uint32_t addr)
{
    return addr * W25Q_PAGE_SIZE;
}

uint32_t W25Q_getSectorIndex(uint32_t addr)
{
    return addr / W25Q_SECTOR_SIZE;
}

uint32_t W25Q_getSectorAddress(uint32_t addr)
{
    return addr * W25Q_SECTOR_SIZE;
}
