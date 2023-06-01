/**
 * @file w25q.h
 * @brief W25Q128JV NOR flash memory interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 22.12.2022
 */

#define W25Q_SIZE 16777216
#define W25Q_PAGE_SIZE 256
#define W25Q_SECTOR_SIZE 4096
#define W25Q_SECTOR_COUNT 4096

#include "definitions.h"                // GPIO

typedef struct {
    bool busy;
} W25Q_DATA;

void W25Q_Init(void);
int W25Q_EnableReset(void);
int W25Q_Reset(void);
int W25Q_ReadStatusRegister(int reg, uint8_t* resp);
int W25Q_IsBusy(bool* busy);
int W25Q_ReadId(uint8_t* resp6);
int W25Q_ReadJedecId(uint8_t* resp4);
int W25Q_WriteEnable(void);
int W25Q_ReadData(uint32_t addr, void* data, uint32_t len);
int W25Q_PageProgram(uint32_t addr, void* data, uint32_t len);
int W25Q_SectorErase(uint32_t addr);
int W25Q_ChipErase(void);
uint32_t W25Q_getPageIndex(uint32_t addr);
uint32_t W25Q_getPageAddress(uint32_t addr);
uint32_t W25Q_getSectorIndex(uint32_t addr);
uint32_t W25Q_getSectorAddress(uint32_t addr);
