/** 
 **************************************************************
 * @file mylib/s4527438_os_ble.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib ble driver
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

#include "s4527438_os_ble.h"
#include "s4527438_os_hci.h"
#include "s4527438_lib_log.h"

#include "tdf.h"
#include "log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define mainHCIWrite_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )
#define mainHCIRead_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )

#define mainHCIWrite_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )
#define mainHCIRead_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )

#define QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK 100

#define QUEUE_LENGTH                        50



/*  Below is used for TDF to send hci packet for getting sensor value */
#define BLE_TDF_342_SID             3

#define BLE_TDF_342_FIELD_PRESSURE_REG_VALUE    'p'
#define BLE_TDF_342_FIELD_PRESSURE_NUM_WORD 1
#define BLE_TDF_342_FIELD_PRESSURE_WORD_SIZE    4
#define BLE_TDF_342_FIELD_PRESSURE_READY_BIT    (1<<0)  

#define BLE_TDF_342_FIELD_TEMPERATURE_REG_VALUE 't'
#define BLE_TDF_342_FIELD_TEMPERATURE_NUM_WORD  1
#define BLE_TDF_342_FIELD_TEMPERATURE_WORD_SIZE 2
#define BLE_TDF_342_FIELD_TEMPERATURE_READY_BIT (1<<1)  

#define BLE_TDF_342_FIELD_READY_MASK    (0x0000 | BLE_TDF_342_FIELD_PRESSURE_READY_BIT | BLE_TDF_342_FIELD_PRESSURE_READY_BIT)





#define BLE_TDF_471_SID             1
    
#define BLE_TDF_471_FIELD_ALL_REG_VALUE 'a'
#define BLE_TDF_471_FIELD_ALL_NUM_WORD  6
#define BLE_TDF_471_FIELD_ALL_WORD_SIZE 2
#define BLE_TDF_471_FIELD_ALL_READY_BIT (1<<0)  

#define BLE_TDF_471_FIELD_READY_MASK    (0x0000 | BLE_TDF_471_FIELD_ALL_READY_BIT ) 



#define BLE_TDF_241_SID             0

#define BLE_TDF_241_FIELD_UPTIME_READY_BIT  (1<<0)  
#define BLE_TDF_241_FIELD_UPTIME_WORD_SIZE  4

#define BLE_TDF_241_FIELD_READY_MASK        (0x00 | BLE_TDF_241_FIELD_UPTIME_READY_BIT)




#define BLE_TDF_472_SID             1

#define BLE_TDF_472_FIELD_ROLL_REG_VALUE    'r'
#define BLE_TDF_472_FIELD_ROLL_NUM_WORD     1
#define BLE_TDF_472_FIELD_ROLL_WORD_SIZE    2
#define BLE_TDF_472_FIELD_ROLL_READY_BIT    (1<<0)  

#define BLE_TDF_472_FIELD_PITCH_REG_VALUE   'p'
#define BLE_TDF_472_FIELD_PITCH_NUM_WORD    1
#define BLE_TDF_472_FIELD_PITCH_WORD_SIZE   2
#define BLE_TDF_472_FIELD_PITCH_READY_BIT   (1<<1)  

#define BLE_TDF_472_FIELD_YAW_REG_VALUE     'y'
#define BLE_TDF_472_FIELD_YAW_NUM_WORD      1
#define BLE_TDF_472_FIELD_YAW_WORD_SIZE     2
#define BLE_TDF_472_FIELD_YAW_READY_BIT     (1<<2)  


#define BLE_TDF_472_FIELD_READY_MASK        (0x00 | BLE_TDF_472_FIELD_ROLL_READY_BIT | BLE_TDF_472_FIELD_PITCH_READY_BIT | BLE_TDF_472_FIELD_YAW_READY_BIT)


#define BLE_TDF_475_SID             4
    
#define BLE_TDF_475_FIELD_RANGE_REG_VALUE   'r'
#define BLE_TDF_475_FIELD_RANGE_NUM_WORD    1
#define BLE_TDF_475_FIELD_RANGE_WORD_SIZE   2
#define BLE_TDF_475_FIELD_RANGE_READY_BIT   (1<<0)  

#define BLE_TDF_475_FIELD_READY_MASK        (0x00 | BLE_TDF_475_FIELD_RANGE_READY_BIT)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TaskHandle_t xTaskBLETdfOsHandle;

static SemaphoreHandle_t s4527438QueueBLETdfPacketSend = NULL;

static QueueSetHandle_t xBLEQueueSet;
static QueueSetHandle_t xHCIReadInternalCommandQueueSet;

