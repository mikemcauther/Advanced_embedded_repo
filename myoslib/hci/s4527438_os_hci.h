/** 
 **************************************************************
 * @file mylib/s4527438_os_hci.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib hci driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */

#ifndef S4527438_OS_HCI_H_
#define S4527438_OS_HCI_H_

#include "s4527438_hal_hci.h"

void s4527438_os_hci_init(void);
void s4527438_os_hci_write_cmd(xHCICommsMessage_t *pxMessage);
#endif /* S4527438_OS_HCI_H_ */
