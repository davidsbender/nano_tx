/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for X1TXN_TXD pin ***/
#define X1TXN_TXD_Get()               ((PORTB >> 9) & 0x1)
#define X1TXN_TXD_PIN                  GPIO_PIN_RB9

/*** Macros for GPIO_RC6_LDOEN pin ***/
#define GPIO_RC6_LDOEN_Set()               (LATCSET = (1<<6))
#define GPIO_RC6_LDOEN_Clear()             (LATCCLR = (1<<6))
#define GPIO_RC6_LDOEN_Toggle()            (LATCINV= (1<<6))
#define GPIO_RC6_LDOEN_OutputEnable()      (TRISCCLR = (1<<6))
#define GPIO_RC6_LDOEN_InputEnable()       (TRISCSET = (1<<6))
#define GPIO_RC6_LDOEN_Get()               ((PORTC >> 6) & 0x1)
#define GPIO_RC6_LDOEN_PIN                  GPIO_PIN_RC6

/*** Macros for X1TXN_RXD pin ***/
#define X1TXN_RXD_Get()               ((PORTC >> 8) & 0x1)
#define X1TXN_RXD_PIN                  GPIO_PIN_RC8

/*** Macros for TRIM_Y pin ***/
#define TRIM_Y_Get()               ((PORTB >> 13) & 0x1)
#define TRIM_Y_PIN                  GPIO_PIN_RB13

/*** Macros for GPIO_RA10_LED pin ***/
#define GPIO_RA10_LED_Set()               (LATASET = (1<<10))
#define GPIO_RA10_LED_Clear()             (LATACLR = (1<<10))
#define GPIO_RA10_LED_Toggle()            (LATAINV= (1<<10))
#define GPIO_RA10_LED_OutputEnable()      (TRISACLR = (1<<10))
#define GPIO_RA10_LED_InputEnable()       (TRISASET = (1<<10))
#define GPIO_RA10_LED_Get()               ((PORTA >> 10) & 0x1)
#define GPIO_RA10_LED_PIN                  GPIO_PIN_RA10

/*** Macros for GPIO_RA7_BUTTON pin ***/
#define GPIO_RA7_BUTTON_Set()               (LATASET = (1<<7))
#define GPIO_RA7_BUTTON_Clear()             (LATACLR = (1<<7))
#define GPIO_RA7_BUTTON_Toggle()            (LATAINV= (1<<7))
#define GPIO_RA7_BUTTON_OutputEnable()      (TRISACLR = (1<<7))
#define GPIO_RA7_BUTTON_InputEnable()       (TRISASET = (1<<7))
#define GPIO_RA7_BUTTON_Get()               ((PORTA >> 7) & 0x1)
#define GPIO_RA7_BUTTON_PIN                  GPIO_PIN_RA7

/*** Macros for BATMS pin ***/
#define BATMS_Get()               ((PORTB >> 15) & 0x1)
#define BATMS_PIN                  GPIO_PIN_RB15

/*** Macros for GIMBAL_X pin ***/
#define GIMBAL_X_Get()               ((PORTA >> 0) & 0x1)
#define GIMBAL_X_PIN                  GPIO_PIN_RA0

/*** Macros for GIMBAL_Y pin ***/
#define GIMBAL_Y_Get()               ((PORTA >> 1) & 0x1)
#define GIMBAL_Y_PIN                  GPIO_PIN_RA1

/*** Macros for GIMBAL_Z pin ***/
#define GIMBAL_Z_Get()               ((PORTB >> 0) & 0x1)
#define GIMBAL_Z_PIN                  GPIO_PIN_RB0

/*** Macros for TRIM_X pin ***/
#define TRIM_X_Get()               ((PORTB >> 1) & 0x1)
#define TRIM_X_PIN                  GPIO_PIN_RB1

/*** Macros for TRIM_Z pin ***/
#define TRIM_Z_Get()               ((PORTC >> 1) & 0x1)
#define TRIM_Z_PIN                  GPIO_PIN_RC1

/*** Macros for GPIO_RC2_BM70_HCI_RST_N pin ***/
#define GPIO_RC2_BM70_HCI_RST_N_Set()               (LATCSET = (1<<2))
#define GPIO_RC2_BM70_HCI_RST_N_Clear()             (LATCCLR = (1<<2))
#define GPIO_RC2_BM70_HCI_RST_N_Toggle()            (LATCINV= (1<<2))
#define GPIO_RC2_BM70_HCI_RST_N_OutputEnable()      (TRISCCLR = (1<<2))
#define GPIO_RC2_BM70_HCI_RST_N_InputEnable()       (TRISCSET = (1<<2))
#define GPIO_RC2_BM70_HCI_RST_N_Get()               ((PORTC >> 2) & 0x1)
#define GPIO_RC2_BM70_HCI_RST_N_PIN                  GPIO_PIN_RC2

