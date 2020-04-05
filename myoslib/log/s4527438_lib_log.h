/** 
 **************************************************************
 * @file mylib/cli/s4527438_lib_cli.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */

#ifndef S4527438_LIB_LOG_H_
#define S4527438_LIB_LOG_H_

#include "s4527438_os_log.h"

void s4527438_lib_log_init(void);
void s4527438_LOGGER( Log_Level_Enum log_level, const char *pcFormat, ... );

#endif /* S4527438_LIB_LOG_H_ */
