/** 
 **************************************************************
 * @file mylib/s4527438_os_led.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib led driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "leds.h"

#include "s4527438_os_led.h"

/* Private typedef -----------------------------------------------------------*/
// 1000 ms
#define LED_CHANGE_TIMER_PERIOD pdMS_TO_TICKS( 1000 )
#define LED_TARGET_ALL (LEDS_RED|LEDS_GREEN|LEDS_BLUE)

struct MyOsLibLEDMessage {    /* Message consists of sequence number and payload string */
    LED_Cmd_Enum   led_cmd;
    LED_Color_Enum led_color;
};
/* Private define ------------------------------------------------------------*/
#define mainRADIO_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )
#define mainRADIO_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )

#define QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK 100

#define EVENT_NUM                           10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TaskHandle_t xTaskLEDOsHandle;
static SemaphoreHandle_t s4527438QueueLEDPacketSend = NULL;
static QueueSetHandle_t xQueueSet;

static TimerHandle_t xLEDTimer;
static eLEDs_t xLEDState = LEDS_NONE;
static eLEDs_t xMaskLEDs = LEDS_NONE;

/* Private function prototypes -----------------------------------------------*/
static void LEDTask( void );
static void prvLEDTimerCallback( TimerHandle_t xExpiredTimer );

void s4527438_os_led_init(void) {

    s4527438QueueLEDPacketSend = xSemaphoreCreateBinary();
    xQueueSet = xQueueCreateSet(EVENT_NUM);
    xQueueAddToSet(s4527438QueueLEDPacketSend, xQueueSet);

    /* Set up LED: R-> G -> B */
    xLEDState = LEDS_RED;
    vLedsOff(LED_TARGET_ALL);
    vLedsOn(xLEDState);

    xLEDTimer = xTimerCreate( "LED change timer", LED_CHANGE_TIMER_PERIOD, pdTRUE, NULL, prvLEDTimerCallback );
    configASSERT( xLEDTimer );

    xTimerStart( xLEDTimer, portMAX_DELAY );

    xTaskCreate( (void *) &LEDTask, (const signed char *) "LEDTask", mainRADIO_TASK_STACK_SIZE, NULL, mainRADIO_TASK_PRIORITY, &xTaskLEDOsHandle );
}

void s4527438_os_led_command(LED_Cmd_Enum led_cmd,LED_Color_Enum led_color) {
    struct MyOsLibLEDMessage SendMessage;

    SendMessage.led_cmd = led_cmd;
    SendMessage.led_color = led_color;

    if (s4527438QueueLEDPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueLEDPacketSend, ( void * ) &SendMessage, ( portTickType ) 0 );
    }
}

static void LEDTask( void ) {
    struct MyOsLibLEDMessage RecvMessage;
    eLEDs_t xTargetLED = LEDS_NONE;

    QueueSetMemberHandle_t xActivatedMember;

    for (;;) {

        xActivatedMember = xQueueSelectFromSet(xQueueSet, QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK);

        if (xActivatedMember == s4527438QueueLEDPacketSend) {  
            /* Receive item */
            xQueueReceive( s4527438QueueLEDPacketSend, &RecvMessage, 0 );
            switch(RecvMessage.led_color)
            {
                case MY_OS_LIB_LED_COLOR_RED:
                    xMaskLEDs |= LEDS_RED;
                    xTargetLED = LEDS_RED;
                    break;
                case MY_OS_LIB_LED_COLOR_GREEN:
                    xMaskLEDs |= LEDS_GREEN;
                    xTargetLED = LEDS_GREEN;
                    break;
                case MY_OS_LIB_LED_COLOR_BLUE:
                    xMaskLEDs |= LEDS_BLUE;
                    xTargetLED = LEDS_BLUE;
                    break;
                default:
                    break;
            }
            if( RecvMessage.led_cmd == MY_OS_LIB_LED_CMD_ON ) {
                vLedsOn(xTargetLED);
            }else if( RecvMessage.led_cmd == MY_OS_LIB_LED_CMD_OFF ){
                vLedsOff(xTargetLED);
            } else if( RecvMessage.led_cmd == MY_OS_LIB_LED_CMD_TOGGLE ){
                vLedsToggle(xTargetLED);
            }
        }

        /* Delay for 10ms */
        vTaskDelay(10);
        
    }
}

static void prvLEDTimerCallback( TimerHandle_t xExpiredTimer )
{
    /* Remove compiler warnings about unused parameters. */
    ( void ) xExpiredTimer;

    vLedsOff(LED_TARGET_ALL & (~xMaskLEDs));
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
    vLedsOn(xLEDState & (~xMaskLEDs));
}

