/**
 * @file sdp3x.c
 * @brief SDP31/32 differential pressure sensor interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 22.12.2022
 */

#include "sdp3x.h"
#include "crc.h"
#include "definitions.h"                // I2C

#define SDP3X_ADDR_A 0x21
#define SDP3X_ADDR_B 0x22
#define SDP3X_ADDR_C 0x23

#define SDP3X_CMD_START_DIFFP_AVG {0x36, 0x15}

SDP3X_DATA sdp3xData = {0};

bool SDP3X_IsInitialized(void)
{
    return sdp3xData.initialized;
}

void SDP3X_Initialize(void)
{
    if (sdp3xData.initialized) {
        return;
    }
    uint8_t wdata[] = SDP3X_CMD_START_DIFFP_AVG;   
    I2C1_Write(SDP3X_ADDR_A, wdata, sizeof(wdata));
    while (I2C1_IsBusy()) {
    }
    sdp3xData.initialized = true;
}

void SDP3X_Read(void)
{
    if (!sdp3xData.initialized) {
        return;
    }
    uint8_t rdata[9];   
    I2C1_Read(SDP3X_ADDR_A, rdata, sizeof(rdata));
    while (I2C1_IsBusy()) {
    }
    
    sdp3xData.dp = (rdata[0] << 8) | (rdata[1] << 0);
    sdp3xData.t  = (rdata[3] << 8) | (rdata[4] << 0);
    sdp3xData.sf = (rdata[6] << 8) | (rdata[7] << 0);    
    
    sdp3xData.rCrcDp = rdata[2];
    sdp3xData.rCrcT  = rdata[5];
    sdp3xData.rCrcSf = rdata[8];
    
    sdp3xData.lCrcDp = crcFast(&rdata[0], 2);
    sdp3xData.lCrcT  = crcFast(&rdata[3], 2);
    sdp3xData.lCrcSf = crcFast(&rdata[6], 2);
}