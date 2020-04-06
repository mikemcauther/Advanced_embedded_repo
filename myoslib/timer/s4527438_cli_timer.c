/** 
 **************************************************************
 * @file mylib/s4527438_cli_timer.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli timer driver
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

#include "FreeRTOS_CLI.h"
#include "rtc.h"

#include "s4527438_lib_log.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static BaseType_t prvTimeGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvTimeFGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

CLI_Command_Definition_t xTimeGetSys = {  /* Structure that defines the "pan" command line command. */
    "time",
    "time: time \r\n",
    prvTimeGetSysCommand,
    0
};

CLI_Command_Definition_t xTimeFGetSys = {  /* Structure that defines the "pan" command line command. */
    "time",
    "time: time [f] \r\n",
    prvTimeFGetSysCommand,
    1
};

void s4527438_cli_time_init(void) {
    /* Register CLI commands */
    /* NOTE: The order is matter, command with more parameters need to be registered. */
    FreeRTOS_CLIRegisterCommand(&xTimeGetSys);
    FreeRTOS_CLIRegisterCommand(&xTimeFGetSys);
}


static BaseType_t prvTimeGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
    // No parameter , just seconds formate
    uint32_t     pulEpochTime = 0;
    BaseType_t returnedValue = pdFALSE;

    bRtcGetEpochTime( eUnixEpoch, &pulEpochTime );
    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Systime: %05d",pulEpochTime);
    return returnedValue;
}

static BaseType_t prvTimeFGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;

    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string != NULL ) {
        switch(cCmd_string[0])
        {
            case 'f':
                {
                    xDateTime_t xDatetime;
                    bRtcGetDatetime(&xDatetime);

                    uint32_t     usSubSecond = xDatetime.xTime.usSecondFraction;
                    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"Formate time: %02d:%02d:%02d.%05d",
                                                                                            xDatetime.xTime.ucHour,
                                                                                            xDatetime.xTime.ucMinute,
                                                                                            xDatetime.xTime.ucSecond,
                                                                                            usSubSecond
                                                                                            );
                }
                break;
            default:
                s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"Error for time cmd: Unknown parameter!!");
                return returnedValue;
        }
    } else {
        return returnedValue;
    }

    return returnedValue;
}