/* Private function prototypes -----------------------------------------------*/
static void BLETdfHandlerTask( void );
static void handle_tdf_342(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_471(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_241(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_472(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_476(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_475(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_474(xBLETdfMessage_t* tdfInfo);
static void handle_continuous_mode(xBLETdfMessage_t* tdfInfo);

void s4527438_os_ble_init(void) {

    s4527438QueueBLETdfPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct xBLETdfMessage_t));
    xBLEQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    xQueueAddToSet(s4527438QueueBLETdfPacketSend, xBLEQueueSet);

    xTaskCreate( (void *) &BLETdfHandlerTask, (const signed char *) "BLETdfHandlerTask", mainHCIWrite_TASK_STACK_SIZE, NULL, mainHCIWrite_TASK_PRIORITY, &xTaskBLETdfOsHandle );

}

void s4527438_os_ble_tdf_get_sensor_value_cmd(uint16_t tdfId) {
    switch(tdfId){
        case 471:
            {
                xBLETdfMessage_t tdfInfo = {
                    .usTdfIndex = tdfId,
                    .usHCIMapSID = BLE_TDF_471_SID,
                    .usI2CRegAddr = 0,
                    .usTdfFieldBit = BLE_TDF_471_FIELD_ALL_READY_BIT,
                    .usI2CRegValue = BLE_TDF_471_FIELD_ALL_REG_VALUE
                };
                s4527438_os_hci_tdf_read_cmd(BLE_TDF_471_FIELD_ALL_NUM_WORD,BLE_TDF_471_FIELD_ALL_WORD_SIZE,&tdfInfo);
            }
            break;
        default:
            break;
    }
}

void s4527438_os_ble_tdf_result_cmd(xBLETdfMessage_t *tdfInfo) {
    if (s4527438QueueBLETdfPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueBLETdfPacketSend, ( void * ) tdfInfo, ( portTickType ) 0 );
    }
}

static void BLETdfHandlerTask( void ) {
    xBLETdfMessage_t tdfInfo;
    QueueSetMemberHandle_t xActivatedMember;

    for (;;) {

        xActivatedMember = xQueueSelectFromSet(xBLEQueueSet, QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK);

        if (xActivatedMember == s4527438QueueBLETdfPacketSend) {  
            /* Receive item */
            xQueueReceive( s4527438QueueBLETdfPacketSend, &tdfInfo, 0 );
            switch(tdfInfo.usTdfIndex)
            {
                /* pressure / temperature */
                case 342:
                    handle_tdf_342(&tdfInfo);
                    break;

                /* xyz gyro-xyz */
                case 471:
                    handle_tdf_471(&tdfInfo);
                    break;

                /* Uptime */
                case 241:
                    break;

                /* 3D orientation */
                case 472:
                    handle_tdf_472(&tdfInfo);
                    break;

                /* height */
                case 476:
                    handle_tdf_476(&tdfInfo);
                    break;

                /* range */
                case 475:
                    handle_tdf_475(&tdfInfo);
                    break;

                /* angle */
                case 474:
                    handle_tdf_474(&tdfInfo);
                    break;
                default:
                    handle_continuous_mode(&tdfInfo);
                    break;
            }
	        eTdfFlushMulti(BLE_LOG);
        }
        /* Delay for 10ms */
        vTaskDelay(10);
    }
}

static void handle_tdf_342(xBLETdfMessage_t* tdfInfo)
{
}

static void handle_tdf_471(xBLETdfMessage_t* tdfInfo)
{
    tdf_lsm6dsl_t tdfSentObj = {
        .acc_x = (tdfInfo->usReceivedValue[0] | (tdfInfo->usReceivedValue[1] << 8)),
        .acc_y = (tdfInfo->usReceivedValue[2] | (tdfInfo->usReceivedValue[3] << 8)),
        .acc_z = (tdfInfo->usReceivedValue[4] | (tdfInfo->usReceivedValue[5] << 8)),
        .gyro_x = (tdfInfo->usReceivedValue[6] | (tdfInfo->usReceivedValue[7] << 8)),
        .gyro_y = (tdfInfo->usReceivedValue[8] | (tdfInfo->usReceivedValue[9] << 8)),
        .gyro_z = (tdfInfo->usReceivedValue[10] | (tdfInfo->usReceivedValue[11] << 8))
    };

	eTdfAddMulti(BLE_LOG, TDF_LSM6DSL, TDF_TIMESTAMP_NONE, NULL, &tdfSentObj);
	eTdfFlushMulti(BLE_LOG);
}

static void handle_tdf_241(xBLETdfMessage_t* tdfInfo)
{
}

static void handle_tdf_472(xBLETdfMessage_t* tdfInfo)
{
}

static void handle_tdf_476(xBLETdfMessage_t* tdfInfo)
{
}

static void handle_tdf_475(xBLETdfMessage_t* tdfInfo)
{
}

static void handle_tdf_474(xBLETdfMessage_t* tdfInfo)
{
}
static void handle_continuous_mode(xBLETdfMessage_t* tdfInfo) {
    switch(tdfInfo->usHCIMapSID)
    {
        /* pressure / temperature */
        case BLE_TDF_342_SID:
            handle_tdf_342(tdfInfo);
            break;
        
        /* NOTE : 471 do not need continuous mode 
        case BLE_TDF_471_SID:
            handle_tdf_471(tdfInfo);
            break;
        */
        case BLE_TDF_472_SID:
            handle_tdf_472(tdfInfo);
            break;
        case BLE_TDF_475_SID:
            handle_tdf_475(tdfInfo);
            break;
        default:
            break;
    }
}
