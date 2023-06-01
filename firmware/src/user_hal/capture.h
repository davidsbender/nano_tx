/**
 * @file capture.h
 * @brief Data capture abstraction layer for W25Q NOR flash memory
 * @author David Bender <d_bender@bluewin.ch>
 * @date 26.12.2022
 * 
 * Sector 0: contains 1024 file start addresses:
 *           sector 0 + 0...3: file 0 start address
 *           sector 0 + 4...7: file 1 start address
 *           sector 0 + n*4*(0...3): file n start address
 *           sector 0 + 4092...4095: file 1023 start address
 * 
 * Sectors 1...4095: contain capture data
 */

#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include "ringbuffer.h"
#include <stdbool.h>

#define CAPTURE_BUFFER_SIZE 10  // Sector erase takes up to 400 ms
                                // Samples arrive every 80 ms
                                // -> Capture buffer must be >= 5 samples
#define CAPTURE_FILES (W25Q_SECTOR_COUNT / sizeof(FILE_OFFSET))

typedef int32_t FILE_OFFSET;

typedef struct {
    int32_t timestamp;  ///< Position 0...3, >= 0: timestamp / -1 empty marker
    int16_t inclX;      ///< Position 4...5
    int16_t inclY;      ///< Position 6...7
    int16_t accAX;      ///< Position 8...5
    int16_t accAY;      ///< Position 10...11
    int16_t accAZ;      ///< Position 12...13
    int16_t accGX;      ///< Position 14...15
    int16_t accGY;      ///< Position 16...17
    int16_t accGZ;      ///< Position 18...19
    int32_t apsP;       ///< Position 20...23
    int16_t dpsP;       ///< Position 24...25
    int16_t temp;       ///< Position 26...27
    int16_t vBat;       ///< Position 28...29
                        // Position 30...31 is used for alignment
} CAPTURE_SAMPLE;

typedef enum {
    CAPTURE_REQUEST_NONE = 0,
    CAPTURE_REQUEST_START,
    CAPTURE_REQUEST_FULL_ERASE,
} CAPTURE_REQUEST;

typedef enum {
    CAPTURE_WRITE = 0,          ///< Default state: write mode
    CAPTURE_ERASE_SECTOR,       ///< Erasing a sector
    CAPTURE_FULL_ERASE_FILES,   ///< Erase all data, erasing files sector
    CAPTURE_FULL_ERASE_DATA1,   ///< Erase all data, erasing sector of data head
    CAPTURE_FULL_ERASE_DATA2,   ///< Erase all data, erasing sector ahead of data head
} CAPTURE_STATE;

typedef struct {
    bool active;
    bool pending;
    int32_t lastErasedSector;
    int32_t dataHead;
    int32_t filesHead;
    bool readActive;
    bool readBinary;
    int32_t readFile;
    int32_t readAddr;
    FILE_OFFSET readFileOffsetStart;
    FILE_OFFSET readFileOffsetEnd;
    RINGBUFFER* readRb;
    uint32_t readTs;
    int16_t sampleTs;
    CAPTURE_REQUEST request;
    CAPTURE_STATE state;
    CAPTURE_SAMPLE buffer[CAPTURE_BUFFER_SIZE];
    RINGBUFFER rb;
    bool rbFull;
    bool firstSample;
} CAPTURE_DATA;

extern CAPTURE_DATA captureData;

void Capture_Init(void);
void Capture_Erase(void);
bool Capture_IsActive(void);
bool Capture_IsPending(void);
void Capture_Stop(void);
void Capture_Start(void);
void Capture_Append(CAPTURE_SAMPLE* flashData);
bool Capture_FirstSample(void);
void Capture_Read(int32_t file, bool binary, RINGBUFFER* rb);
int Capture_PrintSample(char* buf, int size, CAPTURE_SAMPLE* sample);
void Capture_Task(void);

#endif /* _CAPTURE_H_ */
