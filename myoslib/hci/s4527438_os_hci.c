/** 
 **************************************************************
 * @file mylib/s4527438_os_hci.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib hci driver
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

#include "s4527438_os_hci.h"
#include "s4527438_hal_hci.h"
#include "s4527438_lib_log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define mainHCIWrite_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )
#define mainHCIRead_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )

#define mainHCIWrite_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )
#define mainHCIRead_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )

#define QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK 100

#define QUEUE_LENGTH                        50

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TaskHandle_t xTaskHCIWriteOsHandle;
static TaskHandle_t xTaskHCIReadOsHandle;

static SemaphoreHandle_t s4527438QueueHCIWritePacketSend = NULL;
static SemaphoreHandle_t s4527438QueueHCIReadPacketSend = NULL;

static QueueSetHandle_t xHCIWriteQueueSet;
static QueueSetHandle_t xHCIReadQueueSet;

/* Private function prototypes -----------------------------------------------*/
static void HCIWriteTask( void );
static void HCIReadTask( void );
static void vCustomSerialHandler(xHCICommsInterface_t *pxComms,
                          xHCICommsMessage_t *pxMessage);


void s4527438_os_hci_init(void) {

    s4527438QueueHCIWritePacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCICommsMessage_t));
    xHCIWriteQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    xQueueAddToSet(s4527438QueueHCIWritePacketSend, xHCIWriteQueueSet);

    s4527438QueueHCIReadPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCICommsMessage_t));
    xHCIReadQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    xQueueAddToSet(s4527438QueueHCIReadPacketSend, xHCIReadQueueSet);

    xTaskCreate( (void *) &HCIWriteTask, (const signed char *) "HCIWriteTask", mainHCIWrite_TASK_STACK_SIZE, NULL, mainHCIWrite_TASK_PRIORITY, &xTaskHCIWriteOsHandle );
    xTaskCreate( (void *) &HCIReadTask, (const signed char *) "HCIReadTask", mainHCIRead_TASK_STACK_SIZE, NULL, mainHCIRead_TASK_PRIORITY, &xTaskHCIReadOsHandle );

    /* Setup our uart receive handler for HCI */
    xHCIComms.fnReceiveHandler = vCustomSerialHandler;
}

void s4527438_os_hci_write_cmd(xHCICommsMessage_t *pxMessage) {
    if (s4527438QueueHCIWritePacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIWritePacketSend, ( void * ) pxMessage, ( portTickType ) 0 );
    }
}

void s4527438_os_hci_read_cmd(xHCICommsMessage_t *pxMessage)
{
    if (s4527438QueueHCIReadPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIReadPacketSend, ( void * ) pxMessage, ( portTickType ) 0 );
    }
    return;
}

static void HCIWriteTask( void ) {
    xHCICommsMessage_t pxMessage;
    QueueSetMemberHandle_t xActivatedMember;

    for (;;) {

        xActivatedMember = xQueueSelectFromSet(xHCIWriteQueueSet, QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK);

        if (xActivatedMember == s4527438QueueHCIWritePacketSend) {  
            /* Receive item */
            xQueueReceive( s4527438QueueHCIWritePacketSend, &pxMessage, 0 );
            xHCIComms.fnSend(&pxMessage);
        }
        /* Delay for 10ms */
        vTaskDelay(10);
    }
}

static void HCIReadTask( void ) {
    xHCICommsMessage_t pxMessage;
    QueueSetMemberHandle_t xActivatedMember;

    for (;;) {

        xActivatedMember = xQueueSelectFromSet(xHCIReadQueueSet, QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK);

        if (xActivatedMember == s4527438QueueHCIReadPacketSend) {  
            /* Receive item */
            xQueueReceive( s4527438QueueHCIReadPacketSend, &pxMessage, 0 );
            
            char bufferPtr[pxMessage.usPayloadLen + 1];
            bufferPtr[pxMessage.usPayloadLen] = 0x00;
            pvMemcpy( bufferPtr, pxMessage.pucPayload, pxMessage.usPayloadLen );
            
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[HCI Event]: value = [%s] , size = [%d]",bufferPtr,pxMessage.usPayloadLen);
        }
        /* Delay for 10ms */
        vTaskDelay(10);
    }
}

static void vCustomSerialHandler(xHCICommsInterface_t *pxComms,
                          xHCICommsMessage_t *pxMessage)
{
    UNUSED(pxComms);

    s4527438_os_hci_read_cmd(pxMessage);
}
