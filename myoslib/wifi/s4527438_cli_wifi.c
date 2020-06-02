/** 
 **************************************************************
 * @file mylib/s4527438_cli_wifi.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli wifi driver
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

#include "s4527438_lib_log.h"
#include "esp_wifi.h"

#include "memory_operations.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static BaseType_t prvWifiATCmdSendCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

CLI_Command_Definition_t xWifiATCmdSend = {  /* Structure that defines the " lsm6dsl r x" command line command. */
    "wc",
    "wc: wc \r\n",
    prvWifiATCmdSendCommand,
    1
};

void s4527438_cli_wifi_init(void) {
    /* Register CLI commands */
    FreeRTOS_CLIRegisterCommand(&xWifiATCmdSend);
}

static BaseType_t prvWifiATCmdSendCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;

    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string == NULL ) {
        return returnedValue;
    }
    if( lParam_len == 0 ) {
        return returnedValue;
    }
    eEspSendRAWCommnd(cCmd_string,lParam_len);
    return returnedValue;
}
