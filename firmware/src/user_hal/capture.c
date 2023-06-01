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

#include "capture.h"
#include "w25q.h"
#include "iis2iclx.h"
#include "debug.h"
#include "config.h"
#include <stdio.h>

CAPTURE_DATA captureData;

/**
 * Initializes the data capture abstraction layer for W25Q.
 */
void Capture_Init(void)
{
    // Initialize capture data
    dprintf("Capture_Init(): initialize capture data, sample size %d\n",
            sizeof(CAPTURE_SAMPLE));
    memset(&captureData, 0, sizeof(CAPTURE_DATA));
    
    // Initialize capture ring buffer
    dprintf("Capture_Init(): initialize ring buffer\n");
    RB_Init(&captureData.rb, captureData.buffer,
            CAPTURE_BUFFER_SIZE * sizeof(CAPTURE_SAMPLE));
    
    // Wait for W25Q power up initialization
    CORETIMER_DelayMs(5);
    
    // Read and verify W25Q manufacturer and JEDEC IDs
    uint8_t resp[6];
    W25Q_ReadId(resp);
    if ((resp[4] != 0xEF) || (resp[5] != 0x17)) {
        gwsError |= W25Q_INV_ID;
    }
    W25Q_ReadJedecId(resp);
    if ((resp[1] != 0xEF) || (resp[2] != 0x70) || (resp[3] != 0x18)) {
        gwsError |= W25Q_INV_JID;
    }
    
    // Search for data head (followed by first invalid timestamp) in W25Q NOR
    // flash
    // (linear search, binary tree is not possible as we erase only one sector
    //  after the data head to keep erase time low by avoiding full chip erase)
    dprintf("Capture_Init(): search data head...\n");
    int dataHead = W25Q_getSectorAddress(1);
    int32_t timestamp = -1;
    while (dataHead + sizeof(timestamp) <= W25Q_SIZE) {
        int retval = W25Q_ReadData(dataHead, &timestamp, sizeof(timestamp));
        if ((retval != 0) || (timestamp < 0)) {
            break;
        }
        dataHead += sizeof(CAPTURE_SAMPLE);
    }
    dprintf("Capture_Init(): ...done, found %d\n", dataHead);
    if (dataHead + sizeof(CAPTURE_SAMPLE) >= W25Q_SIZE) {
        gwsError |= W25Q_FULL;
    }
    
    // Search for next empty file in W25Q NOR flash files list
    // (linear search, binary tree would work perfectly fine here but for 4096
    //  entries only the run time benefit is negligible)
    dprintf("Capture_Init(): search files head...\n");
    int filesHead = W25Q_getSectorAddress(0);
    FILE_OFFSET fileOffset = -1;
    while (filesHead + sizeof(fileOffset) <= W25Q_getSectorAddress(1)) {
        int retval = W25Q_ReadData(filesHead, &fileOffset, sizeof(fileOffset));
        if ((retval != 0) || (fileOffset < 0)) {
            break;
        }
        filesHead += sizeof(FILE_OFFSET);
    }
    dprintf("Capture_Init(): ...done, found %d\n", filesHead);
    
    // Sector ahead of data head has been erased before
    // Sector containing data must not be erased
    captureData.lastErasedSector = W25Q_getSectorIndex(dataHead) + 1;
    dprintf("Capture_Init(): last erased sector %d\n",
            captureData.lastErasedSector);
    
    // Store found data and files list heads
    captureData.dataHead = dataHead;
    captureData.filesHead = filesHead;   
}

/**
 * Erases all capture data.
 * Only the files list sector and the first two data sectors are erased.
 * Higher sectors will be erased on-the fly when capturing data.
 */
void Capture_Erase(void)
{
    captureData.request = CAPTURE_REQUEST_FULL_ERASE;
}

/**
 * Queries if data capture is currently active.
 * If not, there may still be data pending for write to W25Q (not
 * safe for shutdown).
 * For shutdown safety call Capture_IsPending() instead.
 * @return                      Capture active state.
 */
bool Capture_IsActive(void)
{
    return captureData.active;
}

/**
 * Queries if data capture transactions with W25Q are currently pending.
 * Transactions may be pending for up to about 400 ms upon stopping the capture.
 * The system can be safely shutdown when no transactions are pending.
 * @return                      Capture active state.
 */
bool Capture_IsPending(void)
{
    return captureData.active || captureData.pending;
}

/**
 * Stops the data capture.
 */
void Capture_Stop(void)
{
    dprintf("Capture_Stop()\n");
    captureData.active = false;
    captureData.firstSample = false;
}

/**
 * Starts the data capture.
 */
void Capture_Start(void)
{
    dprintf("Capture_Start()\n");
    if (captureData.active == false) {
        captureData.active = true;
        captureData.pending = true;
        captureData.firstSample = true;
        captureData.request = CAPTURE_REQUEST_START;
    }
}

