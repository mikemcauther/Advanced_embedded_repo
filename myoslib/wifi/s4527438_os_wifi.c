/** 
 **************************************************************
 * @file mylib/s4527438_os_wifi.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib wifi driver
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

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "board.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "s4527438_os_wifi.h"
#include "s4527438_os_ble.h"
#include "s4527438_hal_wifi.h"
#include "s4527438_lib_log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define mainWifiWrite_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )
#define mainWifiRead_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )

#define mainWifiWrite_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )
#define mainWifiRead_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )

#define QUEUE_LENGTH                        50

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TaskHandle_t xTaskWifiWriteOsHandle;
static TaskHandle_t xTaskWifiReadOsHandle;

static SemaphoreHandle_t s4527438QueueWifiWritePacketSend = NULL;
static SemaphoreHandle_t s4527438QueueWifiReadPacketSend = NULL;

/* Private function prototypes -----------------------------------------------*/

static void WifiWriteTask( void );
static void WifiReadTask( void );
static void vCustomSerialHandler(xWifiCommsInterface_t *pxComms,
                          xWifiCommsMessage_t *pxMessage);


void s4527438_os_wifi_init(void) {

    s4527438QueueWifiWritePacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xWifiCommsMessage_t));
    s4527438QueueWifiReadPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xWifiCommsMessage_t));

    xTaskCreate( (void *) &WifiWriteTask, (const signed char *) "WifiWriteTask", mainWifiWrite_TASK_STACK_SIZE, NULL, mainWifiWrite_TASK_PRIORITY, &xTaskWifiWriteOsHandle );
    xTaskCreate( (void *) &WifiReadTask, (const signed char *) "WifiReadTask", mainWifiRead_TASK_STACK_SIZE, NULL, mainWifiRead_TASK_PRIORITY, &xTaskWifiReadOsHandle );

    /* Setup our uart receive handler for Wifi */
    xWifiComms.fnReceiveHandler = vCustomSerialHandler;
}

void s4527438_os_wifi_send_cmd(uint8_t *cmd,uint8_t cmd_len)
{
    xWifiCommsMessage_t pxMessage ={
        0x00
    };

    /* Build Message Packet */
    pxMessage.usPayloadLen = cmd_len;
    pvMemcpy( pxMessage.pucPayload, cmd, cmd_len );

    if (s4527438QueueWifiWritePacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueWifiWritePacketSend, ( void * ) &pxMessage, ( portTickType ) 0 );
    }
}

static void WifiWriteTask( void ) {
    xWifiCommsMessage_t pxMessage;
    QueueSetMemberHandle_t xActivatedMember;

    for (;;) {
        xQueueReceive( s4527438QueueWifiWritePacketSend, &pxMessage,  portMAX_DELAY);
        xWifiComms.fnSend(&pxMessage);
        /* Delay for 10ms */
        vTaskDelay(10);
    }
}

static void WifiReadTask( void ) {
    QueueSetMemberHandle_t xActivatedMember;
    xWifiCommsMessage_t pxDataMessage;

    for (;;) {
        // Wait block
        xQueueReceive( s4527438QueueWifiReadPacketSend, &pxDataMessage,  portMAX_DELAY);
        if( pxDataMessage.usPayloadLen == 0 ) {
            continue;
        }
        /* Delay for 10ms */
        vTaskDelay(10);
    }
}

static void vCustomSerialHandler(xWifiCommsInterface_t *pxComms,
                          xWifiCommsMessage_t *pxMessage)
{
    UNUSED(pxComms);

    if (s4527438QueueWifiReadPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueWifiReadPacketSend, ( void * ) pxMessage, ( portTickType ) 0 );
    }
    return;
}
