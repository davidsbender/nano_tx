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


/* TODO:  Add any necessary local functions.
*/
void APP_X1TXO_Send(void)
{
    const int X1TXO_CHANNELS = 6;
    uint16_t ch[X1TXO_CHANNELS];
    uint8_t x1txoPacket[2 + 2 * X1TXO_CHANNELS];
    
    // Reset packet data
    memset(x1txoPacket, 0, sizeof(x1txoPacket));
    
    // Bytes 0 and 1: bind / normal operation, otherwise unknown
    x1txoPacket[0] = 0x18;
    x1txoPacket[1] = 0x00;
    
    // Bytes 2 to 13: channels
    // 6 most significant bits: channel number
    // 10 least significant bits (2 in upper, 8 in lower byte): channel value    
    int n;
    for (n = 0; n < X1TXO_CHANNELS; n++) {
        x1txoPacket[2 + 2 * n + 0] = (n << 2) | ((ch[n] >> 8) & 0x3);
        x1txoPacket[2 + 2 * n + 1] =            ((ch[n] >> 0) & 0xFF);
    }
    
    (void)x1txoPacket;
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
}


/******************************************************************************
  Function:
    void APP_X1TXO_Tasks ( void )

  Remarks:
    See prototype in app_ble.h.
 */

void APP_X1TXO_Tasks ( void )
{

    /* Check the application's current state. */
    switch (appX1txoData.state)
    {
        /* Application's initial state. */
        case APP_X1TXO_STATE_INIT:
        {
            appX1txoData.state = APP_X1TXO_STATE_SERVICE_TASKS;
            break;
        }

        case APP_X1TXO_STATE_SERVICE_TASKS:
        {
            // Test mode: send incoming character + 1
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
