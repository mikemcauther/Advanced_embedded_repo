/**
 **************************************************************
 * @file myoslib/s4527438_hal_hci.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib HCI driver
 * REFERENCE:
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 ***************************************************************
 */
/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "uart.h"
#include "s4527438_hci_packet.h"
#include "s4527438_hal_hci.h"
#include "s4527438_lib_log.h"

typedef struct xHCIInterfaceHeader_t
{
    uint8_t  ucPreamble;
    uint8_t  usType4AndPayloadLen4;
} ATTR_PACKED xHCIInterfaceHeader_t;

/* Function Declarations ------------------------------------*/

void eHCICommsInit( void );
void eHCICommsEnable( bool bEnable );
void eHCICommsSend( xHCICommsMessage_t *pxMessage );

/* Private Variables ----------------------------------------*/

static const uint8_t pucSyncBytes[1] = { HCI_PACKET_FIELD_PREAMBLE };

static uint8_t pucRxBuffer[MAX_PACKET_BUFFER];

xHCICommsInterface_t xHCIComms = {
    .fnInit           = eHCICommsInit,
    .fnEnable         = eHCICommsEnable,
    .fnSend           = eHCICommsSend,
    .fnReceiveHandler = NULL
};

/*-----------------------------------------------------------*/

void eHCICommsInit( void )
{
    return;
}

/*-----------------------------------------------------------*/

void eHCICommsEnable( bool bEnable )
{
    if ( bEnable ) {
        pxHCIOutput->pxImplementation->fnEnable( pxHCIOutput->pvContext );
    }
    else {
        pxHCIOutput->pxImplementation->fnDisable( pxHCIOutput->pvContext );
    }
    return;
}

/*-----------------------------------------------------------*/

void eHCICommsSend( xHCICommsMessage_t *pxMessage )
{
    static xHCIInterfaceHeader_t xInterfaceHeader = {
        .ucPreamble      = HCI_PACKET_FIELD_PREAMBLE,
        .usType4AndPayloadLen4 = 0,
    };
    char *   pcBuffer;
    uint32_t ulBufferLength, ulTotalLen;

    /* Desination address is ignored as serial is point-to-point */
    xInterfaceHeader.usType4AndPayloadLen4 = HCI_PACKET_FIELD_TYPE_REQUEST_WITH_LENGTH(pxMessage->usPayloadLen);
    ulTotalLen                    = sizeof( xHCIInterfaceHeader_t ) + HCI_PACKET_FIELD_TYPE_LEN_GET_LENGTH(xInterfaceHeader.usType4AndPayloadLen4);

    /* Get the buffer to put formatted data into */
    pcBuffer = pxHCIOutput->pxImplementation->fnClaimBuffer( pxHCIOutput->pvContext, &ulBufferLength );
    /* Check the data we want to send fits in our UART buffer */
    if ( ulBufferLength < ulTotalLen ) {
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[HCI Event][Send]: [%s]",pcBuffer);
        return;
    }
    xBufferBuilder_t xPacketBuilder;
    vBufferBuilderStart( &xPacketBuilder, (uint8_t *) pcBuffer, ulBufferLength );
    /* Pack the data */
    vBufferBuilderPushData( &xPacketBuilder, &xInterfaceHeader, sizeof( xHCIInterfaceHeader_t ) );
    vBufferBuilderPushData( &xPacketBuilder, pxMessage->pucPayload, pxMessage->usPayloadLen );
    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[HCI Event][Send]: [%x]",pcBuffer);
    /* Push the message at the UART driver */
    pxHCIOutput->pxImplementation->fnSendBuffer( pxHCIOutput->pvContext, pcBuffer, xPacketBuilder.ulIndex );
    /* Increment the sequnce number for next message */
    return;
}

/*-----------------------------------------------------------*/

void s4527438_hal_hci_init(void) {
    xHCIComms.fnEnable( true );
}

void vHCIPacketBuilder( char cByte )
{
    static xHCIInterfaceHeader_t xHCIHeader        = { 0x00 };
    static uint16_t                 usRxByteCount        = 0;

    //eLog( LOG_APPLICATION, LOG_APOCALYPSE, "HCI Recv: %X\r\n", (uint8_t) cByte );

    configASSERT( usRxByteCount < MAX_PACKET_BUFFER );
    uint16_t usCurrentByte       = usRxByteCount;
    pucRxBuffer[usRxByteCount++] = (uint8_t) cByte;

    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[HCI Event]: value = [%c]",cByte);
    if ( xHCIComms.fnReceiveHandler == NULL ) {
        return;
    }

    /* If we're looking for SYNC bytes */
    if ( usRxByteCount <= 1 ) {
        /* Check that sync bytes match */
        if ( pucRxBuffer[usCurrentByte] != pucSyncBytes[usCurrentByte] ) {
            usRxByteCount = 0;
        }
    }
    /* If the header is complete */
    else if ( usRxByteCount == sizeof( xHCIInterfaceHeader_t ) ) {
        pvMemcpy( &xHCIHeader, pucRxBuffer, sizeof( xHCIInterfaceHeader_t ) );
    }
    /* If the complete packet has arrived */
    else if ( usRxByteCount == ( sizeof( xHCIInterfaceHeader_t ) + HCI_PACKET_FIELD_TYPE_LEN_GET_LENGTH(xHCIHeader.usType4AndPayloadLen4) ) ) {
        xHCICommsMessage_t xMessage = {
            .usPayloadLen = HCI_PACKET_FIELD_TYPE_LEN_GET_LENGTH(xHCIHeader.usType4AndPayloadLen4)
        };
        pvMemcpy( xMessage.pucPayload, pucRxBuffer + sizeof( xHCIInterfaceHeader_t ), xMessage.usPayloadLen );
        xHCIComms.fnReceiveHandler( &xHCIComms, &xMessage );
        usRxByteCount        = 0;
    }
}

