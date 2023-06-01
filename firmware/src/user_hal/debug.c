#include "debug.h"
#include "ringbuffer.h"
#include "configuration.h"
#include "definitions.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

RINGBUFFER* debugRb = NULL;
ERROR_T gwsError = NO_ERROR;

bool Debug_IsEnabled(void)
{
    return (debugRb == NULL)? false: true;
}

void Debug_Disable(void)
{
    debugRb = NULL;
}

void Debug_Enable(RINGBUFFER* rb)
{
    debugRb = rb;
}

RINGBUFFER* Debug_GetRb(void)
{
    return debugRb;
}

/**
 * Prints a NULL-terminated C string into the active debug stream.
 * @param[in]       format      Like printf().
 * @param[in]       ...         Like printf().
 * @return                      Number of bytes written to the ring buffer.
 *                              Usually the same as the string length.
 *                              0 is returned if the buffer is full. Less than
 *                              size if the buffer became full before writing
 *                              all bytes.
 */
uint32_t dprintf(const char* format, ...)
{
    if (debugRb == NULL) {
        return 0;
    }
    
    while (RB_GetUsed(debugRb) > 0) {
        USB_DEVICE_Tasks(sysObj.usbDevObject0);
        APP_USB_Tasks();
    }
    
    char str[128];
    va_list vlist;
    va_start(vlist, format);
    vsnprintf(str, sizeof(str), format, vlist);
    va_end(vlist);
    return RB_WriteString(debugRb, str);
}