/** 
 **************************************************************
 * @file mylib/cli/s4527438_lib_log.c
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

#include "tiny_printf.h"

/* Scheduler includes. */
#include "s4527438_os_log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Task Priorities ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void s4527438_lib_log_init(void) {
}

void s4527438_LOGGER( Log_Level_Enum log_level, const char *pcFormat, ... ) {
    va_list        va;

    va_start( va, pcFormat );
    s4527438_os_log_buffer_send_cmd( log_level, pcFormat, va );
    va_end( va );
}


