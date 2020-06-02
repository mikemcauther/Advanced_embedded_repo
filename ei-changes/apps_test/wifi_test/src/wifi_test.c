/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 */

/* Includes -------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"

#include "bluetooth.h"
#include "gpio.h"
#include "leds.h"
#include "log.h"
#include "rtc.h"
#include "tdf.h"
#include "watchdog.h"

#include "argon.h"
#include "esp_wifi.h"
#include "uart.h"

/* Private Defines ------------------------------------------*/
// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/

/* Function Declarations ------------------------------------*/

void prvTask( void *pvParams );

/* Private Variables ----------------------------------------*/
STATIC_TASK_STRUCTURES( pxWifi, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 2 );

/*-----------------------------------------------------------*/

void vApplicationSetLogLevels( void )
{
	eLogSetLogLevel( LOG_APPLICATION, LOG_INFO );
	eLogSetLogLevel( LOG_GPS_DRIVER, LOG_INFO );
}

/*-----------------------------------------------------------*/

void vApplicationStartupCallback( void )
{
	WatchdogReboot_t *pxRebootData;
	vLedsOn( LEDS_ALL );
	/* Get Reboot reasons and clear */
	pxRebootData = xWatchdogRebootReason();
	if ( pxRebootData != NULL ) {
		vWatchdogPrintRebootReason( LOG_APPLICATION, LOG_INFO, pxRebootData );
	}

	STATIC_TASK_CREATE( pxWifi, prvTask, "WiFi", NULL );
	vLedsOff( LEDS_ALL );
}
/*-----------------------------------------------------------*/

// void vApplicationTickCallback( void )
void vApplicationTickCallback( uint32_t ulUptime )
{
	UNUSED( ulUptime );
	;
}

/*-----------------------------------------------------------*/

void prvTask( void *pvParams )
{
	eModuleError_t   eError		  = ERROR_NONE;
	uint8_t			 pucMessage[] = "Help";
	eEspConnection_t eConnectionStatus;

	UNUSED( pvParams );

	vEspWifiOn( false );

	eEspTestAT();

	for ( ;; ) {

		eConnectionStatus = eGetConnectionStatus();

		switch ( eConnectionStatus ) {
			case WIFI_CONNECTED:
				vLedsOn( LEDS_BLUE );
				if ( eError == ERROR_NONE ) {
					eEspSetClient( COMMAND_SET );
					eEspSendData( COMMAND_SET, pucMessage, 1 );
					eError = ERROR_GENERIC;
				}
				break;
			case WIFI_DISCONNECTED:
				vLedsOn( LEDS_RED );
				break;
			default:
				vLedsOn( LEDS_GREEN );
		}

		vTaskDelay( pdMS_TO_TICKS( 250 ) );
		vLedsOff( LEDS_ALL );
		vTaskDelay( pdMS_TO_TICKS( 250 ) );
	}
}
