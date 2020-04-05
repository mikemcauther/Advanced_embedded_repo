/** 
 **************************************************************
 * @file mylib/s4527438_cli_log.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli log driver
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
#include "queue.h"

#include "FreeRTOS_CLI.h"

#include "s4527438_os_log.h"
#include "s4527438_lib_log.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static BaseType_t prvLogGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

CLI_Command_Definition_t xLogGetSys = {  /* Structure that defines the "pan" command line command. */
    "log",
    "log: log \r\n",
    prvLogGetSysCommand,
    1
};

void s4527438_cli_log_init(void) {
    /* Register CLI commands */
    FreeRTOS_CLIRegisterCommand(&xLogGetSys);
}

static BaseType_t prvLogGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;

    Log_Level_Enum log_level = MY_OS_LIB_LOG_LEVEL_ERROR;

    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string != NULL ) {
        switch(cCmd_string[0])
        {
            case 'e':
                log_level = MY_OS_LIB_LOG_LEVEL_ERROR;
                break;
            case 'l':
                log_level = MY_OS_LIB_LOG_LEVEL_LOG;
                break;
            case 'd':
                log_level = MY_OS_LIB_LOG_LEVEL_DEBUG;
                break;
            default:
                s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"Error for led cmd: Wrong log level!!");
                return returnedValue;
        }
    } else {
        return returnedValue;
    }

    s4527438_os_log_set_log_level_cmd(log_level);

    return returnedValue;
}
