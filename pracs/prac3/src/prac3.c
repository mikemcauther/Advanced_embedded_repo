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

/* Function Declarations ------------------------------------*/
static void prvPushButtonInterruptRoutine( void );
static bool button_value = false;
/* Private Variables ----------------------------------------*/

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
    
    if( button_value ) {
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Button pushed , <%d>\r\n",button_value);
    } else {
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Button not pushed , <%d>\r\n",button_value);
    }
}

void prvPushButtonInterruptRoutine( void )
{
	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    button_value = (button_value)?false:true;
    //s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"Button pushed by ISR\r\n");
	//xSemaphoreGiveFromISR( xLis3dhInterruptSemaphore, &xHigherPriorityTaskWoken );
	//portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/*-----------------------------------------------------------*/

