/**
 * @file sdp3x.h
 * @brief SDP31/32 differential pressure sensor interface
 * @author David Bender <d_bender@bluewin.ch>
 * @date 22.12.2022
 */

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool initialized;
    int16_t dp;         ///< Differential pressure
    int16_t t;          ///< Temperature in 1/200 °C
    int16_t sf;         ///< Differential pressure scaling factor
    uint8_t rCrcDp;     ///< Remote (received) CRC for differential pressure
    uint8_t rCrcT;      ///< Remote (received) CRC for temperature
    uint8_t rCrcSf;     ///< Remote (received) CRC for pressure scaling factor
    uint8_t lCrcDp;     ///< Local (calculated) CRC for differential pressure
    uint8_t lCrcT;      ///< Local (calculated) CRC for temperature
    uint8_t lCrcSf;     ///< Local (calculated) CRC for pressure scaling factor
} SDP3X_DATA;

extern SDP3X_DATA sdp3xData;

bool SDP3X_IsInitialized(void);
void SDP3X_Initialize(void);
void SDP3X_Read(void);
