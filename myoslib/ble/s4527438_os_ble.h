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
    uint16_t            usTdfIndex; /**< Message Tdf Index */
    uint8_t             usTdfFieldBit; /**< Message Tdf Field Index */
    uint8_t             usHCIMapSID;
    uint8_t             usI2CRegAddr;
    uint8_t             usI2CRegValue;
    uint8_t             usTdfReceivedValueLength; /**< Field usReceivedValue  length */
    uint8_t             usReceivedValue[BLE_TDF_VALUE_MAX_SIZE];   /**< Message payload */
} ATTR_PACKED xBLETdfMessage_t;

typedef enum eBLETdfContinuousModeState_t {
    BLE_TDF_CONTINUOUS_STOP = 0,
    BLE_TDF_CONTINUOUS_START = 1,
    BLE_TDF_CONTINUOUS_NO_ACTION = 2,
    BLE_TDF_CONTINUOUS_DATA_INCOMING = 4
} eBLETdfContinuousModeState_t;

void s4527438_os_ble_init(void);
void s4527438_os_ble_tdf_get_sensor_value_cmd(uint16_t tdfId);
void s4527438_os_ble_tdf_result_cmd(xBLETdfMessage_t *tdfInfo);

void s4527438_os_ble_tdf_continuous_mode(uint16_t tdfId,eBLETdfContinuousModeState_t mode_state);
#endif /* S4527438_OS_BLE_H_ */
