/**
 **************************************************************
 * @file myoslib/s4527438_hal_hci.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib wifi driver
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
#include "s4527438_wifi_packet.h"
#include "s4527438_hal_wifi.h"
#include "s4527438_lib_log.h"

/* Function Declarations ------------------------------------*/

void eWifiCommsInit( void );
void eWifiCommsEnable( bool bEnable );
void eWifiCommsSend( xWifiCommsMessage_t *pxMessage );

/* Private Variables ----------------------------------------*/

xWifiCommsInterface_t xWifiComms = {
    .fnInit           = eWifiCommsInit,
    .fnEnable         = eWifiCommsEnable,
    .fnSend           = eWifiCommsSend,
    .fnReceiveHandler = NULL
};

/*-----------------------------------------------------------*/

void eWifiCommsInit( void )
{
    return;
}

/*-----------------------------------------------------------*/

void eWifiCommsEnable( bool bEnable )
{
    if ( bEnable ) {
        pxWifiATCmdOutput->pxImplementation->fnEnable( pxWifiATCmdOutput->pvContext );
    }
    else {
        pxWifiATCmdOutput->pxImplementation->fnDisable( pxWifiATCmdOutput->pvContext );
    }
    return;
}

/*-----------------------------------------------------------*/

static void fnWriteWrapper( void *pvContext, const char *pcFormat, ... ) {
    va_list        va;

    va_start( va, pcFormat );
	pxWifiATCmdOutput->pxImplementation->fnWrite( pvContext, pcFormat,va);
    va_end( va );
}

void eWifiCommsSend( xWifiCommsMessage_t *pxMessage )
{
    char *   pcBuffer;
    uint32_t ulBufferLength, ulTotalLen;

    /*********Send Length to AT command firstly***********/
	fnWriteWrapper( pxWifiATCmdOutput->pvContext, "AT+CIPSEND=%d",pxMessage->usPayloadLen);

    /*********Send DATA***********/
    ulTotalLen                    = pxMessage->usPayloadLen;

    /* Get the buffer to put formatted data into */
    pcBuffer = pxWifiATCmdOutput->pxImplementation->fnClaimBuffer( pxWifiATCmdOutput->pvContext, &ulBufferLength );
    /* Check the data we want to send fits in our UART buffer */
    if ( ulBufferLength < ulTotalLen ) {
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"[WIFI Event][Send]: buffer len <%d> too small!!",ulBufferLength);
        return;
    }
    xBufferBuilder_t xPacketBuilder;
    vBufferBuilderStart( &xPacketBuilder, (uint8_t *) pcBuffer, ulBufferLength );
    /* Pack the data */
    vBufferBuilderPushData( &xPacketBuilder, pxMessage->pucPayload, pxMessage->usPayloadLen );
    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_DEBUG,"[WIFI Event][Send]: [%x]",pcBuffer);
    /* Push the message at the UART driver */
    pxWifiATCmdOutput->pxImplementation->fnSendBuffer( pxWifiATCmdOutput->pvContext, xPacketBuilder.pucBuffer, xPacketBuilder.ulIndex );

    /* Release buffer */
	pxWifiATCmdOutput->pxImplementation->fnReleaseBuffer( pxWifiATCmdOutput->pvContext, xPacketBuilder.pucBuffer );
    return;
}

/*-----------------------------------------------------------*/

void s4527438_hal_wifi_init(void) {
    xWifiComms.fnEnable( true );
}

void vWifiPacketBuilder( char cByte )
{
    //eLog( LOG_APPLICATION, LOG_APOCALYPSE, "WIFI Recv: [%X](%c)\r\n", (uint8_t) cByte,cByte );
    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[WIFI Event]: value = [%x]",cByte);

    //s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[WIFI Event]: value = [%x]",cByte);
    if ( xWifiComms.fnReceiveHandler == NULL ) {
        return;
    }
}

