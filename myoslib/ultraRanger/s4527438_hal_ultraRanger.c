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


/* Function Declarations ------------------------------------*/
static void init_timer(void);
#if 0
static void tmr_evt_handler(nrf_timer_event_t event_type, void * p_context);
#endif


/* Private Variables ----------------------------------------*/
static  xGpio_t xGpioTriggerPin = ULTRASONIC_TRIGGER_PIN;
static  xGpio_t xGpioEchoPin = ULTRASONIC_ECHO_PIN;

static NRF_TIMER_Type *timer_used = NRF_TIMER2;
static nrf_timer_cc_channel_t timer_channel = NRF_TIMER_CC_CHANNEL0;
//static nrfx_timer_t pxTimer = NRF_TIMER2;
//static uint8_t channel_used = 0;
#define CHANNEL_USED    NRF_TIMER_CC_CHANNEL0
//NRFX_TIMER_INSTANCE(0)
#if 0
static nrfx_timer_t pxTimer = { 
    .p_reg            = NRF_TIMER2,
    //.instance_id      = NRFX_CONCAT_3(NRFX_TIMER, id, _INST_IDX),
    .instance_id      = CHANNEL_USED,
    .cc_channel_count = TIMER2_CC_NUM,
};
static nrf_timer_cc_channel_t timer_channel = CHANNEL_USED;
#endif


/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

void s4527438_hal_ultraRanger_init(void) {
	/* TODO: Init GPIO */
	/* vGpioSetup(ultraRangerGPIO,,) */

	/* TODO: Init Timer */
    init_timer();
}

// init Timer2
static void init_timer(void)
{       
    ret_code_t ret;

#if 0
    nrfx_timer_config_t tmr_config = NRFX_TIMER_DEFAULT_CONFIG;
    tmr_config.frequency = NRF_TIMER_FREQ_1MHz;
    tmr_config.mode = NRF_TIMER_MODE_COUNTER;
    tmr_config.bit_width = NRF_TIMER_BIT_WIDTH_32;
    //ret = nrfx_timer_init(&pxTimer, &tmr_config, tmr_evt_handler);
    ret = nrf_drv_timer_init(&pxTimer, &tmr_config, tmr_evt_handler);
    if (ret != NRFX_SUCCESS)
    {
        return NRF_ERROR_INTERNAL;
    }

    nrfx_timer_compare(&pxTimer, timer_channel, UINT32_MAX, false);
#else
    nrf_timer_event_clear(timer_used,
                          nrf_timer_compare_event_get(CHANNEL_USED));
    //NRFX_IRQ_ENABLE(nrfx_get_irq_number(timer_used));

    nrf_timer_mode_set(timer_used, NRF_TIMER_MODE_COUNTER);
    nrf_timer_bit_width_set(timer_used, NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_frequency_set(timer_used, NRF_TIMER_FREQ_1MHz);
#endif

}

#if 0
static void tmr_evt_handler(nrf_timer_event_t event_type, void * p_context)
{
    UNUSED_PARAMETER(event_type);
    UNUSED_PARAMETER(p_context);
}
#endif

/* TODO */
uint32_t s4527438_hal_ultraRanger_read_distance(){
    uint32_t distance_mm = 0;
    uint32_t time_diff_us = 0;

    // set 0
    vGpioClear( xGpioTriggerPin );

    nrf_delay_us(20);

	vGpioSet( xGpioTriggerPin );

    nrf_delay_us(12);

    vGpioClear( xGpioTriggerPin );

    nrf_delay_us(20);


#if 0
    nrfx_timer_enable(&pxTimer);
#else
    nrf_timer_task_trigger(timer_used, NRF_TIMER_TASK_START);
#endif

    /* Start measure the responce period */
    while(!bGpioRead( xGpioEchoPin ));

#if 0
    nrfx_timer_clear(&pxTimer);
#else
    nrf_timer_task_trigger(timer_used, NRF_TIMER_TASK_CLEAR);
#endif

    while(bGpioRead( xGpioEchoPin ));

#if 0
    time_diff_us = nrfx_timer_capture_get(&pxTimer, timer_channel);
    time_diff_us = nrf_drv_timer_capture_get(timer_used, (nrf_timer_cc_channel_t) busy_state)
#else

    nrf_timer_task_trigger(timer_used,nrf_timer_capture_task_get(timer_channel));
    time_diff_us = nrf_timer_cc_read(timer_used, timer_channel);
#endif


	// Calculate distance (air speed = 343.2 m/s)
	distance_mm = (time_diff_us * 10000/ 1716);

	return distance_mm;
}

