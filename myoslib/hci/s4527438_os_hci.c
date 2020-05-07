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
#include "s4527438_os_ble.h"
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

static QueueSetHandle_t xHCIWriteQueueSet;
//static QueueSetHandle_t xHCIReadQueueSet;
static QueueSetHandle_t xHCIReadInternalCommandQueueSet;

static uint8_t I2C_SIDMapToWriteAddr[] = {0x00 , 
                                            HCI_PACKET_DATA_I2C_ADDR_WRITE_LSM6DSL,
                                            HCI_PACKET_DATA_I2C_ADDR_WRITE_LIS3MDL,
                                            HCI_PACKET_DATA_I2C_ADDR_WRITE_LPS22HB,
                                            HCI_PACKET_DATA_I2C_ADDR_WRITE_VL53L0X,
                                            HCI_PACKET_DATA_I2C_ADDR_WRITE_HTS221};

static uint8_t I2C_SIDMapToReadAddr[] = {0x00 , 
                                            HCI_PACKET_DATA_I2C_ADDR_READ_LSM6DSL,
                                            HCI_PACKET_DATA_I2C_ADDR_READ_LIS3MDL,
                                            HCI_PACKET_DATA_I2C_ADDR_READ_LPS22HB,
                                            HCI_PACKET_DATA_I2C_ADDR_READ_VL53L0X,
                                            HCI_PACKET_DATA_I2C_ADDR_READ_HTS221};
/* Private function prototypes -----------------------------------------------*/

static void HCIWriteTask( void );
static void HCIReadTask( void );
static void vCustomSerialHandler(xHCICommsInterface_t *pxComms,
                          xHCICommsMessage_t *pxMessage);


void s4527438_os_hci_init(void) {

    s4527438QueueHCIWritePacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCICommsMessage_t));
    xHCIWriteQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    xQueueAddToSet(s4527438QueueHCIWritePacketSend, xHCIWriteQueueSet);

    s4527438QueueHCIReadInternalCommandPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCIReceiveRoute_t));
    s4527438QueueHCIReadPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xHCICommsMessage_t));
    //xHCIReadQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    //xQueueAddToSet(s4527438QueueHCIReadPacketSend, xHCIReadQueueSet);
    //xQueueAddToSet(s4527438QueueHCIReadInternalCommandPacketSend, xHCIReadQueueSet);

    xTaskCreate( (void *) &HCIWriteTask, (const signed char *) "HCIWriteTask", mainHCIWrite_TASK_STACK_SIZE, NULL, mainHCIWrite_TASK_PRIORITY, &xTaskHCIWriteOsHandle );
    xTaskCreate( (void *) &HCIReadTask, (const signed char *) "HCIReadTask", mainHCIRead_TASK_STACK_SIZE, NULL, mainHCIRead_TASK_PRIORITY, &xTaskHCIReadOsHandle );

    /* Setup our uart receive handler for HCI */
    xHCIComms.fnReceiveHandler = vCustomSerialHandler;
}

void s4527438_os_hci_write_reg_cmd(uint8_t  sid, uint8_t I2CRegAddr, uint8_t I2CRegValue,uint8_t num_word,uint8_t word_size)
{
    /* Build HCI comms and data field */
    xHCIDataField_t xHCIDataField = {
        0x00    
    };
    xHCICommsMessage_t pxMessage ={
        0x00
    };

    xHCIDataField.ucSID = sid;
    xHCIDataField.ucI2CAddress = I2C_SIDMapToWriteAddr[sid];
    xHCIDataField.ucI2CRegAddr = I2CRegAddr;
    xHCIDataField.ucI2CRegValue = I2CRegValue;

    /* Build Message Packet */
    pxMessage.usPayloadLen = sizeof(xHCIDataField);
    pxMessage.ucPayloadType = HCI_TYPE_REQUEST;
    pvMemcpy( pxMessage.pucPayload, &xHCIDataField, sizeof(xHCIDataField) );

    if (s4527438QueueHCIWritePacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIWritePacketSend, ( void * ) &pxMessage, ( portTickType ) 0 );
    }

    if (s4527438QueueHCIReadPacketSend != NULL) { /* Check if queue exists */
        xHCIReceiveRoute_t pxHCIRoute = {
            .usPayloadLen = 0,
            .usPayloadRoute = HCI_ROUTE_SERIAL_OUTPUT,
            .usPayloadWordInfo = HCI_PACKET_DATA_FIELD_NUM_WORD_AND_WORD_SIZE( num_word,word_size)
        };
        xQueueSend(s4527438QueueHCIReadInternalCommandPacketSend, ( void * ) &pxHCIRoute, ( portTickType ) 0 );
    }
}

