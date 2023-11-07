/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app_usb.c

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

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app_usb.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

//extern bool taskBusy;
uint8_t CACHE_ALIGN cdcReadBuffer[APP_USB_READ_BUFFER_SIZE];
uint8_t CACHE_ALIGN cdcWriteBuffer[APP_USB_READ_BUFFER_SIZE];
uint8_t appUsbHostOutStreamBuffer[APP_USB_STREAM_BUFFER_SIZE];
uint8_t appUsbHostInStreamBuffer[APP_USB_STREAM_BUFFER_SIZE];
RINGBUFFER* appUsbRbHostOut = NULL;
RINGBUFFER* appUsbRbHostIn = NULL;

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

APP_USB_DATA appUsbData;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************



/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/

USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler
(
    USB_DEVICE_CDC_INDEX index,
    USB_DEVICE_CDC_EVENT event,
    void * pData,
    uintptr_t userData
)
{
    APP_USB_DATA * appDataObject;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;
    USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE * eventDataRead;
    
    appDataObject = (APP_USB_DATA *)userData;

    switch(event)
    {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */

            USB_DEVICE_ControlSend(appDataObject->deviceHandle,
                    &appDataObject->getLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */

            USB_DEVICE_ControlReceive(appDataObject->deviceHandle,
                    &appDataObject->setLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *)pData;
            appDataObject->controlLineStateData.dtr = controlLineStateData->dtr;
            appDataObject->controlLineStateData.carrier = controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */

            appDataObject->breakData = ((USB_DEVICE_CDC_EVENT_DATA_SEND_BREAK *)pData)->breakDuration;
            
            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            
            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data*/
            eventDataRead = (USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *)pData;
            appDataObject->isReadComplete = true;
            appDataObject->numBytesRead = eventDataRead->length; 
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data */

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We don't
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            /* This means that the data write got completed. We can schedule
             * the next read. */

            appDataObject->isWriteComplete = true;
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/***********************************************
 * Application USB Device Layer Event Handler.
 ***********************************************/
void APP_USBDeviceEventHandler 
(
    USB_DEVICE_EVENT event, 
    void * eventData, 
    uintptr_t context 
)
{
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;

    switch(event)
    {
        case USB_DEVICE_EVENT_SOF:

            appUsbData.sofEventHasOccurred = true;
            
            break;

        case USB_DEVICE_EVENT_RESET:

            /* Update LED to show reset state */
            //LED_Off();

            appUsbData.isConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*)eventData;
            
            if ( configuredEventData->configurationValue == 1)
            {
                /* Update LED to show configured state */
                //LED_On();
                
                /* Register the CDC Device application event handler here.
                 * Note how the appData object pointer is passed as the
                 * user data */

                USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0, APP_USBDeviceCDCEventHandler, (uintptr_t)&appUsbData);

                /* Mark that the device is now configured */
                appUsbData.isConfigured = true;
            }
            
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(appUsbData.deviceHandle);
            
            /* Mark that the device is now attached */
            appUsbData.isAttached = true;
            
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(appUsbData.deviceHandle);
            
            /* Mark that the device is now detached */
            appUsbData.isAttached = false;
            
            //LED_Off();
            
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            //LED_Off();
            
            break;

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
            
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*****************************************************
 * This function is called in every step of the
 * application state machine.
 *****************************************************/

bool APP_StateReset(void)
{
    /* This function returns true if the device
     * was reset  */

    bool retVal;

    if(appUsbData.isConfigured == false)
    {
        appUsbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;
        appUsbData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appUsbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appUsbData.isReadComplete = true;
        appUsbData.numBytesRead = 0;
        appUsbData.isWriteComplete = true;
        retVal = true;
    }
    else
    {
        retVal = false;
    }

    return(retVal);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize(void)

  Remarks:
    See prototype in app.h.
 */

void APP_USB_Initialize(void)
{
    /* Place the App state machine in its initial state. */
    appUsbData.state = APP_STATE_INIT;
    
    /* Device Layer Handle  */
    appUsbData.deviceHandle = USB_DEVICE_HANDLE_INVALID ;

    /* Device configured status */
    appUsbData.isConfigured = false;

    /* Initial get line coding state */
    appUsbData.getLineCodingData.dwDTERate = 9600;
    appUsbData.getLineCodingData.bParityType = 0;
    appUsbData.getLineCodingData.bParityType = 0;
    appUsbData.getLineCodingData.bDataBits = 8;

    /* Read Transfer Handle */
    appUsbData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Write Transfer Handle */
    appUsbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Initialize the read complete flag and the pending read bytes count */
    appUsbData.isReadComplete = true;
    appUsbData.numBytesRead = 0;

    /*Initialize the write complete flag*/
    appUsbData.isWriteComplete = true;

    /* Reset other flags */
    appUsbData.sofEventHasOccurred = false;

    /* Set up the USB CDC read buffer */
    appUsbData.cdcReadBuffer = &cdcReadBuffer[0];

    /* Set up the USB CDC write buffer */
    appUsbData.cdcWriteBuffer = &cdcWriteBuffer[0];      
    
    /* Set up the host output stream buffer */
    RB_Init(&appUsbData.rbHostOut,
            appUsbHostOutStreamBuffer,
            sizeof(appUsbHostOutStreamBuffer));
    appUsbRbHostOut = &appUsbData.rbHostOut;
    
    /* Set up the host input stream buffer */
    RB_Init(&appUsbData.rbHostIn,
            appUsbHostInStreamBuffer,
            sizeof(appUsbHostInStreamBuffer));
    appUsbRbHostIn = &appUsbData.rbHostIn;
}


/******************************************************************************
  Function:
    void APP_Tasks(void)

  Remarks:
    See prototype in app.h.
 */

void APP_USB_Tasks(void)
{
//    /* Do not enter idle state while USB device is attached */
//    if (appUsbData.isAttached) {
//        taskBusy = true;
//    }
    
    /* Update the application state machine based
     * on the current state */
    //int i;
    
    switch(appUsbData.state)
    {
        case APP_STATE_INIT:

            /* Open the device layer */
            appUsbData.deviceHandle = USB_DEVICE_Open( USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE );

            if(appUsbData.deviceHandle != USB_DEVICE_HANDLE_INVALID)
            {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(appUsbData.deviceHandle, APP_USBDeviceEventHandler, 0);

                appUsbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;
            }
            else
            {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

        case APP_USB_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if(appUsbData.isConfigured)
            {
                /* If the device is configured then lets start reading */
                appUsbData.state = APP_USB_STATE_SCHEDULE_READ;
            }
            
            break;

        case APP_USB_STATE_SCHEDULE_READ:

            if(APP_StateReset())
            {
                break;
            }

            /* If a read is complete, then schedule a read
             * else wait for the current read to complete */

            appUsbData.state = APP_USB_STATE_WAIT_FOR_READ_COMPLETE;
            if ((appUsbData.isReadComplete == true) && (appUsbData.numBytesRead == 0))
            {
                appUsbData.isReadComplete = false;
                appUsbData.readTransferHandle =  USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

                USB_DEVICE_CDC_Read (USB_DEVICE_CDC_INDEX_0,
                        &appUsbData.readTransferHandle, appUsbData.cdcReadBuffer,
                        APP_USB_READ_BUFFER_SIZE);
                
                if(appUsbData.readTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID)
                {
                    appUsbData.state = APP_USB_STATE_ERROR;
                    break;
                }
            }

            break;

        case APP_USB_STATE_WAIT_FOR_READ_COMPLETE:

            if (APP_StateReset())
            {
                break;
            }

            /* Check if a character was received the isReadComplete flag gets 
             * updated in the CDC event handler. */
            if (appUsbData.isReadComplete)
            {
                if (appUsbData.numBytesRead < RB_GetFree(&appUsbData.rbHostOut))
                {
                    RB_Write(&appUsbData.rbHostOut, 
                         appUsbData.cdcReadBuffer,
                         appUsbData.numBytesRead);
                    appUsbData.numBytesRead = 0;
                }
            }
            
            /* Schedule write.
             * If the read is not complete yet we will wait for it as soon as
             * there are no pending writes.
             * See https://github.com/Microchip-MPLAB-Harmony/usb_apps_device/blob/master/apps/cdc_com_port_single/firmware/src/app.c
             * for example. */
            appUsbData.state = APP_USB_STATE_SCHEDULE_WRITE;
            
            break;


        case APP_USB_STATE_SCHEDULE_WRITE:

            if (APP_StateReset())
            {
                break;
            }
            
//            /* Skip write and go ahead with read if no data pending for
//             * write. */
//            if (RB_GetUsed(&appUsbData.rbHostIn) == 0)
//            {
//                appUsbData.state = APP_USB_STATE_SCHEDULE_READ;
//                break;
//            }



//            /* Else echo each received character by adding 1 */
//            for(i = 0; i < appUsbData.numBytesRead; i++)
//            {
//                if((appUsbData.cdcReadBuffer[i] != 0x0A) && (appUsbData.cdcReadBuffer[i] != 0x0D))
//                {
//                    appUsbData.cdcWriteBuffer[i] = appUsbData.cdcReadBuffer[i] + 1;
//                }
//            }
            
            /* Read data from host input stream buffer */
            uint32_t writeBytes = 0;
            writeBytes = RB_Read(&appUsbData.rbHostIn,
                    appUsbData.cdcWriteBuffer,
                    APP_USB_READ_BUFFER_SIZE);
            USB_DEVICE_CDC_TRANSFER_FLAGS txFlags
                    = USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE;
            if (RB_GetUsed(&appUsbData.rbHostIn) > 0) {
                txFlags = USB_DEVICE_CDC_TRANSFER_FLAGS_MORE_DATA_PENDING;
            }
            
            /* Skip write and go ahead with read if no data pending for
             * write. */
            if (writeBytes == 0)
            {
                appUsbData.state = APP_USB_STATE_SCHEDULE_READ;
                break;
            }
                    /* Setup the write */
            appUsbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            appUsbData.isWriteComplete = false;
            appUsbData.state = APP_USB_STATE_WAIT_FOR_WRITE_COMPLETE;            
            
            USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                    &appUsbData.writeTransferHandle,
                    appUsbData.cdcWriteBuffer, writeBytes,
                    txFlags);

            break;

        case APP_USB_STATE_WAIT_FOR_WRITE_COMPLETE:

            if(APP_StateReset())
            {
                break;
            }

            /* Check if a character was sent. The isWriteComplete
             * flag gets updated in the CDC event handler */

            if(appUsbData.isWriteComplete == true)
            {
                appUsbData.state = APP_USB_STATE_SCHEDULE_READ;
            }

            break;

        case APP_USB_STATE_ERROR:
        default:
            
            break;
    }
}

/*******************************************************************************
 End of File
 */

