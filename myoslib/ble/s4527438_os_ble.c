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
static void handle_tdf_472(xBLETdfMessage_t* tdfInfo,eBLETdfContinuousModeState_t mode_state);
static void handle_tdf_476(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_475(xBLETdfMessage_t* tdfInfo);
static void handle_tdf_474(xBLETdfMessage_t* tdfInfo);
static void handle_continuous_mode(xBLETdfMessage_t* tdfInfo,uint16_t tdfId,eBLETdfContinuousModeState_t mode_state);
static uint8_t tdfId_map_to_sid(uint16_t tdfId);

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

void s4527438_os_ble_tdf_continuous_mode(uint16_t tdfId,eBLETdfContinuousModeState_t mode_state) {
    uint8_t sid = tdfId_map_to_sid(tdfId);
    if( mode_state == BLE_TDF_CONTINUOUS_START ) {
        handle_continuous_mode(NULL,tdfId,BLE_TDF_CONTINUOUS_START);
        s4527438_os_hci_read_reg_cmd(sid, 0x00, 'c',0,0);
    } else if(mode_state == BLE_TDF_CONTINUOUS_STOP) {
        handle_continuous_mode(NULL,tdfId,BLE_TDF_CONTINUOUS_STOP);
        s4527438_os_hci_read_reg_cmd(sid, 0x00, 'p',0,0);
    }
}

static uint8_t tdfId_map_to_sid(uint16_t tdfId) {
    uint8_t sid = 0;
    switch(tdfId)
    {
        /* pressure / temperature */
        case 342:
            sid =  BLE_TDF_342_SID;
            break;

        /* xyz gyro-xyz */
        case 471:
            sid =  BLE_TDF_471_SID;
            break;

        /* Uptime */
        case 241:
            sid =  BLE_TDF_241_SID;
            break;

        /* 3D orientation */
        case 472:
            sid =  BLE_TDF_472_SID;
            break;

        /* height */
        case 476:
            break;

        /* range */
        case 475:
            sid =  BLE_TDF_475_SID;
            break;

        /* angle */
        case 474:
            break;
        default:
            break;
      }
    return sid;
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
                /* xyz gyro-xyz */
                case 471:
                /* Uptime */
                case 241:
                /* 3D orientation */
                case 472:
                /* height */
                case 476:
                /* range */
                case 475:
                /* angle */
                case 474:
                    handle_continuous_mode(&tdfInfo,tdfInfo.usTdfIndex,BLE_TDF_CONTINUOUS_NO_ACTION);
                    break;
                default:
                    handle_continuous_mode(&tdfInfo,tdfInfo.usTdfIndex,BLE_TDF_CONTINUOUS_DATA_INCOMING);
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

static void handle_tdf_472(xBLETdfMessage_t* tdfInfo,eBLETdfContinuousModeState_t mode_state)
{
    static tdf_3d_pose_t tdf_3d_pose_obj = {0x00};
    static received_data_number = 0;
    static uint8_t is_started = 0;

    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] handle_tdf_472 mode_state = <%d>  ",mode_state);
    if( mode_state == BLE_TDF_CONTINUOUS_START ) {
        received_data_number = 0;
        memset(&tdf_3d_pose_obj,0x00,sizeof(tdf_3d_pose_t));
        is_started = 1;
    }

    if( mode_state == BLE_TDF_CONTINUOUS_DATA_INCOMING && is_started == 1 && (tdfInfo != NULL) ) {
        // Discard the first packet
        if( received_data_number == 0 ) {
            received_data_number ++;
            return;
        }
        received_data_number ++;

        uint16_t FS_TYPO = 875, FS_TYPO_decimal = 100;
        uint16_t total_decimal = 1000;
        uint16_t ms_100 = 100, ms_decimal = 1000;
        uint16_t negative_mask = 0x8000,is_negative = 0, negative_abs_mask = 0xFFFF;
        uint32_t temp_result = 0, temp_result_decimal = 0;

        uint16_t x_g_data = 0, y_g_data = 0, z_g_data = 0;
        uint16_t x_ang_speed = 0, y_ang_speed = 0, z_ang_speed = 0;
        uint16_t x_ang_diff = 0, y_ang_diff = 0, z_ang_diff = 0;

        // x => pitch
        x_g_data = (tdfInfo->usReceivedValue[6] | (tdfInfo->usReceivedValue[7] << 8)); 
        if( negative_mask & x_g_data ) {
            is_negative = 1;
            x_g_data = (negative_abs_mask ^ x_g_data) + 1;
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] x_g_data negative ");
        } 
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] x_g_data = <%d>  ",x_g_data);

        temp_result = x_g_data;
        temp_result = temp_result * FS_TYPO / FS_TYPO_decimal;
        temp_result = temp_result / total_decimal;
        temp_result = temp_result * ms_100 / ms_decimal;

        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] temp_result = <%d>  ",temp_result);

        if( is_negative == 1){
            tdf_3d_pose_obj.pitch -= (temp_result );
        } else {
            tdf_3d_pose_obj.pitch += (temp_result );
        }

        // y => roll
        y_g_data = (tdfInfo->usReceivedValue[8] | (tdfInfo->usReceivedValue[9] << 8));

        if( negative_mask & y_g_data ) {
            is_negative = 1;
            y_g_data = (negative_abs_mask ^  y_g_data) + 1;
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] y_g_data negative ");
        }
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] y_g_data = <%d>  ",y_g_data);

        temp_result = y_g_data;
        temp_result = temp_result * FS_TYPO / FS_TYPO_decimal;
        temp_result = temp_result / total_decimal;
        temp_result = temp_result * ms_100 / ms_decimal;

        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] temp_result = <%d>  ",temp_result);

        if( is_negative == 1){
            tdf_3d_pose_obj.roll -= (temp_result);
        } else {
            tdf_3d_pose_obj.roll += (temp_result);
        }

        // z => yaw
        z_g_data = (tdfInfo->usReceivedValue[10] | (tdfInfo->usReceivedValue[11] << 8));
        if( negative_mask & z_g_data ) {
            is_negative = 1;
            z_g_data = (negative_abs_mask ^ z_g_data) + 1;
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] z_g_data negative ");
        }

        temp_result = z_g_data;
        temp_result = temp_result * FS_TYPO / FS_TYPO_decimal;
        temp_result = temp_result / total_decimal;
        temp_result = temp_result * ms_100 / ms_decimal;

        if( is_negative == 1){
            tdf_3d_pose_obj.yaw -= (temp_result);
        } else {
            tdf_3d_pose_obj.yaw += (temp_result);
        }

        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[BLE Event] tdf_3d_pose_obj.pitch  = [%d]",tdf_3d_pose_obj.pitch);
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[BLE Event] tdf_3d_pose_obj.roll  = [%d]",tdf_3d_pose_obj.roll);
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[BLE Event] tdf_3d_pose_obj.yaw  = [%d]",tdf_3d_pose_obj.yaw);
    }

    if( mode_state == BLE_TDF_CONTINUOUS_STOP ) {
	    eTdfAddMulti(BLE_LOG, TDF_3D_POSE, TDF_TIMESTAMP_NONE, NULL, &tdf_3d_pose_obj);
	    eTdfFlushMulti(BLE_LOG);
        is_started = 0;
    }
    
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
static void handle_continuous_mode(xBLETdfMessage_t* tdfInfo,uint16_t tdfId,eBLETdfContinuousModeState_t mode_state) {
    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[BLE Event] tdfInfo->usHCIMapSID = <%d>  ",tdfInfo->usHCIMapSID);
    if( tdfInfo != NULL ) {
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
                handle_tdf_472(tdfInfo,mode_state);
                break;
            case BLE_TDF_475_SID:
                handle_tdf_475(tdfInfo);
                break;
            default:
                break;
        }
    } else {
        switch(tdfId){
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
                    handle_tdf_472(NULL,mode_state);
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
                    break;
        }
    }
}