void s4527438_os_hci_read_reg_cmd(uint8_t  sid, uint8_t I2CRegAddr, uint8_t I2CRegValue,uint8_t num_word,uint8_t word_size)
{
    /* Build HCI comms and data field */
    xHCIDataField_t xHCIDataField = {
        0x00    
    };
    xHCICommsMessage_t pxMessage ={
        0x00
    };

    xHCIDataField.ucSID = sid;
    xHCIDataField.ucI2CAddress = I2C_SIDMapToReadAddr[sid];
    xHCIDataField.ucI2CRegAddr = I2CRegAddr;
    xHCIDataField.ucI2CRegValue = I2CRegValue;
    
    /* Build Message Packet */
    pxMessage.usPayloadLen = sizeof(xHCIDataField);
    pxMessage.ucPayloadType = HCI_TYPE_REQUEST;
    pvMemcpy( pxMessage.pucPayload, &xHCIDataField, sizeof(xHCIDataField) );

    if (s4527438QueueHCIWritePacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIWritePacketSend, ( void * ) &pxMessage, ( portTickType ) 0 );
    }

    if (s4527438QueueHCIReadPacketSend != NULL) { /* Check if queue exists */
        xHCIReceiveRoute_t pxHCIRoute = {
            .usPayloadLen = 0,
            .usPayloadRoute = HCI_ROUTE_SERIAL_OUTPUT,
            .usPayloadWordInfo = HCI_PACKET_DATA_FIELD_NUM_WORD_AND_WORD_SIZE( num_word,word_size)
        };
        xQueueSend(s4527438QueueHCIReadInternalCommandPacketSend, ( void * ) &pxHCIRoute, ( portTickType ) 0 );
    }
}

