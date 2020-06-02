/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 */

/* Includes -------------------------------------------------*/

#include "FreeRTOS.h"
#include "board.h"

#include "leds.h"
#include "log.h"
#include "watchdog.h"
#include "argon.h"
#include "gpio.h"
#include "event_groups.h"
#include "esp_wifi.h"

#include "test_reporting.h"

#include "s4527438_hal_hci.h"
#include "s4527438_hal_wifi.h"

#include "s4527438_lib_cli.h"
#include "s4527438_lib_log.h"

#include "s4527438_os_led.h"
#include "s4527438_os_log.h"
#include "s4527438_os_hci.h"
#include "s4527438_os_wifi.h"
#include "s4527438_os_ble.h"

// cli related header
#include "s4527438_cli_led.h"
#include "s4527438_cli_log.h"
#include "s4527438_cli_timer.h"
#include "s4527438_cli_hci.h"
#include "s4527438_cli_wifi.h"
#include "s4527438_cli_ble.h"

/* Private Defines ------------------------------------------*/
// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/

/* Function Declarations ------------------------------------*/
void prvTask( void *pvParams );
/* Private Variables ----------------------------------------*/
EventGroupHandle_t   pxPushButtonState;
STATIC_TASK_STRUCTURES( pxWifi, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 2 );

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


    /* TODO : Add BLE callback API to detect distance / RSSI */
    /*
    xBluetoothComms.fnReceiveHandler = vUnifiedCommsRouterAdapter;
    vUnifiedCommsListen( &xBluetoothComms, COMMS_LISTEN_ON_FOREVER );
    */
    

    s4527438_hal_hci_init();
    //s4527438_hal_wifi_init();

    //s4527438_os_led_init();
    s4527438_os_log_init();
    s4527438_os_hci_init();
    s4527438_os_ble_init();
    //s4527438_os_wifi_init();

    s4527438_lib_cli_init();
    s4527438_lib_log_init();

    //s4527438_cli_led_init();
    s4527438_cli_log_init();
    s4527438_cli_time_init();
    s4527438_cli_hci_init();
    s4527438_cli_ble_init();
    s4527438_cli_wifi_init();

    STATIC_TASK_CREATE( pxWifi, prvTask, "WiFi", NULL );
}

void vApplicationTickCallback(uint32_t ulUptime) 
{
	UNUSED(ulUptime);
}

/* TODO : Re-write BLE route call back to detect distance and forward to PC BSU via wifi */
/*
void vUnifiedCommsRouterAdapter( xCommsInterface_t *           pxComms,
                               xUnifiedCommsIncomingRoute_t *pxCurrentRoute,
                               xUnifiedCommsMessage_t *      pxMessage )
{
	// If distance_mm > SDL	, SDL_violation_count++
	// if( SDL_violation_count >= 2 ) {
		// Warn user
	}

	// Forward packet to PC(BSU) via wifi by TDF report , need to add self identification info
}
*/
void prvTask( void *pvParams )
{
    uint8_t          pucMessage[] = "Help";
    eEspConnection_t eConnectionStatus;
    UNUSED( pvParams );

    vEspWifiOn( true );

    eEspTestAT();

    for ( ;; ) {

        eConnectionStatus = eGetConnectionStatus();

        switch ( eConnectionStatus ) {
            case WIFI_CONNECTED:
                vLedsOn( LEDS_BLUE );
                eEspSetClient( COMMAND_SET );
                eEspSendData( COMMAND_SET, pucMessage, 1 );
                break;
            case WIFI_DISCONNECTED:
                eLog( LOG_APPLICATION, LOG_APOCALYPSE, "DISCONNECTED try to reconnect");
                eEspForceReconnect( COMMAND_SET );
                vLedsOn( LEDS_RED );
                break;
            case WIFI_CONNECTING:
                break;
            default:
                vLedsOn( LEDS_GREEN );
        }

        vTaskDelay( pdMS_TO_TICKS( 250 ) );
        vLedsOff( LEDS_ALL );
        vTaskDelay( pdMS_TO_TICKS( 250 ) );
    }
}
/*-----------------------------------------------------------*/

