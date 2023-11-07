/**
 * @file ringbuffer.c
 * @brief Ring buffer (FIFO) implementation
 * @author David Bender <d_bender@bluewin.ch>
 * @date 22.12.2022
 */

#include "ringbuffer.h"
#include <string.h>
#include <stdarg.h>
//#include <stdio.h>
#include "user_hal/printf.h"

/**
 * Initializes a ring buffer (FIFO).
 * @param[in]       rb          Pointer to ring buffer.
 * @param[in]       buffer      Data buffer assigned to this ring buffer.
 * @param[in]       size        Buffer size in bytes.
 */
void RB_Init(RINGBUFFER* rb, void* buffer, uint32_t size)
{
    rb->buffer = buffer;
    rb->size = size;
    rb->used = 0;
    rb->tail = 0;
}

/**
 * Reports the number of bytes that can be read from the buffer before it
 * becomes empty.
 * @param[in]       rb          Pointer to ring buffer.
 * @return                      Number of used bytes.
 */
uint32_t RB_GetUsed(RINGBUFFER* rb)
{
    return rb->used;
}

/**
 * Reports the number of bytes that can be written to the buffer before it
 * becomes full.
 * @param[in]       rb          Pointer to ring buffer.
 * @return                      Number of free bytes.
 */
uint32_t RB_GetFree(RINGBUFFER* rb)
{
    return rb->size - rb->used;
}

/**
 * Writes into the current contiguous range of a ring buffer.
 * Since there are two separate contiguous ranges, one before and one after the
 * head pointer wrap, two calls may be needed to write all data.
 * @param[in]       rb          Pointer to ring buffer.
 * @param[in]       data        Write data.
 * @param[in]       size        Write data length in bytes.
 * @return                      Number of bytes written to the ring buffer. 0 is
 *                              returned if the buffer is full.
 */
uint32_t RB_WriteContiguous(RINGBUFFER* rb, void* data, uint32_t size)
{
    uint32_t free = rb->size - rb->used;
    uint32_t head = rb->tail + rb->used;
    head -= (head >= rb->size)? rb->size: 0;
    uint32_t chunk = rb->size - head;
    chunk = (chunk > size)? size: chunk;
    chunk = (chunk > free)? free: chunk;
    if (chunk > 0) {
        memcpy((uint8_t*)rb->buffer + head, data, chunk);
        rb->used += chunk;
    }
    return chunk;
}

/**
 * Writes into a ring buffer.
 * @param[in]       rb          Pointer to ring buffer.
 * @param[in]       data        Write data.
 * @param[in]       size        Write data length in bytes.
 * @return                      Number of bytes written to the ring buffer.
 *                              Usually the same as size. 0 is returned if the
 *                              buffer is full. Less than size if the buffer
 *                              became full before writing all bytes.
 */
uint32_t RB_Write(RINGBUFFER* rb, void* data, uint32_t size)
{
    uint32_t done = 0;
    done += RB_WriteContiguous(rb, (uint8_t*)data + done, size - done);
    done += RB_WriteContiguous(rb, (uint8_t*)data + done, size - done);
    return done;
}

/**
 * Writes a NULL-terminated C string into a ring buffer.
 * @param[in]       rb          Pointer to ring buffer.
 * @param[in]       data        NULL-terminated C string.
 * @return                      Number of bytes written to the ring buffer.
 *                              The same as the string length or 0 if the buffer
 *                              is (almost) full.
 */
uint32_t RB_WriteString(RINGBUFFER* rb, char* data)
{
    uint32_t size = strlen(data);
    if (RB_GetFree(rb) < size) {
        return 0;
    }
    return RB_Write(rb, data, size);
}

/**
 * Prints a NULL-terminated C string into a ring buffer.
 * @param[in]       rb          Pointer to ring buffer.
 * @param[in]       format      Like printf().
 * @param[in]       ...         Like printf().
 * @return                      Number of bytes written to the ring buffer.
 *                              Usually the same as the string length.
 *                              0 is returned if the buffer is full. Less than
 *                              size if the buffer became full before writing
 *                              all bytes.
 */
uint32_t RB_Printf(RINGBUFFER* rb, const char* format, ...)
{
    char str[128];
    va_list vlist;
    va_start(vlist, format);
    vsnprintf_(str, sizeof(str), format, vlist);
    va_end(vlist);
    return RB_WriteString(rb, str);
}

