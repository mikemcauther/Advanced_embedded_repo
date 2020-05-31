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

#include "log.h"
#include "board.h"
#include "uart.h"
#include "s4527438_wifi_packet.h"
#include "s4527438_hal_wifi.h"
#include "s4527438_lib_log.h"
#include "nrf_delay.h"

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
    int i = 0;
    /*********Send Length to AT command firstly***********/
	//fnWriteWrapper( pxWifiATCmdOutput->pvContext, "AT+CIPSEND=%d",pxMessage->usPayloadLen);

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
    for( i = 0;i < pxMessage->usPayloadLen;i++)
    {
        s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[WIFI Event][Send]: [%x](%c)",pcBuffer[i],pcBuffer[i]);
    }
    /* Push the message at the UART driver */
    xPacketBuilder.pucBuffer[xPacketBuilder.ulIndex++] = '\r';
    xPacketBuilder.pucBuffer[xPacketBuilder.ulIndex++] = '\n';
    pxWifiATCmdOutput->pxImplementation->fnSendBuffer( pxWifiATCmdOutput->pvContext, xPacketBuilder.pucBuffer, xPacketBuilder.ulIndex );
    nrf_delay_us(10);

    /* Release buffer */
	//pxWifiATCmdOutput->pxImplementation->fnReleaseBuffer( pxWifiATCmdOutput->pvContext, xPacketBuilder.pucBuffer );
    return;
}

/*-----------------------------------------------------------*/

void s4527438_hal_wifi_init(void) {
    xWifiComms.fnEnable( true );
}

void vWifiPacketBuilder( char cByte )
{
    static uint8_t buffer[256] = {0x00};
    static uint8_t cur_idx = 0;

    s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"[WIFI Event]: value = [%x](%c)",cByte,cByte);
    if( (cByte == '\n') || (cByte == '\r') ) {
        if( cur_idx > 0) {
            buffer[cur_idx] = '\n';
            buffer[cur_idx + 1] = '\0';
        }
        cur_idx = 0;
        //s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_ERROR,"%s",buffer);
        eLog( LOG_APPLICATION, LOG_APOCALYPSE, "%s", buffer );
    } else {
        buffer[cur_idx++] = cByte;
        return;
    }
    //eLog( LOG_APPLICATION, LOG_APOCALYPSE, "WIFI Recv: [%X](%c)\r\n", (uint8_t) cByte,cByte );

    if ( xWifiComms.fnReceiveHandler == NULL ) {
        return;
    }
}

