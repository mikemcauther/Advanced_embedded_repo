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
    PUSHBUTTON_ACTIVE_PUSHED = 0x02,
    PUSHBUTTON_ACTIVE = 0x04,
    PUSHBUTTON_DEACTIVE_PUSHED = 0x08,
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
    
    if( xEventGroupGetBits( pxPushButtonState ) & PUSHBUTTON_ACTIVE_PUSHED ) {
        /* Set up LSM6DSL register value */
        s4527438_os_hci_write_reg_cmd(1, 0x11, 0x60,1,1);

        s4527438_os_ble_tdf_continuous_mode(472,BLE_TDF_CONTINUOUS_START);
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Button pushed \r\n");

        xEventGroupClearBits( pxPushButtonState, PUSHBUTTON_ACTIVE_PUSHED );
        xEventGroupSetBits( pxPushButtonState, PUSHBUTTON_ACTIVE);
    }
    if( xEventGroupGetBits( pxPushButtonState ) & PUSHBUTTON_DEACTIVE_PUSHED ) {
        /* Set up LSM6DSL register value */
        s4527438_os_hci_write_reg_cmd(1, 0x11, 0x00,1,1);

        s4527438_os_ble_tdf_continuous_mode(472,BLE_TDF_CONTINUOUS_STOP);
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Button release \r\n");

        xEventGroupClearBits( pxPushButtonState, PUSHBUTTON_DEACTIVE_PUSHED );
        xEventGroupSetBits( pxPushButtonState, PUSHBUTTON_RELEASE);
    }
}

void prvPushButtonInterruptRoutine( void )
{
	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if( xEventGroupGetBitsFromISR( pxPushButtonState ) & PUSHBUTTON_RELEASE ) {
        xEventGroupClearBitsFromISR( pxPushButtonState, PUSHBUTTON_RELEASE );
        xEventGroupSetBitsFromISR( pxPushButtonState, PUSHBUTTON_ACTIVE_PUSHED ,&xHigherPriorityTaskWoken);
    }else if( xEventGroupGetBitsFromISR( pxPushButtonState ) & PUSHBUTTON_ACTIVE ) {
        xEventGroupClearBitsFromISR( pxPushButtonState, PUSHBUTTON_ACTIVE );
        xEventGroupSetBitsFromISR( pxPushButtonState, PUSHBUTTON_DEACTIVE_PUSHED ,&xHigherPriorityTaskWoken);
    }
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    //s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"Button pushed by ISR\r\n");
	//xSemaphoreGiveFromISR( xLis3dhInterruptSemaphore, &xHigherPriorityTaskWoken );
	//portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/*-----------------------------------------------------------*/

