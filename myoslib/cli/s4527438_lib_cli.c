/** 
 **************************************************************
 * @file mylib/cli/s4527438_lib_cli.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "log.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"

#include "device_nvm.h"
#include "unified_comms_serial.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define INPUT_STRING_BUFFER_LEN    60
/* Task Priorities ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void vCustomSerialHandler(xCommsInterface_t *pxComms,
                          xUnifiedCommsIncomingRoute_t *pxCurrentRoute,
                          xUnifiedCommsMessage_t *pxMessage);
static void cli_processor(char *cInputString,size_t cInputStringLen);

void s4527438_lib_cli_init(void) {
    /* Setup our serial receive handler */
    xSerialComms.fnReceiveHandler = vCustomSerialHandler;
    vUnifiedCommsListen(&xSerialComms, COMMS_LISTEN_ON_FOREVER);
}

void s4527438_lib_cli_deinit(void) {
}

void vCustomSerialHandler(xCommsInterface_t *pxComms,
                          xUnifiedCommsIncomingRoute_t *pxCurrentRoute,
                          xUnifiedCommsMessage_t *pxMessage)
{
    char pcLocalString[INPUT_STRING_BUFFER_LEN] = {0};
    UNUSED(pxCurrentRoute);
    UNUSED(pxComms);

    /*
     * Copy the string to a local buffer so it can be NULL terminated properly
     * The %s format specifier does not respect provided lengths
     */
    pvMemcpy(pcLocalString, pxMessage->pucPayload, pxMessage->usPayloadLen);

    eLog(LOG_APPLICATION, LOG_INFO, "\r\nReceived PKT:\r\n");
    eLog(LOG_APPLICATION, LOG_INFO, "\t  Type: %02X\r\n", pxMessage->xPayloadType);
    eLog(LOG_APPLICATION, LOG_INFO, "\tString: %s\r\n\r\n", pcLocalString);

    cli_processor(pcLocalString,sizeof(pcLocalString));
}

static void cli_processor(char *cInputString,size_t cInputStringLen)
{
    char *pcOutputString;
    BaseType_t xReturned;

    pcOutputString = FreeRTOS_CLIGetOutputBuffer();
    xReturned = pdTRUE;

    /* Process command input string. */
    while (xReturned != pdFALSE) {
        pcOutputString = FreeRTOS_CLIGetOutputBuffer();
        /* Returns pdFALSE, when all strings have been returned */
        xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

        /* Display CLI command output string (not thread safe) */
        if(strlen(pcOutputString) > 0 ) {
            eLog(LOG_APPLICATION, LOG_INFO, "\tCommand Result String: %s\r\n\r\n", pcOutputString);
            memset(pcOutputString,0x00,strlen(pcOutputString));
        }
     //vTaskDelay(5);  //Must delay between debug_printfs.
    }    
}