/**
 * Appends a sample to the active data capture. Calls are ignored while capture
 * is not active.
 * @param[in]       flashData   Pointer to sample.
 */
void Capture_Append(CAPTURE_SAMPLE* flashData)
{
    // Append data to ring buffer, capture task will pop off and write data
    if (captureData.active == true) {
        int written = RB_Write(&captureData.rb, flashData, sizeof(CAPTURE_SAMPLE));
        if (written < sizeof(CAPTURE_SAMPLE)) {
            captureData.rbFull = true;
            dprintf("Capture_Append(): sample buffer overflow\n");
        }
        captureData.firstSample = false;
    }
}

/**
 * Queries if the next sample appended will be the first sample in a capture.
 * @return                      First capture sample state.
 */
bool Capture_FirstSample(void)
{
    return captureData.firstSample;
}

/**
 * Prints a sample string into a buffer. 
 * @param[out]      buf         Buffer holding string.
 * @param[in]       size        Buffer size in bytes.
 * @param[in]       sample      Sample.
 * @return                      String length in bytes.
 */
int Capture_PrintSample(char* buf, int size, CAPTURE_SAMPLE* sample)
{
    int len = snprintf(buf, size,
            "%d,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%d,%hd,%hd,%hd;",
            sample->timestamp,
            sample->inclX,
            sample->inclY,
            sample->accAX,
            sample->accAY,
            sample->accAZ,
            sample->accGX,
            sample->accGY,
            sample->accGZ,
            sample->apsP,
            sample->dpsP,
            sample->temp,
            sample->vBat);
    return len;
}

/**
 * Reads a capture file to the host input buffer.
 * @param[in]       file        Capture file number.
 */
void Capture_Read(int32_t file, bool binary, RINGBUFFER* rb)
{
    if (captureData.readActive == false) {
        captureData.readActive = true;
        captureData.readBinary = binary;
        captureData.readFile = file;
        captureData.readAddr = 0;
        captureData.readRb = rb;
    }
}

/**
 * Capture task.
 */
