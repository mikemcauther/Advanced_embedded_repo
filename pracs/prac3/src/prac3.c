/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 */

/* Includes -------------------------------------------------*/

#include "FreeRTOS.h"
#include "board.h"

#include "log.h"
#include "watchdog.h"
#include "argon.h"
#include "gpio.h"
#include "event_groups.h"

#include "test_reporting.h"

#include "s4527438_hal_hci.h"

#include "s4527438_lib_cli.h"
#include "s4527438_lib_log.h"

#include "s4527438_os_led.h"
#include "s4527438_os_log.h"
#include "s4527438_os_hci.h"
#include "s4527438_os_ble.h"

// cli related header
#include "s4527438_cli_led.h"
#include "s4527438_cli_log.h"
#include "s4527438_cli_timer.h"
#include "s4527438_cli_hci.h"
#include "s4527438_cli_ble.h"

/* Private Defines ------------------------------------------*/
// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/
typedef enum ePushButtonState_t {
    PUSHBUTTON_RELEASE  = 0x01,
    PUSHBUTTON_PUSHED = 0x02
} ePushButtonState_t;

/* Function Declarations ------------------------------------*/
static void prvPushButtonInterruptRoutine( void );
/* Private Variables ----------------------------------------*/
EventGroupHandle_t   pxPushButtonState;

/*-----------------------------------------------------------*/

void vApplicationSetLogLevels( void )
{
	eLogSetLogLevel( LOG_RESULT, LOG_INFO );
	eLogSetLogLevel( LOG_APPLICATION, LOG_INFO );
}

/*-----------------------------------------------------------*/

void vApplicationStartupCallback( void )
{
	WatchdogReboot_t *pxRebootData;
	uint32_t		  ulRebootCount;
	char			  pcRebootCount[6] = { 0 };

	UNUSED(pcRebootCount);
	UNUSED(ulRebootCount);

	/* Get Reboot reason */
	pxRebootData = xWatchdogRebootReason();
	if ( pxRebootData != NULL ) {
		vWatchdogPrintRebootReason( LOG_APPLICATION, LOG_INFO, pxRebootData );
	}

	/* Enable the button */
	vGpioSetup( BUTTON_1, GPIO_INPUTPULL, GPIO_INPUTPULL_PULLUP );
	eGpioConfigureInterrupt( BUTTON_1, true, GPIO_INTERRUPT_RISING_EDGE, prvPushButtonInterruptRoutine );
    pxPushButtonState  = xEventGroupCreate();
    xEventGroupSetBits( pxPushButtonState, PUSHBUTTON_RELEASE );

    s4527438_hal_hci_init();

    s4527438_os_led_init();
    s4527438_os_log_init();
    s4527438_os_hci_init();
    s4527438_os_ble_init();

    s4527438_lib_cli_init();
    s4527438_lib_log_init();

    s4527438_cli_led_init();
    s4527438_cli_log_init();
    s4527438_cli_time_init();
    s4527438_cli_hci_init();
    s4527438_cli_ble_init();
}

void vApplicationTickCallback(uint32_t ulUptime) 
{
	UNUSED(ulUptime);
    
    if( xEventGroupGetBits( pxPushButtonState ) & PUSHBUTTON_PUSHED ) {
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Button pushed \r\n");
    }
    if( xEventGroupGetBits( pxPushButtonState ) & PUSHBUTTON_RELEASE ) {
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Button not pushed \r\n");
    }
}

void prvPushButtonInterruptRoutine( void )
{
	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if( xEventGroupGetBitsFromISR( pxPushButtonState ) & PUSHBUTTON_RELEASE ) {
        xEventGroupClearBitsFromISR( pxPushButtonState, PUSHBUTTON_RELEASE );
        xEventGroupSetBitsFromISR( pxPushButtonState, PUSHBUTTON_PUSHED ,&xHigherPriorityTaskWoken);
    }else if( xEventGroupGetBitsFromISR( pxPushButtonState ) & PUSHBUTTON_PUSHED ) {
        xEventGroupClearBitsFromISR( pxPushButtonState, PUSHBUTTON_PUSHED );
        xEventGroupSetBitsFromISR( pxPushButtonState, PUSHBUTTON_RELEASE ,&xHigherPriorityTaskWoken);
    }
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    //s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"Button pushed by ISR\r\n");
	//xSemaphoreGiveFromISR( xLis3dhInterruptSemaphore, &xHigherPriorityTaskWoken );
	//portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/*-----------------------------------------------------------*/

