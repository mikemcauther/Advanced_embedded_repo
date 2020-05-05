/** 
 **************************************************************
 * @file mylib/s4527438_os_ble.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib ble driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */

#ifndef S4527438_OS_BLE_H_
#define S4527438_OS_BLE_H_

#define BLE_TDF_VALUE_MAX_SIZE      12

typedef struct xBLETdfMessage_t
{
    uint8_t             usTdfIndex; /**< Message Tdf Index */
    uint8_t             usTdfFieldBit; /**< Message Tdf Field Index */
    uint8_t             usHCIMapSID;
    uint8_t             usI2CRegAddr;
    uint8_t             usI2CRegValue;
    uint8_t             usTdfReceivedValueLength; /**< Field usReceivedValue  length */
    uint8_t             usReceivedValue[BLE_TDF_VALUE_MAX_SIZE];   /**< Message payload */
} ATTR_PACKED xBLETdfMessage_t;

void s4527438_os_ble_init(void);
void s4527438_os_ble_tdf_get_sensor_value_cmd(uint16_t tdfId);
void s4527438_os_ble_tdf_result_cmd(xBLETdfMessage_t *tdfInfo);
#endif /* S4527438_OS_BLE_H_ */