void Capture_Task(void)
{
    if (captureData.state == CAPTURE_WRITE) {
        // Write state
        
        // First wait until W25Q is not busy
        bool busy = true;
        int retval = W25Q_IsBusy(&busy);
        if ((retval != 0) || (busy == true)) {
            dprintf("Capture_Task() state CAPTURE_WRITE_PAGE: W25Q is busy\n");
            return;
        }        
        
        // Make sure at least one sector ahead is erased
        // this makes operation power off safe
        uint32_t finalAddress =  captureData.dataHead
                + sizeof(CAPTURE_SAMPLE) - 1;
        uint32_t finalSector = W25Q_getSectorIndex(finalAddress);
        
        // If necessary send a sector erase request to the W25Q now
        if ((finalSector >= captureData.lastErasedSector)
                && (captureData.lastErasedSector + 1 < W25Q_SECTOR_COUNT)) {
            uint32_t eraseSector = captureData.lastErasedSector + 1;
            dprintf("Capture_Task() state CAPTURE_WRITE_PAGE: data head %d will write next sector %d, erase sector %d -> CAPTURE_ERASE_SECTOR\n", captureData.dataHead, finalSector, eraseSector);
            W25Q_WriteEnable();
            W25Q_SectorErase(W25Q_getSectorAddress(eraseSector));
            captureData.state = CAPTURE_ERASE_SECTOR;
            return;
        }

        // Write pending capture data
        // Get remaining page size and pop data off the ring buffer to fill it
        uint32_t free = W25Q_SIZE - captureData.dataHead;
        uint32_t nextPageIndex = W25Q_getPageIndex(captureData.dataHead) + 1;
        uint32_t nextPageAddress = W25Q_getPageAddress(nextPageIndex);
        uint32_t chunkSize = nextPageAddress - captureData.dataHead;
        chunkSize = (chunkSize > free)? free: chunkSize;
        if (free < sizeof(CAPTURE_SAMPLE)) {
            chunkSize = 0;
            gwsError |= W25Q_FULL;
        }
        
        const uint32_t maxChunkSize = W25Q_PAGE_SIZE;
        uint8_t chunkBuffer[maxChunkSize];
        chunkSize = RB_Read(&captureData.rb, chunkBuffer, chunkSize);
        
        // Nothing to write if no data available or not enough free memory left:
        // check for pending requests or handle read
        if ((chunkSize == 0) || (chunkSize > free)) {
            
            if (!captureData.active && captureData.pending) {
                // If capture is not active we are now done writing data to W25Q
                captureData.pending = false;
            } else if (captureData.request == CAPTURE_REQUEST_START) {
                // If start is requested open next file
                FILE_OFFSET fileOffset = captureData.dataHead;
                dprintf("Capture_Task() state CAPTURE_WRITE_PAGE: CAPTURE_REQUEST_START -> page program address %d, file offset %d\n", captureData.filesHead, fileOffset);
                W25Q_WriteEnable();
                W25Q_PageProgram(captureData.filesHead, &fileOffset, sizeof(FILE_OFFSET));
                captureData.filesHead += sizeof(FILE_OFFSET);
                captureData.request = CAPTURE_REQUEST_NONE;
                return;
            } else if (captureData.request == CAPTURE_REQUEST_FULL_ERASE) {
                // If erase is requested send a request to W25Q
                dprintf("Capture_Task() state CAPTURE_WRITE_PAGE: CAPTURE_REQUEST_ERASE -> CAPTURE_FULL_ERASE_FILES\n");
                W25Q_WriteEnable();
                W25Q_SectorErase(W25Q_getSectorAddress(0));
                captureData.lastErasedSector = 2;
                captureData.filesHead = W25Q_getSectorAddress(0);
                captureData.dataHead = W25Q_getSectorAddress(1);
                captureData.request = CAPTURE_REQUEST_NONE;
                captureData.state = CAPTURE_FULL_ERASE_FILES;
                return;
            }
            
            // Read file
            // Nothing to do if not active
            if (captureData.readActive == false) {
                return;
            }
            
            //dprintf("Capture_Task() read: active\n");
            
            FILE_OFFSET fileOffsetStart = captureData.readFileOffsetStart;
            fileOffsetStart += captureData.readAddr;
            FILE_OFFSET fileOffsetEnd = captureData.readFileOffsetEnd;
            
            if (captureData.readAddr == 0) {
                // Get start of file address
                // Stop if file does not exist or if file list cannot be read
                fileOffsetStart = -1;
                int32_t filesAddrStart =
                        (captureData.readFile + 0) * sizeof(fileOffsetStart);
                if (filesAddrStart < captureData.filesHead) {
                    int retval = W25Q_ReadData(filesAddrStart,
                            &fileOffsetStart, sizeof(fileOffsetStart));
                    if (retval != 0) {
                        if (RB_WriteString(captureData.readRb, config.ansEol)) {
                            captureData.readActive = false;
                        }
                        return;
                    }
                    fileOffsetStart += captureData.readAddr;
                } else {
                    if (RB_WriteString(captureData.readRb, config.ansEol)) {
                        captureData.readActive = false;
                    }
                    return;
                }

                // Get end of file address (start of next file - 1 if it exists,
                // current data head - 1 otherwise)
                // Stop if file list cannot be read
                fileOffsetEnd = -1;
                int32_t filesAddrEnd =
                            (captureData.readFile + 1) * sizeof(fileOffsetEnd);
                if (filesAddrEnd < captureData.filesHead) {
                    int retval = W25Q_ReadData(filesAddrEnd,
                            &fileOffsetEnd, sizeof(fileOffsetEnd));
                    if (retval != 0) {
                        if (RB_WriteString(captureData.readRb, config.ansEol)) {
                            captureData.readActive = false;
                        } 
                        return;
                    }
                    fileOffsetEnd -= 1;
                } else {
                    fileOffsetEnd = captureData.dataHead - 1;
                }
                
                captureData.readFileOffsetStart = fileOffsetStart;
                captureData.readFileOffsetEnd = fileOffsetEnd;
                
                // In binary mode send header
                if (captureData.readBinary) {
                    char lenStr[12];
                    char headerStr[15];
                    int fileSize = captureData.readFileOffsetEnd
                            - captureData.readFileOffsetStart + 1;
                    snprintf(lenStr, sizeof(lenStr), "%d", fileSize);
                    snprintf(headerStr, sizeof(headerStr), "#%d%s",
                            strlen(lenStr), lenStr);
                    if (RB_WriteString(captureData.readRb, headerStr) == 0) {
                        return;
                    }
                }
            }
            
            //dprintf("fileOffsetStart %d, fileOffsetEnd %d\n", fileOffsetStart, fileOffsetEnd);

            // Available chunk size must be a at least a full sample
            // Stop if available chunk size is less
            chunkSize = fileOffsetEnd - fileOffsetStart + 1;
            chunkSize = (fileOffsetEnd < fileOffsetStart)? 0: chunkSize;
            chunkSize = (chunkSize > maxChunkSize)? maxChunkSize: chunkSize;
            //chunkSize = (chunkSize > 32)? 32: chunkSize;
            chunkSize = (chunkSize / sizeof(CAPTURE_SAMPLE))
                    * sizeof(CAPTURE_SAMPLE);
            if (chunkSize < sizeof(CAPTURE_SAMPLE)) {
                if (RB_WriteString(captureData.readRb, config.ansEol)) {
                    captureData.readActive = false;
                } 
                return;
            }

            // Read data from W25Q
            // Stop if data cannot be read
            retval = W25Q_ReadData(fileOffsetStart, chunkBuffer, chunkSize);
            if (retval != 0) {
                if (RB_WriteString(captureData.readRb, config.ansEol)) {
                    captureData.readActive = false;
                } 
                return;
            }
            
            // Write data to host input buffer
            CAPTURE_SAMPLE* sample = (CAPTURE_SAMPLE*)chunkBuffer;
            
            //dprintf("Capture_Task() read: captureData.readAddr %d, chunkSize %d\n", captureData.readAddr, chunkSize);
            
            while (chunkSize >= sizeof(CAPTURE_SAMPLE)) {
//                // Normalize timestamp
//                if (captureData.readAddr == 0) {
//                    captureData.readTs = 0;
//                } else {
//                    int16_t deltaTs = sample->timestamp - captureData.sampleTs;
//                    deltaTs += (deltaTs >= 0)? 0: 0x8000;
//                    captureData.readTs += (uint32_t)deltaTs;
//                }
//                captureData.sampleTs = sample->timestamp;
//                
                // Write binary data or string
                if (captureData.readBinary) {
                    // Write to binary sample data to output ring buffer
                    if (RB_GetFree(captureData.readRb) < sizeof(CAPTURE_SAMPLE)) {
                        break;
                    }
                    //sample->timestamp = captureData.readTs & 0x7FFF;
                    RB_Write(captureData.readRb, sample, sizeof(CAPTURE_SAMPLE));
                } else {
                    // Convert sample to string
                    char str[128];
                    int len = Capture_PrintSample(str, sizeof(str), sample);

                    // Write to output ring buffer
                    if (RB_GetFree(captureData.readRb) < len) {
                        break;
                    }
                    RB_Write(captureData.readRb, str, len);
                }
                
                captureData.readAddr += sizeof(CAPTURE_SAMPLE);
                sample += 1;
                chunkSize -= sizeof(CAPTURE_SAMPLE);                
            }
            return;
        }
        
        // Issue a page program request now
        dprintf("Capture_Task() state CAPTURE_WRITE_PAGE: page program address %d, size %d\n", captureData.dataHead, chunkSize);
        
        W25Q_WriteEnable();
        W25Q_PageProgram(captureData.dataHead, chunkBuffer, chunkSize);
        captureData.dataHead += chunkSize;
        return;

    } else if ((captureData.state == CAPTURE_ERASE_SECTOR)
            || (captureData.state == CAPTURE_FULL_ERASE_FILES)
            || (captureData.state == CAPTURE_FULL_ERASE_DATA1)
            || (captureData.state == CAPTURE_FULL_ERASE_DATA2)) {
        // Erase sector state
        // Nothing to do while flash memory is busy completing pending request
        bool busy = true;
        int retval = W25Q_IsBusy(&busy);
        if ((retval != 0) || (busy == true)) {
            dprintf("Capture_Task() state CAPTURE_*_ERASE_*: busy\n");
            return;
        }
        
        // Enter next state when erase is complete
        if (captureData.state == CAPTURE_FULL_ERASE_FILES) {
            dprintf("Capture_Task() state CAPTURE_FULL_ERASE_FILES: complete -> CAPTURE_FULL_ERASE_DATA1\n");
            W25Q_WriteEnable();
            W25Q_SectorErase(W25Q_getSectorAddress(1));
            captureData.state = CAPTURE_FULL_ERASE_DATA1;
            return;
        } else if (captureData.state == CAPTURE_FULL_ERASE_DATA1) {
            dprintf("Capture_Task() state CAPTURE_FULL_ERASE_DATA1: complete -> CAPTURE_FULL_ERASE_DATA2\n");
            W25Q_WriteEnable();
            W25Q_SectorErase(W25Q_getSectorAddress(2));
            captureData.state = CAPTURE_FULL_ERASE_DATA2;
            return;
        } else if (captureData.state == CAPTURE_FULL_ERASE_DATA2) {
            dprintf("Capture_Task() state CAPTURE_FULL_ERASE_DATA2: complete -> CAPTURE_WRITE\n");
            dprintf("    data head %d, files head %d, last erased sector %d\n",
                    captureData.dataHead, captureData.filesHead, captureData.lastErasedSector);
            captureData.state = CAPTURE_WRITE;
            return;
        }
        dprintf("Capture_Task() state CAPTURE_ERASE_SECTOR: complete -> CAPTURE_WRITE\n");
        captureData.lastErasedSector += 1;
        captureData.state = CAPTURE_WRITE;
        return;
    }
}
