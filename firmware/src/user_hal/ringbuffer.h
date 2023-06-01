/**
 * @file ringbuffer.h
 * @brief Ring buffer (FIFO) implementation
 * @author David Bender <d_bender@bluewin.ch>
 * @date 22.12.2022
 */

#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    void* buffer;
    uint32_t size;
    uint32_t used;
    uint32_t tail;
} RINGBUFFER;

void RB_Init(RINGBUFFER* rb, void* buffer, uint32_t size);
uint32_t RB_GetUsed(RINGBUFFER* rb);
uint32_t RB_GetFree(RINGBUFFER* rb);
uint32_t RB_Write(RINGBUFFER* rb, void* data, uint32_t size);
uint32_t RB_WriteString(RINGBUFFER* rb, char* data);
uint32_t RB_Printf(RINGBUFFER* rb, const char* format, ...);
uint32_t RB_Read(RINGBUFFER* rb, void* data, uint32_t size);
uint32_t RB_ReadLine(RINGBUFFER* rb, char* data, uint32_t size, char* delim);
void* RB_mempbrk(void* mem, char* str, uint32_t size);

#endif /* _RINGBUFFER_H_ */
