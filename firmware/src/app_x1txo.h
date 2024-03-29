/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_x1txo.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_X1TXO_Initialize" and "APP_X1TXO_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_X1TXO_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_X1TXO_H
#define _APP_X1TXO_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_X1TXO_STATE_INIT=0,
    APP_X1TXO_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_X1TXO_STATES;

typedef enum
{
    X1TXO_MODE_INIT = 0,
    X1TXO_MODE_START,
    X1TXO_MODE_NORMAL,
    X1TXO_MODE_BIND,
} X1TXO_MODE;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

#define X1TXO_CHANNELS              6
#define X1TXO_START_PACKETS         17
// Packet interval: 21 to 23 ms works fine with an orange R615X receiver
// With <= 20 or >= 24 ms there are interruptions or no connection at all
#define X1TXO_DEFAULT_INTERVAL_MS   21
#define X1TXO_DEFAULT_PAUSE_US      0

typedef struct
{
    /* The application's current state */
    APP_X1TXO_STATES state;

    /* Additional data used by the application. */
    X1TXO_MODE mode;
    uint32_t ts32;
    uint32_t interval;
    uint32_t pause;
    uint16_t ch[X1TXO_CHANNELS];
    uint32_t modeStartPackets;
} APP_X1TXO_DATA;
extern APP_X1TXO_DATA appX1txoData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/
void APP_X1TXO_setMode(X1TXO_MODE mode);

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_X1TXO_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_X1TXO_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_X1TXO_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_X1TXO_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_X1TXO_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_X1TXO_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_X1TXO_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_X1TXO_H */

/*******************************************************************************
 End of File
 */

