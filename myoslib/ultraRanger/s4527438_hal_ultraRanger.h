/**
 **************************************************************
 * @file mylib/s4527438_hal_ultraRanger.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib ultraRanger driver
 * REFERENCE:
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 ***************************************************************
 */

#ifndef S4527438_HAL_ULTRARANGER_H_
#define S4527438_HAL_ULTRARANGER_H_

#include "s4527438_hci_packet.h"

void s4527438_hal_ultraRanger_init(void);
uint32_t s4527438_hal_ultraRanger_read_distance();

#endif /* S4527438_HAL_ULTRARANGER_H_ */
