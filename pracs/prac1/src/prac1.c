/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 */

/* Includes -------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "board.h"

#include "gpio.h"
#include "leds.h"
#include "log.h"
#include "rtc.h"
#include "watchdog.h"

#include "test_reporting.h"
#include "tiny_printf.h"

#include "device_nvm.h"
#include "unified_comms_serial.h"

/* Private Defines ------------------------------------------*/
// 1000 ms
#define LED_CHANGE_TIMER_PERIOD pdMS_TO_TICKS( 1000 )
#define LED_TARGET_ALL (LEDS_RED|LEDS_GREEN|LEDS_BLUE)
// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/

/* Function Declarations ------------------------------------*/
static void prvLEDTimerTestCallback( TimerHandle_t xExpiredTimer );
/* Private Variables ----------------------------------------*/
static TimerHandle_t xLEDTimer;
static eLEDs_t xLEDState = ~(LEDS_ALL);

void vCustomSerialHandler(xCommsInterface_t *pxComms,
                          xUnifiedCommsIncomingRoute_t *pxCurrentRoute,
                          xUnifiedCommsMessage_t *pxMessage);
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

    /* Set up LED: R-> G -> B */
    xLEDState = LEDS_RED;
    vLedsOff(LED_TARGET_ALL);
    vLedsOn(xLEDState);

    xLEDTimer = xTimerCreate( "LED change timer", LED_CHANGE_TIMER_PERIOD, pdTRUE, NULL, prvLEDTimerTestCallback );
    configASSERT( xLEDTimer );

    xTimerStart( xLEDTimer, portMAX_DELAY );

    /* Set up Serial input listen */
    /* Setup our serial receive handler */
    xSerialComms.fnReceiveHandler = vCustomSerialHandler;
    vUnifiedCommsListen(&xSerialComms, COMMS_LISTEN_ON_FOREVER);
}

/*-----------------------------------------------------------*/
static void prvLEDTimerTestCallback( TimerHandle_t xExpiredTimer )
{
    /* Remove compiler warnings about unused parameters. */
    ( void ) xExpiredTimer;

    vLedsOff(LED_TARGET_ALL);
    switch(xLEDState){
        case LEDS_RED:
            xLEDState = LEDS_GREEN;
            break;
        case LEDS_GREEN:
            xLEDState = LEDS_BLUE;
            break;
        case LEDS_BLUE:
            xLEDState = LEDS_RED;
            break;
        default:
            break;
    }
    vLedsSet(xLEDState);
}

void vCustomSerialHandler(xCommsInterface_t *pxComms,
                          xUnifiedCommsIncomingRoute_t *pxCurrentRoute,
                          xUnifiedCommsMessage_t *pxMessage)
{
    char pcLocalString[60] = {0};
    UNUSED(pxCurrentRoute);
    UNUSED(pxComms);
    /*
     * Copy the string to a local buffer so it can be NULL terminated properly
     * The %s format specifier does not respect provided lengths
     */
    pvMemcpy(pcLocalString, pxMessage->pucPayload, pxMessage->usPayloadLen);

    eLog(LOG_APPLICATION, LOG_INFO, "\r\nReceived PKT:\r\n");
    eLog(LOG_APPLICATION, LOG_INFO, "\t  Type: %02X\r\n", pxMessage->xPayloadType);
    eLog(LOG_APPLICATION, LOG_INFO, "\tString: %s\r\n\r\n", pcLocalString);

}

