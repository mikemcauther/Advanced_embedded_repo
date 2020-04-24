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
#include "s4527438_hci_packet.h"

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
static SemaphoreHandle_t s4527438QueueHCIReadInternalCommandPacketSend = NULL;
static SemaphoreHandle_t s4527438QueueHCIReadSemaphore = NULL;

static QueueSetHandle_t xHCIWriteQueueSet;
//static QueueSetHandle_t xHCIReadQueueSet;
static QueueSetHandle_t xHCIReadInternalCommandQueueSet;

/* Private function prototypes -----------------------------------------------*/
static void HCIWriteTask( void );
static void HCIReadTask( void );
static void vCustomSerialHandler(xHCICommsInterface_t *pxComms,
                          xHCICommsMessage_t *pxMessage);


void s4527438_os_hci_init(void) {

    s4527438QueueHCIWritePacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCICommsMessage_t));
    xHCIWriteQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    xQueueAddToSet(s4527438QueueHCIWritePacketSend, xHCIWriteQueueSet);

    vSemaphoreCreateBinary( s4527438QueueHCIReadSemaphore );
    s4527438QueueHCIReadInternalCommandPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCICommsMessage_t));
    s4527438QueueHCIReadPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCICommsMessage_t));
    //xHCIReadQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    //xQueueAddToSet(s4527438QueueHCIReadPacketSend, xHCIReadQueueSet);
    //xQueueAddToSet(s4527438QueueHCIReadInternalCommandPacketSend, xHCIReadQueueSet);

    xTaskCreate( (void *) &HCIWriteTask, (const signed char *) "HCIWriteTask", mainHCIWrite_TASK_STACK_SIZE, NULL, mainHCIWrite_TASK_PRIORITY, &xTaskHCIWriteOsHandle );
    xTaskCreate( (void *) &HCIReadTask, (const signed char *) "HCIReadTask", mainHCIRead_TASK_STACK_SIZE, NULL, mainHCIRead_TASK_PRIORITY, &xTaskHCIReadOsHandle );

    /* Setup our uart receive handler for HCI */
    xHCIComms.fnReceiveHandler = vCustomSerialHandler;
}

void s4527438_os_hci_write_cmd(xHCICommsMessage_t *pxMessage,uint8_t num_word,uint8_t word_size) {
    if (s4527438QueueHCIWritePacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIWritePacketSend, ( void * ) pxMessage, ( portTickType ) 0 );
    }

    if (s4527438QueueHCIReadPacketSend != NULL) { /* Check if queue exists */
        xHCICommsMessage_t xMessage = {
            .usPayloadLen = 1,
            .pucPayload = HCI_PACKET_DATA_FIELD_NUM_WORD_AND_WORD_SIZE( num_word,word_size)
        };
        xQueueSend(s4527438QueueHCIReadInternalCommandPacketSend, ( void * ) &xMessage, ( portTickType ) 0 );
    }
}

void s4527438_os_hci_read_cmd(xHCICommsMessage_t *pxMessage)
{
    if (s4527438QueueHCIReadPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIReadPacketSend, ( void * ) pxMessage, ( portTickType ) 0 );
    }
    xSemaphoreGive(s4527438QueueHCIReadSemaphore);
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

#define STATE_WAIT_NEW_RESPONSE 0
#define STATE_READY_RECEIVE_DATA_PACKET 2
static void HCIReadTask( void ) {
    xHCICommsMessage_t pxMessage;
    QueueSetMemberHandle_t xActivatedMember;
    uint8_t read_task_state = STATE_WAIT_NEW_RESPONSE;

    uint8_t word_size = 0;
    uint8_t num_word = 0;
    for (;;) {

        if(read_task_state == STATE_WAIT_NEW_RESPONSE) {
            word_size = 0;
            num_word = 0;
            // Wait block
            xSemaphoreTake(s4527438QueueHCIReadSemaphore,portMAX_DELAY);
            read_task_state = STATE_READY_RECEIVE_DATA_PACKET;

            // Get Packet Internal Header for word size info
            xQueueReceive( s4527438QueueHCIReadInternalCommandPacketSend, &pxMessage, 0 );
            if( pxMessage.usPayloadLen == 0 ) {
                read_task_state = STATE_WAIT_NEW_RESPONSE;
                continue;
            }
            word_size = HCI_PACKET_DATA_LOW_GET_WORD_SIZE(pxMessage.pucPayload[0]);
            num_word = HCI_PACKET_DATA_HIGH_GET_NUM_WORD(pxMessage.pucPayload[0]);
        } 

        if(read_task_state == STATE_READY_RECEIVE_DATA_PACKET) {
            xHCICommsMessage_t pxDataMessage;
            /* Receive item */
            xQueueReceive( s4527438QueueHCIReadPacketSend, &pxDataMessage, 0 );
            int i = 0,j = 0;

            if( pxDataMessage.usPayloadLen == 0 ) {
                read_task_state = STATE_WAIT_NEW_RESPONSE;
                continue;
            }

            /* DO two queue sync first */
            while(1) {
                if(num_word*word_size == pxDataMessage.usPayloadLen ){
                    break;
                }
                // Get Packet Internal Header for word size info
                if(xQueueReceive( s4527438QueueHCIReadInternalCommandPacketSend, &pxMessage, 0 ) != pdPASS){
                    read_task_state = STATE_WAIT_NEW_RESPONSE;
                    break;
                }
                if( pxMessage.usPayloadLen == 0 ) {
                    continue;
                }
                word_size = HCI_PACKET_DATA_LOW_GET_WORD_SIZE(pxMessage.pucPayload[0]);
                num_word = HCI_PACKET_DATA_HIGH_GET_NUM_WORD(pxMessage.pucPayload[0]);
            }
            if( read_task_state = STATE_WAIT_NEW_RESPONSE ){
                continue;
            }
            
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[HCI Event]: num_word = [%d]",num_word);
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[HCI Event]: word_size = [%d]",word_size);
            for(i = 0;i < num_word;i++){
                uint16_t result = pxDataMessage.pucPayload[i*word_size + 0];
                uint16_t next = (result << 8);
                for(j = 0;j < word_size;j++){
                    if( j == (word_size - 1) ) {
                        break;
                    }
                    result = (next | pxDataMessage.pucPayload[i*word_size + j]);
                    next = (result << 8);
                }
                s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[HCI Event]: value = [%x]",result);
            }
            read_task_state = STATE_WAIT_NEW_RESPONSE;
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
