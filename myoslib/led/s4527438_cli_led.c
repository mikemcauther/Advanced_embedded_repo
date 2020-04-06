/** 
 **************************************************************
 * @file mylib/s4527438_cli_led.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli led driver
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

#include "s4527438_os_led.h"
#include "s4527438_lib_log.h"
#include "log.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static BaseType_t prvLEDGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

CLI_Command_Definition_t xLEDGetSys = {  /* Structure that defines the "pan" command line command. */
    "led",
    "led: led \r\n",
    prvLEDGetSysCommand,
    2
};

void s4527438_cli_led_init(void) {
    /* Register CLI commands */
    FreeRTOS_CLIRegisterCommand(&xLEDGetSys);
}

static BaseType_t prvLEDGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    char *target_string = NULL;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;

    LED_Cmd_Enum led_cmd = MY_OS_LIB_LED_CMD_NONE;
    LED_Color_Enum led_color = MY_OS_LIB_LED_COLOR_NONE;


    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string != NULL ) {
        switch(cCmd_string[0])
        {
            case 'o':
                led_cmd = MY_OS_LIB_LED_CMD_ON;
                break;
            case 'f':
                led_cmd = MY_OS_LIB_LED_CMD_OFF;
                break;
            case 't':
                led_cmd = MY_OS_LIB_LED_CMD_TOGGLE;
                break;
            default:
                s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"Error for led cmd: Wrong cmd!!");
                return returnedValue;
        }
    } else {
        return returnedValue;
    }

    /* Get parameters 2 from command string */

    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lParam_len);
    if( cCmd_string != NULL ) {
        switch(cCmd_string[0])
        {
            case 'r':
                led_color = MY_OS_LIB_LED_COLOR_RED;
                break;
            case 'g':
                led_color = MY_OS_LIB_LED_COLOR_GREEN;
                break;
            case 'b':
                led_color = MY_OS_LIB_LED_COLOR_BLUE;
                break;
            default:
                s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"Error for led cmd: Wrong color!!");
                return returnedValue;
        }
    } else {
        return returnedValue;
    } 

    s4527438_os_led_command(led_cmd,led_color);

    return returnedValue;
}
