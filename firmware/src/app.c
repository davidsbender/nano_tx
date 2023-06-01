/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

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

#include "app.h"
#include "definitions.h"                // GPIO
#include "user_hal/crc.h"
//#include "user_hal/iis2iclx.h"
#include "user_hal/lsm6dsv16x.h"
//#include "user_hal/lps22hh.h"
//#include "user_hal/sdp3x.h"
//#include "user_hal/w25q.h"
#include "user_hal/debug.h"
#include "user_hal/config.h"
#include <math.h>
#include <stdio.h>

#define TS_FREQ 1000
#define TS_TICKS (CORE_TIMER_FREQUENCY / TS_FREQ)

#if CORE_TIMER_FREQUENCY==36000000
    #define TS_120S_MASK    0xFFFFFFFF
    #define TS_8S_MASK      0x0FFFFFFF
    #define TS_2S_MASK      0x03FFFFFF
    #define TS_1S_MASK      0x01FFFFFF
    #define TS_30MS_MASK    0x000FFFFF
#endif

/// Debounce low time: >= 0.1 s
#define BUTTON_T_LOW_LONG (0.1 * CORE_TIMER_FREQUENCY)
    
/// Long button press high time: >= 2 s
#define BUTTON_T_HIGH_LONG (2 * CORE_TIMER_FREQUENCY)
    
/// Battery low shutdown threshold: 3.5 V
#define VBAT_LOW_SHUTDOWN 3.5

#define CAL_ADC_VBAT_GAIN 2.716619E-04f

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

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

void getParam(char** rem, char** param)
{
    int i;

    // Return with no parameter if buffer empty
    if ((*rem == NULL) || ((*rem)[0] == '\0')) {
        *param = NULL;
        return;
    }
    // Skip leading white spaces
    for (; ; (*rem)++) {
        if ((**rem != ' ') && (**rem != ',')) {
            break;
        }
    }
    // Find end of parameter
    *param = *rem;
    char *pend;
    for (i = 0; ; i++) {
        if ((*rem)[i] == ',') {
            (*rem)[i] = '\0';
            pend = (*rem) + i + 0;
            *rem = (*rem) + i + 1;
            break;
        } else  if (((*rem)[i] == '\0') || ((*rem)[i] == '\r')) {
            (*rem)[i] = '\0';
            pend = (*rem) + i + 0;
            *rem = NULL;
            break;
        }
    }
    // Skip trailing white spaces
    for (i = 1; pend - i > *param; i++) {
        if ((*(pend - i) != ' ') && (*(pend - i) != ',')) {
            break;
        }
        *(pend - i) = '\0';
    }
}

int isCmd(char* buf, char* cmd, char** rem)
{
    if (buf == NULL) {
        return 0;
    }
    int i;
    if (rem != NULL) {
        *rem = NULL;
    }
    for (i = 0; ; i++) {
        if ((buf[i] == ' ') && (cmd[i] == '\0')) {
            if (rem != NULL) {
                *rem = &buf[i+1];
            }
            return 1;
        } else  if ((buf[i] == '\0') && (cmd[i] == '\0')) {
            return 1;
        } else  if ((buf[i] == '\r') && (cmd[i] == '\0')) {
            return 1;
        } else if (buf[i] != cmd[i]) {
            return 0;
        }
    }
}

