/**
 * @file iis2iclx.c
 * @brief IIS2ICLX inclination sensor interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 05.12.2022
 */

#ifndef IIS2ICLX_H_
#define IIS2ICLX_H_

#include <stdint.h>

typedef struct {
	int id;
	int x;
	int y;
	float w;
	int allCalls;
	int newDataCalls;
    int consecutiveNewDataCalls;
	int newData;
} IIS2ICLX_DATA;

extern IIS2ICLX_DATA iis2iclxData;

void IIS2ICLX_Init(void);
int IIS2ICLX_Write(int address, uint8_t data);
int IIS2ICLX_Read(int address, uint8_t* data);
int IIS2ICLX_ReadDiagnostic(void);
int IIS2ICLX_ReadAcceleration(void);

#endif /* IIS2ICLX_H_ */
