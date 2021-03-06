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
#include "xenon.h"
#include "gpio.h"
#include "event_groups.h"

#include "test_reporting.h"

#include "s4527438_hal_hci.h"
#include "s4527438_hal_ultraRanger.h"

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

#include "tdf.h"
#include "rtc.h"
/* Private Defines ------------------------------------------*/
// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/

/* Function Declarations ------------------------------------*/
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


    /* TODO : Add BLE callback API to detect distance / RSSI */
    /*
    xBluetoothComms.fnReceiveHandler = vUnifiedCommsRouterAdapter;
    vUnifiedCommsListen( &xBluetoothComms, COMMS_LISTEN_ON_FOREVER );
    */
    

    s4527438_hal_hci_init();
    s4527438_hal_ultraRanger_init();

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
	xTdfTime_t xTime;
    tdf_range_mm_t tdf_range_mm_obj = {0x00};

    // range
    uint32_t distance_mm = s4527438_hal_ultraRanger_read_distance();
    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[HCSR04 Event]: [%d]",distance_mm);

    tdf_range_mm_obj.range = distance_mm; 

	bRtcGetTdfTime( &xTime );
	eTdfAddMulti(BLE_LOG, TDF_3D_POSE, TDF_TIMESTAMP_NONE, &xTime, &tdf_range_mm_obj);
	eTdfFlushMulti(BLE_LOG);
}

/* TODO : Re-write BLE route call back to detect distance and forward to PC BSU via wifi */
#if 0
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
#endif
/*-----------------------------------------------------------*/

