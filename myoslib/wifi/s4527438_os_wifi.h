/** 
 **************************************************************
 * @file mylib/s4527438_os_wifi.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib wifi driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */

#ifndef S4527438_OS_WIFI_H_
#define S4527438_OS_WIFI_H_

#include "s4527438_hal_wifi.h"

void s4527438_os_wifi_init(void);
void s4527438_os_wifi_send_cmd(uint8_t *cmd,uint8_t cmd_len);
#endif /* S4527438_OS_HCI_H_ */
