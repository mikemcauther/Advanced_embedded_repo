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

#include "unified_comms_bluetooth.h"

/* Private Defines ------------------------------------------*/
// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/

/* Function Declarations ------------------------------------*/
void prvTask( void *pvParams );
void vCustomBluetoothHandler( const uint8_t *pucAddress, eBluetoothAddressType_t eAddressType, 
							int8_t cRssi, bool bConnectable, 
							uint8_t *pucData, uint8_t ucDataLen );

/* Private Variables ----------------------------------------*/
EventGroupHandle_t   pxPushButtonState;
STATIC_TASK_STRUCTURES( pxWifi, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 2 );

static uint8_t global_send_buffer[512] = {0};
static uint8_t global_send_buffer_len = 0;
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

	/* Setup our Bluetooth receive handler */
	vUnifiedCommsBluetoothCustomHandler(vCustomBluetoothHandler);
	vUnifiedCommsListen(&xBluetoothComms, COMMS_LISTEN_ON_FOREVER);
}

void vApplicationTickCallback(uint32_t ulUptime) 
{
	UNUSED(ulUptime);
}

void vCustomBluetoothHandler( const uint8_t *pucAddress, eBluetoothAddressType_t eAddressType, 
							int8_t cRssi, bool bConnectable, 
							uint8_t *pucData, uint8_t ucDataLen )
{
	UNUSED(eAddressType);
	UNUSED(bConnectable);
	UNUSED(pucData);
	UNUSED(ucDataLen);

	/* Limit printed devices based on RSSI */
	if (cRssi < -60) {
		return;
	}

    xLogBuilder_t logBuilder;
    eLogBuilderStart( &logBuilder, LOG_APPLICATION );
    //eLogBuilderPush( &logBuilder, LOG_INFO, "%:6R,%3d,dBm = % *A\r\n", pucAddress, cRssi, ucDataLen, pucData );
    eLogBuilderPush( &logBuilder, LOG_INFO, "%:6R,%3d", pucAddress, cRssi );

	//eLog(LOG_APPLICATION, LOG_INFO, "buffer = <%s>\r\n", logBuilder.pcString );
    pvMemcpy(global_send_buffer,logBuilder.pcString,logBuilder.ulIndex);
    global_send_buffer_len = logBuilder.ulIndex;
    global_send_buffer[global_send_buffer_len++] = '\r';
    global_send_buffer[global_send_buffer_len++] = '\n';
    global_send_buffer[global_send_buffer_len++] = '\0';

    eLogBuilderFinish( &logBuilder );

	//eLog(LOG_APPLICATION, LOG_INFO, "%:6R %3d dBm = % *A\r\n", pucAddress, cRssi, ucDataLen, pucData );
}

void prvTask( void *pvParams )
{
    uint8_t          pucMessage[] = "Help";
    eEspConnection_t eConnectionStatus;
    UNUSED( pvParams );
    UNUSED( pucMessage );

    vEspWifiOn( true );

    eEspTestAT();

    for ( ;; ) {

        eConnectionStatus = eGetConnectionStatus();

        switch ( eConnectionStatus ) {
            case WIFI_CONNECTED:
                vLedsOn( LEDS_BLUE );
                eEspSetClient( COMMAND_SET );
                eEspSendData( COMMAND_SET, global_send_buffer, global_send_buffer_len );
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

