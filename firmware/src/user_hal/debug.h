#include "ringbuffer.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    NO_ERROR = 0,
    W25Q_INV_ID = 1,
    W25Q_INV_JID = 2,
    W25Q_FULL = 4,
    BM7X_UART_TX_QUEUE_OVERFLOW = 8,
    BM7X_UART_RX_OVERRUN = 16,
    BM7X_UART_RX_FRAMING = 32,
    BM7X_UART_RX_PARITY = 64,
} ERROR_T;
extern ERROR_T gwsError;

bool Debug_IsEnabled(void);
void Debug_Disable(void);
void Debug_Enable(RINGBUFFER* rb);
RINGBUFFER* Debug_GetRb(void);
uint32_t dprintf(const char* format, ...);
