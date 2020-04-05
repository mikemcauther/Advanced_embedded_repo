/** 
 **************************************************************
 * @file mylib/s4527438_os_log.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib log driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */

#ifndef S4527438_OS_LOG_H_
#define S4527438_OS_LOG_H_

typedef enum{
    MY_OS_LIB_LOG_LEVEL_ERROR = 0,
    MY_OS_LIB_LOG_LEVEL_LOG = 1,
    MY_OS_LIB_LOG_LEVEL_DEBUG = 2,
} Log_Level_Enum;

void s4527438_os_log_init(void);
void s4527438_os_log_set_log_level_cmd(Log_Level_Enum log_level);
void s4527438_os_log_buffer_send_cmd(Log_Level_Enum log_level, const char *pcFormat, va_list va );
#endif /* S4527438_OS_LOG_H_ */