bool paramToInt(char* param, int* value)
{
    if (param == NULL) {
        return false;
    }
    char* pend;
    int valueStrtol = (int)strtol(param, &pend, 10);
    if (*pend == '\0') {
        *value = valueStrtol;
        return true;
    }
    return false;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    memset(&appData, 0, sizeof(APP_DATA));
    
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    /* Start core timer */
    CORETIMER_Start();
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            //Debug_Enable(appUsbRbHostIn);
            GPIO_RA10_LED_Set();
            crcInit();
            //IIS2ICLX_Init();
            //LPS22HH_Init();
            LSM6DSV16X_Init();
            AD1CON1SET = _AD1CON1_ASAM_MASK;
            //W25Q_Init();
            //Capture_Init();
            appData.state = APP_STATE_SERVICE_TASKS;
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {   
            // User button
            uint32_t button = (GPIO_RA7_BUTTON_Get())? 0 : 1;
            uint32_t ts32 = _CP0_GET_COUNT();

            // Identify edges and store edge timestamps
            if ((button != 0) && (appData.button == 0)) {
                // Rising edge
                dprintf("<BUTTON T HIGH STARTS>\n");
                appData.buttonTRise = ts32;
                appData.buttonEventPending |= appData.buttonEventArmed;
                appData.buttonEventArmed = false;
            } else if ((button == 0) && (appData.button != 0)) {
                // Falling edge
                dprintf("<BUTTON T LOW STARTS>\n");
                appData.buttonTFall = ts32;
            }
            appData.button = button;

            // Identify short event when low time reaches threshold
            uint32_t tLow = ts32 - appData.buttonTFall;
            if ((button == 0) && (tLow >= BUTTON_T_LOW_LONG)) {
                appData.buttonEventArmed = true;
                if (appData.buttonEventPending) {
                    dprintf("<SHORT BUTTON PRESS EVENT>\n");
                    appData.buttonEventPending = false;
                    if (!appData.shutdown) {
                        //(Capture_IsActive())? Capture_Stop(): Capture_Start();
                    }
                }
            }

            // Identify long event when high time reaches threshold
            uint32_t tHigh = ts32 - appData.buttonTRise;
            if ((button == 1) && (tHigh >= BUTTON_T_HIGH_LONG)) {
                if (appData.buttonEventPending) {
                    dprintf("<LONG BUTTON PRESS EVENT>\n");
                    appData.buttonEventPending = false;
                    appData.shutdown = true;
                    //Capture_Stop();
                }
            }
            
            // Shutdown mode: wait until capture is no longer pending, then
            // disable power supply
            if (appData.shutdown /*&& !Capture_IsPending()*/) {
                GPIO_RB1_LDOEN_Clear();
            }
            
            // Command line interface
            char* rem;
            char command[128];
            uint32_t link;
            for (link = 0; link < 2; link++) {
                // Select link
                RINGBUFFER* rbHostOut;
                RINGBUFFER* rbHostIn;
                if (link == 0) {
                    rbHostOut = appUsbRbHostOut;
                    rbHostIn = appUsbRbHostIn;
                } else {
                    rbHostOut = appBleRbHostOut;
                    rbHostIn = appBleRbHostIn;
                }
                
                // Handle link
                uint32_t len = RB_ReadLine(rbHostOut, command, sizeof(command), "\r\n");
                if (len == 0) {
                    continue;
                }
                
                char ans[256] = "";
                if (strstr(command, "HALLO") != NULL) {
                    sprintf(ans, "AU HOI!");
//                } else if (isCmd(command, "CAPT?", NULL)) {
//                    sprintf(ans, "%d%s", (Capture_IsActive())? 1: 0, config.ansEol);
//                } else if (isCmd(command, "CAPT 1", NULL)) {
//                    Capture_Start();
//                } else if (isCmd(command, "CAPT 0", NULL)) {
//                    Capture_Stop();
//                } else if (strstr(command, "DATA:DEL ALL") != NULL) {
//                    Capture_Erase();
//                } else if (isCmd(command, "DATA:READ?", &rem)) {
//                    char* param0;
//                    char* param1;
//                    int file = -1;
//                    getParam(&rem, &param0);
//                    getParam(&rem, &param1);
//                    if (isCmd(param0, "LAST", NULL)) {
//                        int len = Capture_PrintSample(ans, sizeof(ans),
//                                &appData.sample);
//                        sprintf(ans + len, "%s", config.ansEol);
//                    } else if (paramToInt(param0, &file)) {
//                        bool binary = (isCmd(param1, "BIN", NULL))? true: false;
//                        Capture_Read(file, binary, rbHostIn);
//                    } else {
//                        sprintf(ans, "%s", config.ansEol);
//                    }
                } else if (isCmd(command, "DIAG:ACC?", NULL)) {
                    //LSM6DSV16X_ReadDiagnostic();
                    sprintf(ans, "id: %d, t: %d, ax: %d, ay: %d, az: %d, gx: %d, gy: %d, gz: %d%s",
                            lsm6dsv16xData.id, lsm6dsv16xData.t,
                            lsm6dsv16xData.ax, lsm6dsv16xData.ay, lsm6dsv16xData.az,
                            lsm6dsv16xData.gx, lsm6dsv16xData.gy, lsm6dsv16xData.gz,
                            config.ansEol);

                } else if (isCmd(command, "DIAG:APP?", NULL)) {
                    sprintf(ans, "appData.maxCycleTime: %d, .lastCycleTime: %d%s",
                            appData.maxCycleTime, appData.lastCycleTime,
                            config.ansEol);
//                } else if (isCmd(command, "DIAG:APS?", NULL)) {
//                    LPS22HH_ReadDiagnostic();
//                    sprintf(ans, "id: %d, p: %u, t: %u%s",
//                            lps22hhData.id, lps22hhData.p, lps22hhData.t,
//                            config.ansEol);
                } else if (strstr(command, "DIAG:ADC?") != NULL) {
                    sprintf(ans, "BATMS %u (%f V)%s", appData.adcAd9Batms,
                            (float)appData.adcAd9Batms * CAL_ADC_VBAT_GAIN,
                            config.ansEol);
//                } else if (isCmd(command, "DIAG:CAPT?", NULL)) {
//                    sprintf(ans, "sample size: %d, captureData.lastErasedSector: %d, .dataHead: %d, .filesHead: %d (files: %d), .rbFull: %d%s",
//                            sizeof(CAPTURE_SAMPLE),
//                            captureData.lastErasedSector,
//                            captureData.dataHead,
//                            captureData.filesHead,
//                            captureData.filesHead / sizeof(FILE_OFFSET),
//                            captureData.rbFull,
//                            config.ansEol);
                } else if (isCmd(command, "DIAG:DEBUG", &rem)) {
                    char* param;
                    int value = 0;
                    getParam(&rem, &param);
                    if (paramToInt(param, &value) == false) {
                        // Invalid parameter
                    } else if (value) {
                        Debug_Enable(rbHostIn);
                        sprintf(ans, "Enable debug output%s", config.ansEol);
                    } else {
                        Debug_Disable();
                        sprintf(ans, "Disable debug output%s", config.ansEol);
                    }
//                } else if (isCmd(command, "DIAG:DPS?", NULL)) {
//                    sprintf(ans, "sdp3xData.dp: %d, .t: %d, .sf: %d"
//                            ", .rCrcDp: %d, .rCrcT: %d, .rCrcSf: %d"
//                            ", .lCrcDp: %d, .lCrcT: %d, .lCrcSf: %d%s",
//                    sdp3xData.dp, sdp3xData.t, sdp3xData.sf,
//                    sdp3xData.rCrcDp, sdp3xData.rCrcT, sdp3xData.rCrcSf,
//                    sdp3xData.lCrcDp, sdp3xData.lCrcT, sdp3xData.lCrcSf,
//                    config.ansEol);
                } else if (isCmd(command, "DIAG:ERR?", NULL)) {
                    sprintf(ans, "%d%s", gwsError, config.ansEol);
//                } else if (strstr(command, "DIAG:FLAS?") != NULL) {
//                    uint8_t resp6[6] = {0};
//                    W25Q_ReadId(resp6);
//                    uint8_t resp4[4] = {0};
//                    W25Q_ReadJedecId(resp4);
//                    sprintf(ans, "ID: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X, JEDEC ID: 0x%02X 0x%02X 0x%02X 0x%02X%s",
//                            resp6[0], resp6[1], resp6[2], resp6[3], resp6[4], resp6[5],
//                            resp4[0], resp4[1], resp4[2], resp4[3],
//                            config.ansEol);
//                } else if (strstr(command, "DIAG:FLAS:ERAS") != NULL) {
//                    W25Q_WriteEnable();
//                    W25Q_ChipErase();
//                } else if (strstr(command, "DIAG:FLAS:WEL") != NULL) {
//                    W25Q_WriteEnable();
//                } else if (isCmd(command, "DIAG:FLAS:STAT?", &rem)) {
//                    char* param;
//                    int reg = -1;
//                    getParam(&rem, &param);
//                    bool retval = paramToInt(param, &reg);
//                    if ((retval == true) && (reg >= 1) && (reg <= 3)) {
//                        uint8_t resp = 0;
//                        W25Q_ReadStatusRegister(reg, &resp);
//                        sprintf(ans, "W25Q status register %d: 0x%02X%s",
//                                reg, resp, config.ansEol);
//                    } else {
//                        sprintf(ans, "Invalid W25Q status register selection"
//                                " (must be 1...3)%s",
//                                config.ansEol);
//                    }
//                } else if (strstr(command, "DIAG:INCL?") != NULL) {
//                    sprintf(ans,
//                            "id: %d, ax: %d, ay: %d, calls: %d, updates: %d,"
//                            " consecutive updates: %d%s",
//                            iis2iclxData.id,
//                            iis2iclxData.x,
//                            iis2iclxData.y,
//                            iis2iclxData.allCalls,
//                            iis2iclxData.newDataCalls,
//                            iis2iclxData.consecutiveNewDataCalls,
//                            config.ansEol);
                } else if (strstr(command, "DIAG:RES") != NULL) {
                    //W25Q_Init();
                    //Capture_Init();
                } else if (isCmd(command, "TIME?", NULL)) {
                    sprintf(ans, "%llu%s", appData.ts64, config.ansEol);
                }

                // Return answer
                if (ans[0] != '\0') {
                    RB_Write(rbHostIn, ans, strlen(ans));
                }
            }
            
            // High resolution cycle time measurement
            uint32_t ts32d = (uint32_t)(appData.ts64 & 0x00000000FFFFFFFFllu);
            appData.lastCycleTime = ts32 - ts32d;
            if ((appData.ts64 > 0)
                    && (appData.lastCycleTime > appData.maxCycleTime)) {
                appData.maxCycleTime = appData.lastCycleTime;
            }
            
            // High resolution 64 bits timestamp
            if (ts32 < (uint32_t)(appData.ts64 & 0x00000000FFFFFFFFllu)) {
                appData.ts64 += 0x0000000100000000llu;
            };
            appData.ts64 = ((uint64_t)ts32)
                    | (appData.ts64 & 0xFFFFFFFF00000000llu);
            
            // LED flasher
            uint32_t tsLed = ts32 >> 18;
            //extern CORETIMER_OBJECT coreTmr;
            if (appData.shutdown) {
                GPIO_RA10_LED_Clear();
            } else if ((tsLed & 0x78) == 0x00) {
                GPIO_RA10_LED_Set();
            } else if (((tsLed & 0x78) == 0x10)/* && Capture_IsActive()*/) {
                GPIO_RA10_LED_Set();
            } else if (((tsLed & 0x78) == 0x10) && (gwsError != NO_ERROR)) {
                GPIO_RA10_LED_Set();
            } else if (((tsLed & 0x78) == 0x20) && (gwsError != NO_ERROR)) {
                GPIO_RA10_LED_Set();
            } else if (((tsLed & 0x78) == 0x30) && (gwsError != NO_ERROR)) {
                GPIO_RA10_LED_Set();
            } else {
                GPIO_RA10_LED_Clear();
            }

            // Read IIS2ICLX
            // The IIS2ICLX is our main data source and defines the sample rate,
            // whenever an IIS2ICLX sample is ready we read all other sources
            // too
//            iis2iclxData.newData = false;
//            if ((appData.counter & 0x1F) == 0) {
//                IIS2ICLX_ReadAcceleration();
//            }
//            
//            // Read LPS22H absolute pressure sensor
//            if (iis2iclxData.newData) {
//                LPS22HH_ReadPressure();
//            }
//            
//            // Read LSM6DSV16(B)X accelerometer/gyroscope
//            if (iis2iclxData.newData) {
            if ((appData.counter & 0x1F) == 0) {
                LSM6DSV16X_ReadTAG();
            }
            
//            // Read SDP3X differential pressure sensor
//            // The SDP3X has 25 ms power up time, thus delay initialization
//            if (!SDP3X_IsInitialized()) {
//                if ((ts32 & TS_30MS_MASK) < (ts32d & TS_30MS_MASK)) {
//                    SDP3X_Initialize();
//                }
//            } else if (iis2iclxData.newData) {
//                SDP3X_Read();
//            }
            
            // ADC battery voltage
            // (I don't like the generated plib_adc.c library, write own one)
            if (IFS0bits.AD1IF) {
                appData.adcAd9Batms = appData.adcAd9Batms * 15 / 16
                        + ADC_ResultGet(ADC_RESULT_BUFFER_0);
                IFS0CLR = _IFS0_AD1IF_MASK;
            }
            
            // Low battery voltage shutdown
            if ((ts32 & TS_2S_MASK) < (ts32d & TS_2S_MASK)) {
                float vBat = (float)appData.adcAd9Batms * CAL_ADC_VBAT_GAIN;
                if (vBat < VBAT_LOW_SHUTDOWN) {
                    appData.shutdown = true;
                    //Capture_Stop();
                    //RB_Printf(appUsbRbHostIn, "LOW BATTERY VOLTAGE\n");
                }
            }

//            // Collect a full sample
//            if (iis2iclxData.newData) {
//                if (Capture_FirstSample()) {
//                    appData.ts64FirstSample = appData.ts64;
//                }
//                uint64_t ts64 = appData.ts64 - appData.ts64FirstSample;
//                appData.sample.timestamp = (ts64 / TS_TICKS) & 0x7FFFFFFF;
//                appData.sample.inclX     = iis2iclxData.x;
//                appData.sample.inclY     = iis2iclxData.y;
//                appData.sample.accAX     = lsm6dsv16xData.ax;
//                appData.sample.accAY     = lsm6dsv16xData.ay;
//                appData.sample.accAZ     = lsm6dsv16xData.az;
//                appData.sample.accGX     = lsm6dsv16xData.gx;
//                appData.sample.accGY     = lsm6dsv16xData.gy;
//                appData.sample.accGZ     = lsm6dsv16xData.gz;
//                appData.sample.apsP      = lps22hhData.p;
//                appData.sample.dpsP      = sdp3xData.dp;
//                appData.sample.temp      = lps22hhData.t;
//                appData.sample.vBat      = appData.adcAd9Batms;
//                Capture_Append(&appData.sample);
//            }
//            
//            // Capture interface task (flash memory)
//            Capture_Task();

            appData.counter++;
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* Handle error in application's state machine. */
            break;
        }
    }
}

/*******************************************************************************
 End of File
 */
