/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_x1txo.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_x1txo.h"
#include "user_hal/debug.h"
#include "definitions.h"                // GPIO, UART

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

extern bool taskBusy;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_X1TXO_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_X1TXO_DATA appX1txoData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


void APP_X1TXO_setMode(X1TXO_MODE mode)
{
    // Nothing to do if already in desired target mode
    if (mode == appX1txoData.mode) {
        return;
    }
    
    // Transition from INIT to NORMAL requires intermediate mode START
    if ((mode == X1TXO_MODE_NORMAL) && (appX1txoData.mode == X1TXO_MODE_INIT)) {
        appX1txoData.modeStartPackets = 0;
        appX1txoData.mode = X1TXO_MODE_START;
        return;
    }
    
    // Transition from other modes to NORMAL is immediate
    if (mode == X1TXO_MODE_NORMAL) {
        appX1txoData.mode = X1TXO_MODE_NORMAL;
        return;
    }
    
    // Transition to BIND is immediate
    if (mode == X1TXO_MODE_BIND) {
        appX1txoData.mode = X1TXO_MODE_BIND;
        return;
    }
    
    // Transition to INIT is immediate
    if (mode == X1TXO_MODE_INIT) {
        appX1txoData.mode = X1TXO_MODE_INIT;
        return;
    }
}

/**
 * Generates and, if appropriate, sends an X1TXO packet.
 * Depending on the X1TXO mode, the packet sent contains RC channel data,
 * receiver bind data.
 * In idle modes, no data is sent.
 */
void APP_X1TXO_Send(void)
{
    uint8_t x1txoPacket[2 + 2 * X1TXO_CHANNELS];
    uint16_t* ch = appX1txoData.ch;
    
    // Reset packet data to all zeroes
    memset(x1txoPacket, 0, sizeof(x1txoPacket));
    
    if (appX1txoData.mode == X1TXO_MODE_START) {
        // Bytes 0 and 1: bind / normal operation, otherwise unknown
        x1txoPacket[0] = 0x18;
        x1txoPacket[1] = 0x00;
    
        // Bytes 2 to 13: all zeroes for INIT, so nothing to do here
        
        // Count number of start packets
        // Then switch to normal mode after sending all start packets
        appX1txoData.modeStartPackets++;
        if (appX1txoData.modeStartPackets >= X1TXO_START_PACKETS) {
            appX1txoData.mode = X1TXO_MODE_NORMAL;
            appX1txoData.modeStartPackets = 0;
        }
    } else if (appX1txoData.mode == X1TXO_MODE_NORMAL) {
        // Bytes 0 and 1: bind / normal operation, otherwise unknown
        x1txoPacket[0] = 0x18;
        x1txoPacket[1] = 0x00;
    
        // Bytes 2 to 13: channels
        // 6 most significant bits: channel number
        // 10 least significant bits (2 in low, 8 in high byte): channel value    
        int n;
        for (n = 0; n < X1TXO_CHANNELS; n++) {
            x1txoPacket[2 + 2 * n + 0] = (n << 2) | ((ch[n] >> 8) & 0x3);
            x1txoPacket[2 + 2 * n + 1] =            ((ch[n] >> 0) & 0xFF);
        }
    } else if (appX1txoData.mode == X1TXO_MODE_BIND) {
        // Bytes 0 and 1: bind / normal operation, otherwise unknown
        x1txoPacket[0] = 0x98;
        x1txoPacket[1] = 0x00;
    
        // Bytes 2 to 13: unknown, maybe failsafe channel settings?
        // We just send what the DX5E transmitter sends:
        // 0x9800000005FF09FF0DFF10AA14AA
        x1txoPacket[2]  = 0x00;
        x1txoPacket[3]  = 0x00;
        x1txoPacket[4]  = 0x05;
        x1txoPacket[5]  = 0xFF;
        x1txoPacket[6]  = 0x09;
        x1txoPacket[7]  = 0xFF;
        x1txoPacket[8]  = 0x0D;
        x1txoPacket[9]  = 0xFF;
        x1txoPacket[10] = 0x10;
        x1txoPacket[11] = 0xAA;
        x1txoPacket[12] = 0x14;
        x1txoPacket[13] = 0xAA;
    } else {
        // Default mode (INIT): do not send any packets yet
        return;
    }
    
    UART2_Write(x1txoPacket, sizeof(x1txoPacket));
    
    dprintf("APP_X1TXO_Send: <");
    int i;
    for (i = 0; i < sizeof(x1txoPacket); i++) {
        dprintf("%02X", x1txoPacket[i]);
    }            
    dprintf(">\n");
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_X1TXO_Initialize ( void )

  Remarks:
    See prototype in app_ble.h.
 */

void APP_X1TXO_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appX1txoData.state = APP_X1TXO_STATE_INIT;
    appX1txoData.modeStartPackets = 0;
}


/******************************************************************************
  Function:
    void APP_X1TXO_Tasks ( void )

  Remarks:
    See prototype in app_x1txo.h.
 */

void APP_X1TXO_Tasks ( void )
{
    uint32_t ts32 = _CP0_GET_COUNT();
    
    /* Check the application's current state. */
    switch (appX1txoData.state)
    {
        /* Application's initial state. */
        case APP_X1TXO_STATE_INIT:
        {
            appX1txoData.state = APP_X1TXO_STATE_SERVICE_TASKS;
            
            // Default packet transmit interval
            appX1txoData.interval =
                    X1TXO_DEFAULT_INTERVAL_MS * 0.001 * CORE_TIMER_FREQUENCY;
            
            // Initialize timestamp of last packet send to X1TXO
            appX1txoData.ts32 = ts32;
            break;
        }

        case APP_X1TXO_STATE_SERVICE_TASKS:
        {
            uint32_t ts32 = _CP0_GET_COUNT();
            
            // X1TXO TX port (RX for processor): print received data
            // Can also be used to listen to data transferred from Spektrum
            // controller to X1TXO module
            unsigned char buffer[128];
            uint32_t size = UART2_Read(buffer, sizeof(buffer) - 1);
            if (size > 0) {
                dprintf("APP_X1TXO_Tasks: read <");
                int i;
                for (i = 0; i < size; i++) {
                    dprintf("%02X", buffer[i]);
                }            
                dprintf(">\n");
            }
            
            // X1TXO RX port (TX for processor): periodically send packet
            if (ts32 - appX1txoData.ts32 > appX1txoData.interval) {
                APP_X1TXO_Send();
                // Remember timestamp of last packet sent to X1TXO
                appX1txoData.ts32 = ts32;                
            }
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
