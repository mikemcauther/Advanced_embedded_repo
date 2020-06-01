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
#include "s4527438_lib_log.h"
#include <nrfx.h>
#include "nrfx_timer.h"
#include "nrf_gpio.h"
#include "xenon.h"
#include "nrf_drv_timer.h"
#include "nrf_delay.h"


/* Function Declarations ------------------------------------*/
static void init_timer(void);

/* Private Variables ----------------------------------------*/
static  xGpio_t xGpioTriggerPin = ULTRASONIC_TRIGGER_PIN;
static  xGpio_t xGpioEchoPin = ULTRASONIC_ECHO_PIN;
static volatile uint32_t tCount = 0;
static volatile float countToUs = 1;

static NRF_TIMER_Type *timer_used = NRF_TIMER1;
static nrf_timer_cc_channel_t timer_channel = NRF_TIMER_CC_CHANNEL1;
#define CHANNEL_USED    NRF_TIMER_CC_CHANNEL1
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

void s4527438_hal_ultraRanger_init(void) {
	vGpioSetup( xGpioTriggerPin, GPIO_PUSHPULL, GPIO_PUSHPULL_LOW);
    vGpioSetup( xGpioEchoPin, GPIO_INPUTPULL, GPIO_INPUTPULL_PULLDOWN );
    init_timer();
}

// init Timer2
static void init_timer(void)
{       
    nrf_timer_mode_set(timer_used, NRF_TIMER_MODE_TIMER);
    nrf_timer_task_trigger(timer_used, NRF_TIMER_TASK_CLEAR);
    nrf_timer_bit_width_set(timer_used, NRF_TIMER_BIT_WIDTH_16);

    uint8_t prescaler = NRF_TIMER_FREQ_16MHz;
    nrf_timer_frequency_set(timer_used, NRF_TIMER_FREQ_16MHz);

     /* Timeout Delay = 500ms = 16000000 * 0.5  */
    uint16_t comp1 = 500;
	nrf_timer_cc_write( timer_used, CHANNEL_USED, comp1 ); 
    countToUs = 0.0625*comp1*(1 << prescaler);

    nrf_timer_event_clear(timer_used,
                          nrf_timer_compare_event_get(CHANNEL_USED));

    nrf_timer_int_enable(timer_used, nrf_timer_compare_int_get(CHANNEL_USED));
	//nrf_timer_shorts_enable( timer_used, NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK );
	nrf_timer_shorts_enable( timer_used, NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK );
    NRFX_IRQ_ENABLE(nrfx_get_irq_number(timer_used));
    nrf_timer_task_trigger(timer_used, NRF_TIMER_TASK_START);
}

uint32_t s4527438_hal_ultraRanger_read_distance(){
    uint32_t distance_mm = 0;
    //uint32_t time_diff_us = 0;
    float time_diff_us = 0;

    vGpioClear( xGpioTriggerPin );

    nrf_delay_us(20);

	vGpioSet( xGpioTriggerPin );

    nrf_delay_us(12);

    vGpioClear( xGpioTriggerPin );

    nrf_delay_us(20);

    /* Start measure the responce period */
    while(!bGpioRead( xGpioEchoPin ));

    tCount = 0;

    while(bGpioRead( xGpioEchoPin ));

    // calculate duration in us
    float duration = countToUs*tCount;
 
	// Calculate distance (air speed = 343.2 m/s)
    // dist = duration * speed of sound * 1/2
    // dist in cm = duration in us * 10^-6 * 340.29 * 100 * 1/2
    float distance = duration*0.017;
  
    // check value
    if(distance >= 4000.0) {

        return 0;
    }

    time_diff_us = duration;

    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[HCSR04 Event]: time_diff_us = [%f]",time_diff_us);

	distance_mm = distance * 10;

	return distance_mm;
}

//void nrfx_timer_2_irq_handler(void)
void TIMER1_IRQHandler(void)
{
    nrf_timer_event_t event = nrf_timer_compare_event_get(timer_channel);
    nrf_timer_int_mask_t int_mask = nrf_timer_compare_int_get(timer_channel);

    if (nrf_timer_event_check(timer_used, event) &&
        nrf_timer_int_enable_check(timer_used, int_mask))
    {
        nrf_timer_event_clear(timer_used, event);
        tCount++;
    }
}

