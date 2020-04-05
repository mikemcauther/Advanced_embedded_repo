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

#include "s4527438_lib_cli.h"

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

    s4527438_lib_cli_init();
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

