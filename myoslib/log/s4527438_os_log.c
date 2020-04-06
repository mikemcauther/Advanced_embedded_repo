/** 
 **************************************************************
 * @file mylib/s4527438_os_log.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib log driver
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
#include "tiny_printf.h"

#include "s4527438_os_log.h"

/* Private typedef -----------------------------------------------------------*/

#define MESSAGE_LOG_STRING_BUFFER_SIZE      60

typedef enum{
    MESSAGE_SEND_LOG_TYPE,
    MESSAGE_LOG_CMD_SET_LOG_LEVEL_TYPE,
} Message_Type_Enum;

struct MyOsLibLogMessage {    /* Message consists of sequence number and payload string */
    Message_Type_Enum   MessageType;

    Log_Level_Enum      log_level;
    uint8_t             log_buffer[MESSAGE_LOG_STRING_BUFFER_SIZE];
    uint8_t             used_buffer_size;
};
/* Private define ------------------------------------------------------------*/
#define mainLOG_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )
#define mainLog_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )

#define QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK 100

#define QUEUE_LENGTH                        50

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TaskHandle_t xTaskLogOsHandle;
static SemaphoreHandle_t s4527438QueueLogPacketSend = NULL;
static QueueSetHandle_t xQueueSet;

static Log_Level_Enum currnetLogLevel = MY_OS_LIB_LOG_LEVEL_ERROR ;  

/* Private function prototypes -----------------------------------------------*/
static void LogTask( void );


void s4527438_os_log_init(void) {

    s4527438QueueLogPacketSend = xQueueCreate(QUEUE_LENGTH, sizeof(struct MyOsLibLogMessage));
    xQueueSet = xQueueCreateSet(QUEUE_LENGTH);
    xQueueAddToSet(s4527438QueueLogPacketSend, xQueueSet);

    currnetLogLevel = MY_OS_LIB_LOG_LEVEL_ERROR;  
    
    xTaskCreate( (void *) &LogTask, (const signed char *) "LogTask", mainLog_TASK_STACK_SIZE, NULL, mainLOG_TASK_PRIORITY, &xTaskLogOsHandle );
}

void s4527438_os_log_set_log_level_cmd(Log_Level_Enum log_level) {
    struct MyOsLibLogMessage SendMessage;

    SendMessage.MessageType = MESSAGE_LOG_CMD_SET_LOG_LEVEL_TYPE;
    SendMessage.log_level = log_level;

    if (s4527438QueueLogPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueLogPacketSend, ( void * ) &SendMessage, ( portTickType ) 0 );
    }
}

void s4527438_os_log_buffer_send_cmd(Log_Level_Enum log_level, const char *pcFormat, va_list va )
{
    struct MyOsLibLogMessage SendMessage;

    SendMessage.MessageType = MESSAGE_SEND_LOG_TYPE;
    SendMessage.log_level = log_level;
    SendMessage.used_buffer_size = 0;

    char * pcString = SendMessage.log_buffer;
    SendMessage.used_buffer_size = tiny_vsnprintf( (char *) pcString, MESSAGE_LOG_STRING_BUFFER_SIZE - 1, pcFormat, va );

    if (s4527438QueueLogPacketSend != NULL) { /* Check if queue exists */
        xQueueSend(s4527438QueueLogPacketSend, ( void * ) &SendMessage, ( portTickType ) 0 );
    }
    return;
}

static void LogTask( void ) {
    struct MyOsLibLogMessage RecvMessage;
    QueueSetMemberHandle_t xActivatedMember;

    char * pcString = NULL;
    uint32_t ulMaxLen;
    int32_t     total_len = RecvMessage.used_buffer_size;

    for (;;) {

        xActivatedMember = xQueueSelectFromSet(xQueueSet, QUEUE_BLOCK_TIME_AS_FSM_PERIOD_TICK);

        if (xActivatedMember == s4527438QueueLogPacketSend) {  
            /* Receive item */
            xQueueReceive( s4527438QueueLogPacketSend, &RecvMessage, 0 );
            switch(RecvMessage.MessageType)
            {
                case MESSAGE_SEND_LOG_TYPE:
                    if( RecvMessage.log_level > currnetLogLevel ) {
                        break;
                    }

                    pcString = pxSerialOutput->pxImplementation->fnClaimBuffer( pxSerialOutput->pvContext, &ulMaxLen );

                    // Check Buffer size
                    if( ulMaxLen < (strlen(ANSI_COLOR_RED) + strlen(ANSI_COLOR_RESET) + RecvMessage.used_buffer_size)  ) {
                        pxSerialOutput->pxImplementation->fnReleaseBuffer( pxSerialOutput->pvContext, pcString );
                        break; 
                    }

                    char *bufferPtr = pcString;
                    switch( RecvMessage.log_level ){
                        case MY_OS_LIB_LOG_LEVEL_ERROR:

                            memcpy(bufferPtr,ANSI_COLOR_RED,strlen(ANSI_COLOR_RED));
                            bufferPtr += strlen(ANSI_COLOR_RED);

                            break;
                        case MY_OS_LIB_LOG_LEVEL_LOG:

                            memcpy(bufferPtr,ANSI_COLOR_GREEN,strlen(ANSI_COLOR_GREEN));
                            bufferPtr += strlen(ANSI_COLOR_GREEN);

                            break;
                        case MY_OS_LIB_LOG_LEVEL_DEBUG:
                            memcpy(bufferPtr,ANSI_COLOR_BLUE,strlen(ANSI_COLOR_BLUE));
                            bufferPtr += strlen(ANSI_COLOR_BLUE);
                            break;
                        default:
                            break;
                    }
                    memcpy(bufferPtr,RecvMessage.log_buffer,RecvMessage.used_buffer_size);
                    bufferPtr += RecvMessage.used_buffer_size;

                    memcpy(bufferPtr,ANSI_COLOR_RESET,strlen(ANSI_COLOR_BLUE));
                    bufferPtr += strlen(ANSI_COLOR_RESET);
                    *(bufferPtr++) = '\0';

                    pxSerialOutput->pxImplementation->fnSendBuffer( pxSerialOutput->pvContext, pcString, (bufferPtr - pcString) );
                    pxSerialOutput->pxImplementation->fnReleaseBuffer( pxSerialOutput->pvContext, pcString );
                    break;
                case MESSAGE_LOG_CMD_SET_LOG_LEVEL_TYPE:
                    currnetLogLevel = RecvMessage.log_level;
                    break;
                default:
                    break;
            }
        }

        /* Delay for 10ms */
        vTaskDelay(10);
    }
}

