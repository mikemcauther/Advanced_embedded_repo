/**
 **************************************************************
 * @file mylib/s4527438_hal_hci.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib HCI driver
 * REFERENCE:
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 ***************************************************************
 */

#ifndef S4527438_HAL_HCI_H_
#define S4527438_HAL_HCI_H_

/**<@brief Base Message Structure & Metadata */
typedef struct xHCICommsMessage_t
{
    const uint8_t *pucPayload;   /**< Message payload */
    uint8_t       usPayloadLen; /**< Message payload length */
} ATTR_PACKED xHCICommsMessage_t;

typedef struct xHCICommsInterface_t xHCICommsInterface_t;

typedef void ( *eHCICommsInit_t )( void );
typedef void ( *eHCICommsEnable_t )( bool bEnable );
typedef void ( *eHCICommsSend_t )( xHCICommsMessage_t *pxMessage );
typedef void ( *vHCICommsReceiveHandler_t )( xHCICommsInterface_t *			pxComms,
										  xHCICommsMessage_t *		pxMessage );

struct xHCICommsInterface_t
{
    eHCICommsInit_t           fnInit;                      /**< Initialisation function. */
    eHCICommsEnable_t         fnEnable;                    /**< Enable/disable function. */
    eHCICommsSend_t           fnSend;                      /**< Send function. */
    vHCICommsReceiveHandler_t fnReceiveHandler;            /**< Function to call on packet reception. */
};

void vHCIPacketBuilder( char cByte );

extern xHCICommsInterface_t xHCIComms;
#endif /* S4527438_HAL_HCI_H_ */