void s4527438_os_hci_tdf_read_cmd(uint8_t num_word,uint8_t word_size,xBLETdfMessage_t* tdfInfo) {
    /* Build HCI comms and data field */
    xHCIDataField_t xHCIDataField = {
        0x00    
    };
    xHCICommsMessage_t pxMessage ={
        0x00
    };

    xHCIDataField.ucSID = tdfInfo->usHCIMapSID;
    xHCIDataField.ucI2CAddress = I2C_SIDMapToReadAddr[tdfInfo->usHCIMapSID];
    xHCIDataField.ucI2CRegAddr = 0;
    xHCIDataField.ucI2CRegValue = tdfInfo->usI2CRegValue;

    /* Build Message Packet */
    pxMessage.usPayloadLen = sizeof(xHCIDataField);
    pxMessage.ucPayloadType = HCI_TYPE_REQUEST;
    pvMemcpy( pxMessage.pucPayload, &xHCIDataField, sizeof(xHCIDataField) );

    if (s4527438QueueHCIWritePacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIWritePacketSend, ( void * ) &pxMessage, ( portTickType ) 0 );
    }

    /* Build HCI internal route package */
    if (s4527438QueueHCIReadPacketSend != NULL) { /* Check if queue exists */
        uint8_t cpSize = 0;
        xHCIReceiveRoute_t pxHCIRoute = {
            .usPayloadLen = sizeof(xBLETdfMessage_t),
            .usPayloadRoute = HCI_ROUTE_BLE_TDF,
            .usPayloadWordInfo = HCI_PACKET_DATA_FIELD_NUM_WORD_AND_WORD_SIZE( num_word,word_size)
        };
        if( sizeof(xBLETdfMessage_t) > sizeof(pxHCIRoute.pucPayload) ) {
            cpSize = sizeof(pxHCIRoute.pucPayload);
        } else {
            cpSize = sizeof(xBLETdfMessage_t);
        }
        pvMemcpy(pxHCIRoute.pucPayload,tdfInfo,cpSize);
        xQueueSend(s4527438QueueHCIReadInternalCommandPacketSend, ( void * ) &pxHCIRoute, ( portTickType ) 0 );
    }
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
#define STATE_READY_RECEIVE_INTERNAL_CMD_PACKET 1
#define STATE_READY_PROCESS_DATA_PACKET 2
#define STATE_READY_PROCESS_CONTINUOUS_DATA_PACKET 3
static void HCIReadTask( void ) {
    xHCIReceiveRoute_t pxHCIRoute;
    QueueSetMemberHandle_t xActivatedMember;
    xHCICommsMessage_t pxDataMessage;
    uint8_t read_task_state = STATE_WAIT_NEW_RESPONSE;

    uint8_t word_size = 0;
    uint8_t num_word = 0;
    for (;;) {

        if(read_task_state == STATE_WAIT_NEW_RESPONSE) {
            // Wait block
            xQueueReceive( s4527438QueueHCIReadPacketSend, &pxDataMessage,  portMAX_DELAY);
            if( pxDataMessage.usPayloadLen == 0 ) {
                read_task_state = STATE_WAIT_NEW_RESPONSE;
                continue;
            }
            if( pxDataMessage.ucPayloadType == HCI_TYPE_RESPONSE ) {
                read_task_state = STATE_READY_RECEIVE_INTERNAL_CMD_PACKET;
            } else if( pxDataMessage.ucPayloadType == HCI_TYPE_CONTINUOUS ) {
                read_task_state = STATE_READY_PROCESS_CONTINUOUS_DATA_PACKET;
            }
        } 

        if(read_task_state == STATE_READY_RECEIVE_INTERNAL_CMD_PACKET) {
            word_size = 0;
            num_word = 0;
            read_task_state = STATE_READY_PROCESS_DATA_PACKET;

            // Get Packet Internal Header for word size info
            xQueueReceive( s4527438QueueHCIReadInternalCommandPacketSend, &pxHCIRoute, 0 );
            word_size = HCI_PACKET_DATA_LOW_GET_WORD_SIZE(pxHCIRoute.usPayloadWordInfo);
            num_word = HCI_PACKET_DATA_HIGH_GET_NUM_WORD(pxHCIRoute.usPayloadWordInfo);
        } 

        if(read_task_state == STATE_READY_PROCESS_DATA_PACKET) {
            xBLETdfMessage_t *tdfRoute = NULL;
            int i = 0,j = 0;

            /* DO two queue sync first */
            while(1) {
                if(num_word*word_size == pxDataMessage.usPayloadLen ){
                    break;
                }
                // Get Packet Internal Header for word size info
                if(xQueueReceive( s4527438QueueHCIReadInternalCommandPacketSend, &pxHCIRoute, 0 ) != pdPASS){
                    read_task_state = STATE_WAIT_NEW_RESPONSE;
                    break;
                }
                word_size = HCI_PACKET_DATA_LOW_GET_WORD_SIZE(pxHCIRoute.usPayloadWordInfo);
                num_word = HCI_PACKET_DATA_HIGH_GET_NUM_WORD(pxHCIRoute.usPayloadWordInfo);
            }
            if( read_task_state = STATE_WAIT_NEW_RESPONSE ){
                continue;
            }
            
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[HCI Event]: num_word = [%d]",num_word);
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[HCI Event]: word_size = [%d]",word_size);
            tdfRoute = (xBLETdfMessage_t *)(pxHCIRoute.pucPayload);
            for(i = 0;i < num_word;i++){
                uint16_t result = 0;
                uint16_t next = 0;
                for(j = 0;j < word_size;j++){
                    next = pxDataMessage.pucPayload[i*word_size + j];
                    if( j == (word_size - 1) ) {
                        break;
                    }
                    result = (next << (8*j)) | result;
                }
                if( pxHCIRoute.usPayloadRoute == HCI_ROUTE_SERIAL_OUTPUT ) {
                    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[HCI Event]: value = [%x]",result);
                }
            }
            if( pxHCIRoute.usPayloadRoute == HCI_ROUTE_BLE_TDF ) {
                if( (num_word * word_size) > sizeof(tdfRoute->usReceivedValue) ) {
                    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[HCI Event]: num_word * word_size exceed the tdfRoute buffer ");
                }else {
                    pvMemcpy(tdfRoute->usReceivedValue,pxDataMessage.pucPayload,(num_word * word_size));
                }
                s4527438_os_ble_tdf_result_cmd(tdfRoute);
            }
            read_task_state = STATE_WAIT_NEW_RESPONSE;
        }

        if(read_task_state == STATE_READY_PROCESS_CONTINUOUS_DATA_PACKET) {
            xBLETdfMessage_t tdfRoute = {0x00};
            int i = 0,j = 0;

            // NOTE : The first byte is SID value
            tdfRoute.usTdfIndex = 0;
            tdfRoute.usHCIMapSID = pxDataMessage.pucPayload[0];
            tdfRoute.usTdfReceivedValueLength = pxDataMessage.usPayloadLen - 1;
            tdfRoute.usI2CRegValue = 'c';
            if( (pxDataMessage.usPayloadLen - 1) > sizeof(tdfRoute.usReceivedValue) ) {
                s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[HCI Event]: %d exceed the tdfRoute buffer ",(pxDataMessage.usPayloadLen - 1));
                read_task_state = STATE_WAIT_NEW_RESPONSE;
                continue;
            }else {
                pvMemcpy(tdfRoute.usReceivedValue,&(pxDataMessage.pucPayload[1]),(pxDataMessage.usPayloadLen - 1));
            }
            s4527438_os_ble_tdf_result_cmd(&tdfRoute);
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

    if (s4527438QueueHCIReadPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueHCIReadPacketSend, ( void * ) pxMessage, ( portTickType ) 0 );
    }
    return;
}
