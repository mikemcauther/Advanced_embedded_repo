/** 
 **************************************************************
 * @file mylib/s4527438_cli_ble.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli ble driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"

#include "FreeRTOS_CLI.h"

#include "s4527438_os_ble.h"
#include "s4527438_lib_log.h"
#include "unified_comms_bluetooth.h"

#include "memory_operations.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static BaseType_t prvBLESysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );


CLI_Command_Definition_t xBLESys = {  /* Structure that defines the " lsm6dsl r x" command line command. */
    "ble",
    "ble: ble \r\n",
    prvBLESysCommand,
    2
};

void s4527438_cli_ble_init(void) {
    /* Register CLI commands */
    FreeRTOS_CLIRegisterCommand(&xBLESys);
}


static BaseType_t prvBLESysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;

    uint8_t sub_cmd = 's';
    uint8_t scan_switch = 'o';
    uint16_t tdf_idx = 0;
    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string == NULL ) {
        return returnedValue;
    }
    sub_cmd = cCmd_string[0];
    if( (sub_cmd != 's') && (sub_cmd != 'c') ) {
        return returnedValue;
    }

    /* Get parameters 2 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lParam_len);

    if( cCmd_string == NULL ) {
        return returnedValue;
    }
    scan_switch = cCmd_string[0];

    if( sub_cmd == 's' ) {
        if( scan_switch == 'o' ) {
            xBluetoothComms.fnEnable(true);
        } else if (scan_switch == 'f') {
            xBluetoothComms.fnEnable(false);
        }
    }

    if( sub_cmd == 'c' ) {
        if( cCmd_string != NULL ) {
            int i = 0;
            uint8_t numeric_string[4];

            memset(numeric_string,0x00,sizeof(numeric_string));
            for( i = 0; i < lParam_len ;i++) {
                if( cCmd_string[i] < '0'
                    || cCmd_string[i] > '9' ) {
                    return returnedValue;
                }
                numeric_string[i] = cCmd_string[i];
            }
            tdf_idx = atoi(numeric_string);
        } else {
            return returnedValue;
        }
        s4527438_os_ble_tdf_get_sensor_value_cmd(tdf_idx);
    }


    return returnedValue;
}
