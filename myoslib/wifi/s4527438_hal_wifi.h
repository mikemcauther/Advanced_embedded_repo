/**
 **************************************************************
 * @file mylib/s4527438_hal_wifi.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib wifi driver
 * REFERENCE:
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 ***************************************************************
 */

#ifndef S4527438_HAL_WIFI_H_
#define S4527438_HAL_WIFI_H_

#include "s4527438_wifi_packet.h"

#define MESSAGE_WIFI_STRING_BUFFER_SIZE      60

typedef struct xWifiCommsMessage_t
{
    uint8_t                     usPayloadLen; /**< Message payload length */
    uint8_t                     pucPayload[MESSAGE_WIFI_STRING_BUFFER_SIZE];   /**< Message payload */
} ATTR_PACKED xWifiCommsMessage_t;

typedef struct xWifiCommsInterface_t xWifiCommsInterface_t;

typedef void ( *eWifiCommsInit_t )( void );
typedef void ( *eWifiCommsEnable_t )( bool bEnable );
typedef void ( *eWifiCommsSend_t )( xWifiCommsMessage_t *pxMessage );
typedef void ( *vWifiCommsReceiveHandler_t )( xWifiCommsInterface_t *			pxComms,
										  xWifiCommsMessage_t *		pxMessage );

struct xWifiCommsInterface_t
{
    eWifiCommsInit_t           fnInit;                      /**< Initialisation function. */
    eWifiCommsEnable_t         fnEnable;                    /**< Enable/disable function. */
    eWifiCommsSend_t           fnSend;                      /**< Send function. */
    vWifiCommsReceiveHandler_t fnReceiveHandler;            /**< Function to call on packet reception. */
};

void vWifiPacketBuilder( char cByte );
void s4527438_hal_wifi_init(void);

extern xWifiCommsInterface_t xWifiComms;
#endif /* S4527438_HAL_WIFI_H_ */