/*** Macros for _input_only_RA4_ pin ***/
#define _input_only_RA4__Get()               ((PORTA >> 4) & 0x1)
#define _input_only_RA4__PIN                  GPIO_PIN_RA4

/*** Macros for GPIO_RC4_SPI_NCS_ACC pin ***/
#define GPIO_RC4_SPI_NCS_ACC_Set()               (LATCSET = (1<<4))
#define GPIO_RC4_SPI_NCS_ACC_Clear()             (LATCCLR = (1<<4))
#define GPIO_RC4_SPI_NCS_ACC_Toggle()            (LATCINV= (1<<4))
#define GPIO_RC4_SPI_NCS_ACC_OutputEnable()      (TRISCCLR = (1<<4))
#define GPIO_RC4_SPI_NCS_ACC_InputEnable()       (TRISCSET = (1<<4))
#define GPIO_RC4_SPI_NCS_ACC_Get()               ((PORTC >> 4) & 0x1)
#define GPIO_RC4_SPI_NCS_ACC_PIN                  GPIO_PIN_RC4

/*** Macros for GPIO_RC5_BM70_P2_0 pin ***/
#define GPIO_RC5_BM70_P2_0_Set()               (LATCSET = (1<<5))
#define GPIO_RC5_BM70_P2_0_Clear()             (LATCCLR = (1<<5))
#define GPIO_RC5_BM70_P2_0_Toggle()            (LATCINV= (1<<5))
#define GPIO_RC5_BM70_P2_0_OutputEnable()      (TRISCCLR = (1<<5))
#define GPIO_RC5_BM70_P2_0_InputEnable()       (TRISCSET = (1<<5))
#define GPIO_RC5_BM70_P2_0_Get()               ((PORTC >> 5) & 0x1)
#define GPIO_RC5_BM70_P2_0_PIN                  GPIO_PIN_RC5


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/

typedef enum
{
    GPIO_PORT_A = 0,
    GPIO_PORT_B = 1,
    GPIO_PORT_C = 2,
} GPIO_PORT;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/

typedef enum
{
    GPIO_PIN_RA0 = 0,
    GPIO_PIN_RA1 = 1,
    GPIO_PIN_RA2 = 2,
    GPIO_PIN_RA3 = 3,
    GPIO_PIN_RA4 = 4,
    GPIO_PIN_RA7 = 7,
    GPIO_PIN_RA8 = 8,
    GPIO_PIN_RA9 = 9,
    GPIO_PIN_RA10 = 10,
    GPIO_PIN_RB0 = 16,
    GPIO_PIN_RB1 = 17,
    GPIO_PIN_RB2 = 18,
    GPIO_PIN_RB3 = 19,
    GPIO_PIN_RB4 = 20,
    GPIO_PIN_RB5 = 21,
    GPIO_PIN_RB7 = 23,
    GPIO_PIN_RB8 = 24,
    GPIO_PIN_RB9 = 25,
    GPIO_PIN_RB13 = 29,
    GPIO_PIN_RB14 = 30,
    GPIO_PIN_RB15 = 31,
    GPIO_PIN_RC0 = 32,
    GPIO_PIN_RC1 = 33,
    GPIO_PIN_RC2 = 34,
    GPIO_PIN_RC3 = 35,
    GPIO_PIN_RC4 = 36,
    GPIO_PIN_RC5 = 37,
    GPIO_PIN_RC6 = 38,
    GPIO_PIN_RC7 = 39,
    GPIO_PIN_RC8 = 40,
    GPIO_PIN_RC9 = 41,

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
    GPIO_PIN_NONE = -1

} GPIO_PIN;


void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
    GPIO_PortWrite((GPIO_PORT)(pin>>4), (uint32_t)(0x1) << (pin & 0xF), (uint32_t)(value) << (pin & 0xF));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return (bool)(((GPIO_PortRead((GPIO_PORT)(pin>>4))) >> (pin & 0xF)) & 0x1);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (bool)((GPIO_PortLatchRead((GPIO_PORT)(pin>>4)) >> (pin & 0xF)) & 0x1);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
