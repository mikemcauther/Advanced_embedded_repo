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
#include "s4527438_os_ble.h"

#define HCI_CMD_PAYLOAD_SIZE      60

typedef enum eHCIPacketRoute_t {
    HCI_ROUTE_BLE_TDF,
    HCI_ROUTE_SERIAL_OUTPUT 
} eHCIPacketRoute_t;

typedef struct xHCIReceiveRoute_t
{
    uint8_t             usPayloadLen; /**< Message payload length */
    eHCIPacketRoute_t   usPayloadRoute; /**< Message payload route */
    uint8_t             usPayloadWordInfo; /**< Message payload route */
    uint8_t             pucPayload[HCI_CMD_PAYLOAD_SIZE];   /**< Message payload */
} ATTR_PACKED xHCIReceiveRoute_t;

void s4527438_os_hci_init(void);
void s4527438_os_hci_write_reg_cmd(uint8_t  sid, uint8_t I2CRegAddr, uint8_t I2CRegValue,uint8_t num_word,uint8_t word_size);
void s4527438_os_hci_read_reg_cmd(uint8_t  sid, uint8_t I2CRegAddr, uint8_t I2CRegValue,uint8_t num_word,uint8_t word_size);
void s4527438_os_hci_tdf_read_cmd(uint8_t num_word,uint8_t word_size,xBLETdfMessage_t* tdfInfo);
#endif /* S4527438_OS_HCI_H_ */
