/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble.c

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

#include "app_ble.h"
#include "user_hal/bm7x.h"
#include <stdint.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

extern bool taskBusy;
uint8_t appBleHostOutStreamBuffer[APP_BLE_STREAM_BUFFER_SIZE];
uint8_t appBleHostInStreamBuffer[APP_BLE_STREAM_BUFFER_SIZE];
RINGBUFFER* appBleRbHostOut = NULL;
RINGBUFFER* appBleRbHostIn = NULL;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_BLE_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_BLE_DATA appBleData;

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


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_BLE_Initialize ( void )

  Remarks:
    See prototype in app_ble.h.
 */

void APP_BLE_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appBleData.state = APP_BLE_STATE_INIT;

    /* Set up the host output stream buffer */
    RB_Init(&appBleData.rbHostOut,
            appBleHostOutStreamBuffer,
            sizeof(appBleHostOutStreamBuffer));
    appBleRbHostOut = &appBleData.rbHostOut;
    
    /* Set up the host input stream buffer */
    RB_Init(&appBleData.rbHostIn,
            appBleHostInStreamBuffer,
            sizeof(appBleHostInStreamBuffer));
    appBleRbHostIn = &appBleData.rbHostIn;
}


/******************************************************************************
  Function:
    void APP_BLE_Tasks ( void )

  Remarks:
    See prototype in app_ble.h.
 */

void APP_BLE_Tasks ( void )
{

    /* Check the application's current state. */
    switch (appBleData.state)
    {
        /* Application's initial state. */
        case APP_BLE_STATE_INIT:
        {
            BM7X_Initialize();
            appBleData.state = APP_BLE_STATE_SERVICE_TASKS;
            break;
        }

        case APP_BLE_STATE_SERVICE_TASKS:
        {
//            // Test mode: send incoming character + 1
//            char* buffer[128];
//            uint32_t size = BM7X_Read(buffer, sizeof(buffer) - 1);
//            buffer[size] = '\0';
//            dprintf("APP_BLE_Tasks: read <%s>\n", buffer);
//            int i;
//            for (i = 0; i < size; i++) {
//                buffer[i]++;
//            }
//            BM7X_Write(buffer, size);
            
            // Forward error messages
            gwsError |= BM7X_GetError();
            
            // Forward incoming data to BLE ring buffer
            uint8_t buffer[128];
            uint32_t size = RB_GetFree(&appBleData.rbHostOut);
            size = (size <= sizeof(buffer))? size : sizeof(buffer);
            size = BM7X_Read(buffer, size);
            RB_Write(&appBleData.rbHostOut, buffer, size);
            if (size > 0) {
                buffer[sizeof(buffer) - 1] = '\0';
                if (size < sizeof(buffer)) {
                    buffer[size] = '\0';
                }
                dprintf("APP_BLE_Tasks: read <%s>\n", buffer);
            }
            
            // Forward outgoing data from BLE ring buffer
            size = BM7X_GetWriteBufferFree();
            size = (size <= sizeof(buffer))? size : sizeof(buffer);
            size = RB_Read(&appBleData.rbHostIn, buffer, size);
            uint32_t sent = BM7X_Write(buffer, size);            
            if (sent != size) {
                gwsError |= BM7X_UART_TX_QUEUE_OVERFLOW;
            }
            if (sent > 0) {
                buffer[sizeof(buffer) - 1] = '\0';
                if (sent < sizeof(buffer)) {
                    buffer[sent] = '\0';
                }
                dprintf("APP_BLE_Tasks: write <%s>\n", buffer);
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
