/**
 **************************************************************
 * @file myoslib/s4527438_hal_hci.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib HCI driver
 * REFERENCE:
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 ***************************************************************
 */
/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "uart.h"
#include "s4527438_hci_packet.h"
#include "s4527438_hal_hci.h"
#include "s4527438_lib_log.h"


/* Function Declarations ------------------------------------*/


/* Private Variables ----------------------------------------*/


/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* TODO: */
/*
xGpio_t xGpio = ultraRangerGPIO = ;
start_time = 0
end_time = 0
*/
void s4527438_hal_hci_init(void) {
	/* TODO: Init GPIO */
	/* vGpioSetup(ultraRangerGPIO,,) */

	/* TODO : GPIO interrupt */
	/*eError = eGpioConfigureInterrupt( ultraRangerGPIO, true, GPIO_INTERRUPT_FALLING_EDGE, prvLis3dhInterruptRoutine );*/
	/* TODO: Init Timer */
}
/* TODO */
/*
int s4527438_hal_ultraRanger_read_distance(){
	// Start time
	start_time = get_time(); 	
	// Send 
	vGpioWrite( xGpio_t xGpio, 1 );

	// Wait
	xSemaphoreTake(s4527438Semaphore_ultraRanger_Event, port_MaxDealy );
	
	// Calculate time
	time_diff = end_time - start_time

	// NOTE : Need to convert tick to ms
	freq = System_core_freq / (Q + 1)
	tick_period = 1000 / freq 

	time_diff_ms = time_diff * tick_period

	// Calculate distance (air speed = 343.2 m/s)
	distance_mm = (time_diff_ms / 2) * (343.2 * 1000 / 1000 )

	return distance_mm
}
*/

/* TODO: Get end time */
/*
irq_handler(){
	end_time = get_time(); 	

	// Wake up
	xSemaphoreGive(s4527438Semaphore_ultraRanger_Event);
}
*/

