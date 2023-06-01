/**
 * @file bm7x.c
 * @brief BM70/71 bluetooth low energy module interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 30.01.2023
 */

#include "bm7x.h"
#include "definitions.h"                // GPIO, UART

void BM7X_Initialize(void)
{
    // Wait for BM7X minimum reset pulse width
    CORETIMER_DelayMs(1);
    GPIO_RC2_BM70_HCI_RST_N_Set();
}

uint32_t BM7X_Read(void* buffer, uint32_t size)
{
    return UART1_Read(buffer, size);
}

uint32_t BM7X_Write(void* buffer, uint32_t size)
{
    return UART1_Write(buffer, size);
}

uint32_t BM7X_GetWriteBufferFree(void)
{
    return UART1_WriteFreeBufferCountGet();
}

ERROR_T BM7X_GetError(void)
{
    UART_ERROR eUart = UART1_ErrorGet();
    ERROR_T eGws = 0;
    eGws |= (eUart == UART_ERROR_OVERRUN)? BM7X_UART_RX_OVERRUN: 0;
    eGws |= (eUart == UART_ERROR_FRAMING)? BM7X_UART_RX_FRAMING: 0;
    eGws |= (eUart == UART_ERROR_PARITY)? BM7X_UART_RX_PARITY: 0;
    return eGws;
}