/**
 * Reads from the current contiguous range of a ring buffer.
 * Since there are two separate contiguous ranges, one before and one after the
 * tail pointer wrap, two calls may be needed to read all data.
 * @param[in]       rb          Pointer to ring buffer.
 * @param[out]      data        Read data.
 * @param[in]       size        Read data length in bytes.
 * @return                      Number of bytes read from the ring buffer. 0 is
 *                              returned if the buffer is full.
 */
uint32_t RB_ReadContiguous(RINGBUFFER* rb, void* data, uint32_t size)
{
    uint32_t chunk = rb->size - rb->tail;
    chunk = (chunk > size)? size: chunk;
    chunk = (chunk > rb->used)? rb->used: chunk;
    if (chunk > 0) {
        memcpy(data, (uint8_t*)rb->buffer + rb->tail, chunk);
        rb->used -= chunk;
        rb->tail += chunk;
        rb->tail -= (rb->tail >= rb->size)? rb->size: 0;
    }
    return chunk;
}

/**
 * Reads from a ring buffer.
 * @param[in]       rb          Pointer to ring buffer.
 * @param[out]      data        Read data.
 * @param[in]       size        Read data length in bytes.
 * @return                      Number of bytes read from the ring buffer.
 *                              Usually the same as size. 0 is returned if the
 *                              buffer is empty. Less than size if the buffer
 *                              became empty before reading all bytes.
 */
uint32_t RB_Read(RINGBUFFER* rb, void* data, uint32_t size)
{
    uint32_t done = 0;
    done += RB_ReadContiguous(rb, (uint8_t*)data + done, size - done);
    done += RB_ReadContiguous(rb, (uint8_t*)data + done, size - done);
    return done;
}

/**
 * Reads from a ring buffer, until (and including) a delimiter character.
 * @param[in]       rb          Pointer to ring buffer.
 * @param[out]      data        Read data, NULL terminated string.
 * @param[in]       size        Read data length in bytes excluding termination
 *                              character.
 * @return                      Number of bytes read from the ring buffer,
 *                              including the delimiter. 0 if ring buffer is
 *                              empty or does not contain the delimiter.
 */
uint32_t RB_ReadLine(RINGBUFFER* rb, char* data, uint32_t size, char* delim)
{
    // Reserve character for NULL termination
    if (size == 0) {
        return 0;
    }
    size--;

    // Start with zero checked characters and current tail and usage
    uint32_t used = rb->used;
    uint32_t tail = rb->tail;
    uint32_t checked = 0;

    // Search for delimiter until reaching end of buffer
    while (1) {
        uint32_t chunk = rb->size - tail;
        chunk = (chunk > size)? size: chunk;
        chunk = (chunk > used)? used: chunk;

        if (chunk > 0) {
            // See if delimiter is found within current chunk of data
            //void* pDelim = memchr((uint8_t*)rb->buffer + tail, delim, chunk);
            void* pBrk = RB_mempbrk((uint8_t*)rb->buffer + tail, delim, chunk);
            if (pBrk != NULL) {
                // String size is 1 + delimiter position - tail position
                size = 1 + (uint8_t*)pBrk - ((uint8_t*)rb->buffer + tail);
                size += checked;

                // Size underflows if tail is before and delimiter is after wrap
                // -> make it overflow to undo underflow :-)
                size += (size >= rb->size)? rb->size: 0;

                // Read until and including delimiter
                RB_Read(rb, data, size);

                // Null terminate and return, reported size is excluding
                // termination character
                data[size] = '\0';
                return size;
            }

            // Move tail, update usage and remember number of checked characters
            used -= chunk;
            tail += chunk;
            tail -= (tail >= rb->size)? rb->size: 0;
            checked += chunk;

        } else {
            // If current chunk is empty there is no delimiter so return empty
            // string
            data[0] = '\0';
            return 0;
        }
    }
}

/**
 * Returns position of first break character in data buffer.
 * Basically a combination of memchr() and strpbrk(), so unlike strpbrk() it
 * does not only stop at a '\0' character but also at the specified size.
 * @param[in]       void        Pointer to data buffer.
 * @param[in]       str         Null-terminated array of break characters.
 * @param[in]       size        Data buffer size in bytes.
 * @return                      Pointer to first break character or NULL if no
 *                              break character is found.
 */
void* RB_mempbrk(void* mem, char* str, uint32_t size)
{
    char* mem_c = (char*)mem;
    uint32_t i;
    for (i = 0; (i < size) && (mem_c[i] != '\0'); i++) {
        uint32_t j;
        for (j = 0; str[j] != '\0'; j++) {
            if (mem_c[i] == str[j]) {
                return (void*)&mem_c[i];
            }
        }
    }
    return NULL;
}
