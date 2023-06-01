/**
 * @file bm7x.h
 * @brief BM70/71 bluetooth low energy module interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 30.01.2023
 */

#ifndef _BM7X_H_
#define _BM7X_H_

#include <stdint.h>
#include "debug.h"

void BM7X_Initialize(void);
uint32_t BM7X_Read(void* buffer, uint32_t size);
uint32_t BM7X_Write(void* buffer, uint32_t size);
uint32_t BM7X_GetWriteBufferFree(void);
ERROR_T BM7X_GetError(void);

#endif /* _BM7X_H_ */
