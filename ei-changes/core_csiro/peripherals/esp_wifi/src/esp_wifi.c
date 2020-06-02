/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 */
/* Includes -------------------------------------------------*/

#include "string.h"

#include "FreeRTOS.h"
#include "task.h"

#include "crc.h"
#include "gpio.h"
#include "log.h"
#include "memory_operations.h"

#include "argon.h"
#include "esp_wifi.h"
#include "uart.h"
#include "nrf_delay.h"
#include "s4527438_lib_log.h"

/* Private Defines ------------------------------------------*/
// clang-format off
#if 0
#define RECV_ERROR 			"ERROR\r\n\0"
#define RECV_OK 			"OK\r\n\0"
#define RECV_BUSY 			"busy p...\r\n\0"
#define RECV_CONNECTED		"WIFI CONNECTED\r\n\0"
#define RECV_GOT_IP			"WIFI GOT IP\r\n\0"
#define RECV_DISCONNECT		"WIFI DISCONNECT\r\n\0"
#define RECV_SEND_WAITING	">\r\n\0"
#define RECV_REMOTE_CONNECT		"CONNECT\r\n\0"
#define RECV_CONNECT_ERROR		"+CWJAP:2\r\n\0"
#else
#define RECV_ERROR 			"ERROR"
#define RECV_OK 			"OK"
#define RECV_BUSY 			"busy p..."
#define RECV_CONNECTED		"WIFI CONNECTED"
#define RECV_GOT_IP			"WIFI GOT IP"
#define RECV_DISCONNECT		"WIFI DISCONNECT"
#define RECV_SEND_WAITING	">"
#define RECV_REMOTE_CONNECT		"CONNECT"
#define RECV_CONNECT_ERROR		"+CWJAP:2"
#endif
#define RECV_BUFFER 		50
#define RECV_MAX_RESPONSE 	18
// clang-format on

/* Type Definitions -----------------------------------------*/

/* Set up the message struct to be sent/recieved from esp */
typedef struct xEspMessage_t
{
	uint8_t pucMessage[RECV_BUFFER];
	uint8_t ucIndex;
} xEspMessage_t;

/* Used for error handling of message responses */
typedef enum eEspRecievedCommand_t {
	MSG_OK			   = 0x00,
	MSG_BUSY		   = 0x01,
	MSG_ERROR		   = 0x02,
	MSG_EXECUTE		   = 0x03,
	MSG_CONNECT		   = 0x04,
	MSG_DISCONNECT	 = 0x05,
	MSG_REMOTE_CONNECT = 0x06,
	MSG_GOT_IP		   = 0x07,
	MSG_UNKNOWN		   = 0x08,
	MSG_CONNECT_ERROR  = 0x09,
} eEspRecievedCommand_t;

/* Function Declarations ------------------------------------*/

/* Checks the type of command recieved */
eEspRecievedCommand_t eEspCheckCommand( xEspMessage_t *pxEspMessage );

/* Adds end of string chars and sends the message over uart */
eModuleError_t eEspSendCommand( xBufferBuilder_t *pxBuilder );

/* Compares the message recieved with a possible message */
bool bEspCompare( xEspMessage_t *pucRevMessage, uint8_t *pucComResponse, uint8_t ucMax );

/* Handles each byte recieved and adds them to message buffer */
void vEspSerialByteHandler( char cRxByte );

/* Handles messages recieved from the ESP to perform things like reconnecting */
void vEspController( void *pvParameters );

/* Private Variables ----------------------------------------*/

STATIC_TASK_STRUCTURES( xEspRecvTask, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 1 );
STATIC_TASK_STRUCTURES( xEspControllerTask, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY + 2 );

static xEspAtInit_t *pxConfig;
static xEspMessage_t xEspResponse;
static xEspMessage_t xEspMessage;

static xGpio_t xRts = UNUSED_GPIO;
static xGpio_t xCts = UNUSED_GPIO;

static QueueHandle_t	 pxCommandQueue;
static SemaphoreHandle_t xCommsSemaphore;
static SemaphoreHandle_t xReadySemaphore;

static uint8_t pucEspResponseCheck[RECV_MAX_RESPONSE];

bool bIsConnecting  = false;
bool bStarted		= false;
bool bConnected		= false;
bool bDisconnected  = false;
bool bRemoteConnect = false;
bool bSaved;

xWifiNetwork_t xNetwork = {
	.pcSSID		= "BruceTest",
	.pcPassword = "csse4011",
	.pcBSSID	= "",
};

xWifiConnection_t xConnection = {
	.pcProtocol  = "TCP", /**< TCP OR UDP OR SSL */
	.pcConnectIP = "192.168.43.166",
	.pcPort		 = "8080",
};

/*-----------------------------------------------------------*/

eEspConnection_t eGetConnectionStatus( void )
{
    if ( bIsConnecting == true ) {
		return WIFI_CONNECTING;
    }
	if ( bConnected ) {
		return WIFI_CONNECTED;
	}
	else if ( bDisconnected ) {
		return WIFI_DISCONNECTED;
	}
	return WIFI_NO_RESPONSE;
}

/*-----------------------------------------------------------*/

void vEspInit( xEspAtInit_t *pxInit )
{
	static xSerialReceiveArgs_t xArgs;
	pxConfig = pxInit;

	xRts = pxConfig->pxUart->xPlatform.xRts;
	xCts = pxConfig->pxUart->xPlatform.xCts;

	/* Start our serial handler thread */
	xArgs.pxUart	= pxConfig->pxUart;
	xArgs.fnHandler = vEspSerialByteHandler;

	xCommsSemaphore = xSemaphoreCreateBinary();
	xReadySemaphore = xSemaphoreCreateBinary();
	pxCommandQueue  = xQueueCreate( 4, sizeof( xEspMessage_t ) );

	configASSERT( pxCommandQueue );
	configASSERT( xCommsSemaphore != NULL );
	configASSERT( xCommsSemaphore != NULL );

	STATIC_TASK_CREATE( xEspRecvTask, vSerialReceiveTask, "WiFi Recv", &xArgs );
	STATIC_TASK_CREATE( xEspControllerTask, vEspController, "WiFi Cont", NULL );
}

/*-----------------------------------------------------------*/

void vEspWifiOn( bool bResetNetwork )
{
	/* Re-initialise variables for ESP reboot */
	bStarted = false;
	// vUartChangeBaud( pxConfig->pxUart, 115200 );
	vUartOn( pxConfig->pxUart );

	/* Restart ESP32 */
	vGpioSet( pxConfig->xEspBootMode );
	vTaskDelay( pdMS_TO_TICKS( 100 ) );
	vGpioClear( pxConfig->xEspEnable );
	vTaskDelay( pdMS_TO_TICKS( 100 ) );
	vGpioSet( pxConfig->xEspEnable );

	/* Bootloader message at different baud rate will cause ERRORSRC to trigger */
	xSemaphoreTake( xCommsSemaphore, pdMS_TO_TICKS( 2000 ) );

	/* Setup for normal UART operation */
	// vUartChangeBaud( pxConfig->pxUart, 921600 );
	xEspResponse.ucIndex = 0;
	bStarted			 = true;
	xSemaphoreGive( xReadySemaphore );

	/* Acts wierd when messages are sent straight after boot */
	vTaskDelay( pdMS_TO_TICKS( 3000 ) );

	if ( bResetNetwork ) {
		/* Set connection inforamtion is ESP NVM */
		eEspConnect( COMMAND_SET );
	}
}

/*-----------------------------------------------------------*/

void vEspWifiOff( void )
{
	vGpioClear( pxConfig->xEspBootMode );
	vTaskDelay( pdMS_TO_TICKS( 100 ) );
	vGpioClear( pxConfig->xEspEnable );
	vTaskDelay( pdMS_TO_TICKS( 100 ) );
	vUartOff( pxConfig->pxUart );
}

/*-----------------------------------------------------------*/

eModuleError_t eEspTestAT( void )
{
	xBufferBuilder_t xBuilder;
	uint32_t		 ulBufferLen;
	uint8_t *		 pucBuffer;
	uint8_t			 ucDataLen;

	/* Construct the packet to send */
	// pucBuffer = (uint8_t *) pcUartClaimBuffer( pxConfig->pxUart, &ulBufferLen, portMAX_DELAY );
	pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer(pxConfig->pxUart, &ulBufferLen);
	configASSERT( pucBuffer != NULL );
	// UNUSED(pucBuffer);
	// UNUSED(ulBufferLen);
	//ucDataLen = 7;
	vBufferBuilderStart( &xBuilder, pucBuffer, ulBufferLen );
	//vBufferBuilderPushData( &xBuilder, "AT+CGMR", ucDataLen );
	ucDataLen = 2;
	vBufferBuilderPushData( &xBuilder, "AT", ucDataLen );

	return eEspSendCommand( &xBuilder );
}

/*-----------------------------------------------------------*/
eModuleError_t eEspSendRAWCommnd(uint8_t *cmd,uint8_t cmd_len)
{
	xBufferBuilder_t xBuilder;
	uint32_t		 ulBufferLen;
	uint8_t *		 pucBuffer;
	uint8_t			 ucDataLen;

	pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer(pxConfig->pxUart, &ulBufferLen);
	configASSERT( pucBuffer != NULL );

	ucDataLen = cmd_len;
	vBufferBuilderStart( &xBuilder, pucBuffer, ulBufferLen );
	vBufferBuilderPushData( &xBuilder, cmd, ucDataLen );

	vBufferBuilderPushData( &xBuilder, "\r\n", 2 );
	configASSERT( bBufferBuilderValid( &xBuilder ) );

	/* Send commands */
	// vUartQueueBuffer( pxConfig->pxUart, (int8_t *) pxBuilder->pucBuffer, pxBuilder->ulIndex );
	xUartBackend.fnSendBuffer(pxConfig->pxUart, (char *) xBuilder.pucBuffer, xBuilder.ulIndex);
    nrf_delay_us(10);

	//return eEspSendCommand( &xBuilder );
	return ERROR_NONE;
}
/*-----------------------------------------------------------*/

eModuleError_t eEspSendData( eEspCommand_t eQueryType, uint8_t *pucData, uint8_t ucDataLen )
{
	xBufferBuilder_t xBuilder;
	uint32_t		 ulBufferLen;
	uint8_t *		 pucBuffer;

	/* Error currently returns command not valid for send command */
	UNUSED( pucData );
	UNUSED( ucDataLen );

	/* Construct the packet to send */
	// pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen, portMAX_DELAY );
	pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen);
	configASSERT( pucBuffer != NULL );

	vBufferBuilderStart( &xBuilder, pucBuffer, ulBufferLen );
	vBufferBuilderPushData( &xBuilder, "AT+CIPSEND", 10 );

	switch ( eQueryType ) {
		case COMMAND_TEST:
			break;
		case COMMAND_QUERY:
			break;
		case COMMAND_SET:

			vBufferBuilderPushByte( &xBuilder, '=' );
			vBufferBuilderPushByte( &xBuilder, '1' );
			break;
		case COMMAND_EXECUTE:
			break;
	}

	return eEspSendCommand( &xBuilder );
}

/*-----------------------------------------------------------*/
eModuleError_t eEspForceReconnect( eEspCommand_t eQueryType )
{
	xBufferBuilder_t xBuilder;
	uint32_t		 ulBufferLen;
	uint8_t *		 pucBuffer;
	uint8_t			 ucDataLen;

#if 0
    if ( bIsConnecting == true ) {
	    return ERROR_NONE;
    }
    
#endif
	/* Construct the packet to send */
	// pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen, portMAX_DELAY );
	pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen);
	configASSERT( pucBuffer != NULL );

	ucDataLen = 8;
	vBufferBuilderStart( &xBuilder, pucBuffer, ulBufferLen );
	vBufferBuilderPushData( &xBuilder, "AT+CWLAP", ucDataLen );

	switch ( eQueryType ) {
		case COMMAND_TEST:
			break;

		case COMMAND_QUERY:
			break;

		case COMMAND_SET:
			break;

		case COMMAND_EXECUTE:
			break;
	}

    //return eEspSendRAWCommnd(xBuilder.pucBuffer,xBuilder.ulIndex);
	return eEspSendCommand( &xBuilder );
}
/*-----------------------------------------------------------*/

eModuleError_t eEspSetMode( eEspCommand_t eQueryType, eEspWifiMode_t eMode )
{
	xBufferBuilder_t xBuilder;
	uint32_t		 ulBufferLen;
	uint8_t *		 pucBuffer;
	uint8_t			 ucDataLen;

	/* Construct the packet to send */
	// pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen, portMAX_DELAY );
	pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen);
	configASSERT( pucBuffer != NULL );

	ucDataLen = 9;
	vBufferBuilderStart( &xBuilder, pucBuffer, ulBufferLen );
	vBufferBuilderPushData( &xBuilder, "AT+CWMODE", ucDataLen );

	switch ( eQueryType ) {
		case COMMAND_TEST:
			ucDataLen += 2;
			vBufferBuilderPushData( &xBuilder, "=?", 2 );
			break;

		case COMMAND_QUERY:
			ucDataLen += 1;
			vBufferBuilderPushByte( &xBuilder, '?' );
			break;

		case COMMAND_SET:
			ucDataLen += 2;
			vBufferBuilderPushByte( &xBuilder, '=' );
			vBufferBuilderPushByte( &xBuilder, eMode + '0' );
			break;

		case COMMAND_EXECUTE:
			break;
	}

	return eEspSendCommand( &xBuilder );
}

/*-----------------------------------------------------------*/

eModuleError_t eEspSetClient( eEspCommand_t eQueryType )
{
	xBufferBuilder_t xBuilder;
	uint32_t		 ulBufferLen;
	uint8_t *		 pucBuffer;
	uint8_t			 ucLength;

	/* Construct the packet to send */
	// pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen, portMAX_DELAY );
	pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen);
	configASSERT( pucBuffer != NULL );

	vBufferBuilderStart( &xBuilder, pucBuffer, ulBufferLen );
	vBufferBuilderPushData( &xBuilder, "AT+CIPSTART", 11 );

	switch ( eQueryType ) {

		case COMMAND_TEST:
			break;

		case COMMAND_QUERY:
			break;

		case COMMAND_SET:
			vBufferBuilderPushData( &xBuilder, "=\"", 2 );

			vBufferBuilderPushData( &xBuilder, xConnection.pcProtocol, 3 );
			vBufferBuilderPushData( &xBuilder, "\",\"", 3 );

			for ( ucLength = 0; xConnection.pcConnectIP[ucLength] != '\0'; ucLength++ ) {
				;
			}
			vBufferBuilderPushData( &xBuilder, xConnection.pcConnectIP, ucLength );
			vBufferBuilderPushData( &xBuilder, "\",", 2 );

			for ( ucLength = 0; xConnection.pcPort[ucLength] != '\0'; ucLength++ ) {
				;
			}
			vBufferBuilderPushData( &xBuilder, xConnection.pcPort, ucLength );
			break;

		case COMMAND_EXECUTE:
			break;
	}

	return eEspSendCommand( &xBuilder );
}

/*-----------------------------------------------------------*/

eModuleError_t eEspConnect( eEspCommand_t eQueryType )
{
	xBufferBuilder_t xBuilder;
	uint32_t		 ulBufferLen;
	uint8_t *		 pucBuffer;
	uint8_t			 ucLength;

	/* Construct the packet to send */
	// pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen, portMAX_DELAY );
	pucBuffer = (uint8_t *) xUartBackend.fnClaimBuffer( pxConfig->pxUart, &ulBufferLen);
	configASSERT( pucBuffer != NULL );

	vBufferBuilderStart( &xBuilder, pucBuffer, ulBufferLen );
	vBufferBuilderPushData( &xBuilder, "AT+CWJAP", 8 );

	switch ( eQueryType ) {

		case COMMAND_TEST:
			vBufferBuilderPushData( &xBuilder, "=?", 2 );
			break;

		case COMMAND_QUERY:
			vBufferBuilderPushByte( &xBuilder, '?' );
			break;

		case COMMAND_SET:

			vBufferBuilderPushData( &xBuilder, "=\"", 2 );

			for ( ucLength = 0; xNetwork.pcSSID[ucLength] != '\0'; ucLength++ ) {
				;
			}
			vBufferBuilderPushData( &xBuilder, xNetwork.pcSSID, ucLength );
			vBufferBuilderPushData( &xBuilder, "\",\"", 3 );

			for ( ucLength = 0; xNetwork.pcPassword[ucLength] != '\0'; ucLength++ ) {
				;
			}
			vBufferBuilderPushData( &xBuilder, xNetwork.pcPassword, ucLength );
			vBufferBuilderPushByte( &xBuilder, '\"' );

			for ( ucLength = 0; xNetwork.pcBSSID[ucLength] != '\0'; ucLength++ ) {
				;
			}

			if ( ucLength > 0 ) {
				vBufferBuilderPushData( &xBuilder, ",\"", 2 );
				vBufferBuilderPushData( &xBuilder, xNetwork.pcBSSID, ucLength );
				vBufferBuilderPushByte( &xBuilder, '\"' );
			}
			break;

		case COMMAND_EXECUTE:
			break;
	}

    bIsConnecting = true;
	return eEspSendCommand( &xBuilder );
}

/*-----------------------------------------------------------*/

eModuleError_t eEspSendCommand( xBufferBuilder_t *pxBuilder )
{
	eModuleError_t eError = ERROR_NONE;

	vBufferBuilderPushData( pxBuilder, "\r\n", 2 );
	configASSERT( bBufferBuilderValid( pxBuilder ) );

	/* Wait until ready or for a long time... */
	xSemaphoreTake( xReadySemaphore, pdMS_TO_TICKS( 10000 ) );

	/* Send commands */
	// vUartQueueBuffer( pxConfig->pxUart, (int8_t *) pxBuilder->pucBuffer, pxBuilder->ulIndex );
	xUartBackend.fnSendBuffer(pxConfig->pxUart, (char *) pxBuilder->pucBuffer, pxBuilder->ulIndex);

	if ( xSemaphoreTake( pxConfig->pxUart->xTxDone, pdMS_TO_TICKS( 500 ) ) != pdPASS ) {
		eError = ERROR_TIMEOUT;
	}
	return eError;
}

/*-----------------------------------------------------------*/

bool bEspCompare( xEspMessage_t *pucRevMessage, uint8_t *pucComResponse, uint8_t ucMax )
{
	uint8_t ucLength;
	int8_t  ucComparison;

	UNUSED( ucMax );
#if 0
	ucLength = strlen( (char *) pucEspResponseCheck );

	/* Check if the expected response is less than the actual */
	if ( pucRevMessage->ucIndex < ucLength ) {
		return false;
	}

	//ucComparison = strncmp( (char *) &pucRevMessage->pucMessage[pucRevMessage->ucIndex - ucLength], (char *) pucComResponse, ucLength );
	ucComparison = strncmp( (char *) &pucRevMessage->pucMessage, (char *) pucComResponse, ucLength );
	if ( ucComparison <= ucMax && ucComparison >= -ucMax ) {
		return true;
	}
#else
	ucLength = strlen( (char *) pucEspResponseCheck );

	/* Check if the expected response is less than the actual */
	if ( pucRevMessage->ucIndex < ucLength ) {
		return false;
	}

	ucComparison = strncmp( (char *) &pucRevMessage->pucMessage, (char *) pucComResponse, ucLength );
	if ( ucComparison == 0 ) {
		return true;
	}
#endif

	return false;
}

/*-----------------------------------------------------------*/

eEspRecievedCommand_t eEspCheckCommand( xEspMessage_t *pucEspMessage )
{

	/* Check if OK response received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_OK, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 0 ) ) {
		return MSG_OK;
	}

	/* Check if Error response received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_ERROR, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 0 ) ) {
		return MSG_ERROR;
	}

	/* Check if Busy response received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_BUSY, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 0 ) ) {
		return MSG_BUSY;
	}

	/* Check if  response received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_CONNECTED, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 0 ) ) {
		return MSG_CONNECT;
	}

	/* Check if disconnected response received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_DISCONNECT, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 0 ) ) {
		return MSG_DISCONNECT;
	}

	/* Check if disconnected response received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_REMOTE_CONNECT, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 0 ) ) {
		return MSG_REMOTE_CONNECT;
	}

	/* Check if got ip received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_GOT_IP, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 0 ) ) {
		return MSG_GOT_IP;
	}

	/* Check for connection error received */
	pvMemcpy( (char *) pucEspResponseCheck, RECV_CONNECT_ERROR, RECV_MAX_RESPONSE );
	if ( bEspCompare( pucEspMessage, pucEspResponseCheck, 2 ) ) {
		return MSG_CONNECT_ERROR;
	}

	return MSG_UNKNOWN;
}

/*-----------------------------------------------------------*/

void vEspSerialByteHandler( char cRxByte )
{

	if ( bStarted == false ) {

		xEspResponse.pucMessage[xEspResponse.ucIndex++] = (uint8_t) cRxByte;
		xEspResponse.ucIndex							= xEspResponse.ucIndex % RECV_BUFFER;

		/* If in boot mode, check for the end of the message */
		if ( xEspResponse.pucMessage[( xEspResponse.ucIndex - 19 ) % RECV_BUFFER] == 'n' &&
			 xEspResponse.pucMessage[( xEspResponse.ucIndex - 4 ) % RECV_BUFFER] == '[' &&
			 xEspResponse.pucMessage[( xEspResponse.ucIndex - 3 ) % RECV_BUFFER] == '0' &&
			 xEspResponse.pucMessage[( xEspResponse.ucIndex - 2 ) % RECV_BUFFER] == ';' ) {

			xSemaphoreGive( xCommsSemaphore );
		}
	}
	else {

		/* Wait for end of line to be sent - some messages start with \r\n, as for the >2 section */
		if ( (cRxByte == '\r' || cRxByte == '\n') && xEspResponse.ucIndex > 0 ) {
#if 0
		    xEspResponse.pucMessage[xEspResponse.ucIndex++] = '\r';
		    xEspResponse.pucMessage[xEspResponse.ucIndex++] = '\n';
#else
		    xEspResponse.pucMessage[xEspResponse.ucIndex] = '\r';
		    xEspResponse.pucMessage[xEspResponse.ucIndex+1] = '\n';
		    xEspResponse.pucMessage[xEspResponse.ucIndex+2] = '\0';
#endif
			xQueueSend( pxCommandQueue, &xEspResponse, portMAX_DELAY );
			xEspResponse.ucIndex = 0;
        } else if((cRxByte == '\r' || cRxByte == '\n')) {
			xEspResponse.ucIndex = 0;
		}else {
		    xEspResponse.pucMessage[xEspResponse.ucIndex++] = (uint8_t) cRxByte;
        }
	}
}

/*-----------------------------------------------------------*/

void vEspController( void *pvParameters )
{
	UNUSED( pvParameters );

	eEspRecievedCommand_t eError;

	for ( ;; ) {

		if ( xQueueReceive( pxCommandQueue, &xEspMessage, portMAX_DELAY ) == pdPASS ) {
            eLog( LOG_APPLICATION, LOG_APOCALYPSE, "%s", xEspMessage.pucMessage );
            s4527438_LOGGER(MY_OS_LIB_LOG_LEVEL_LOG,"%s",xEspMessage.pucMessage);
			eError = eEspCheckCommand( &xEspMessage );

			switch ( eError ) {
				case MSG_OK:
					xSemaphoreGive( xReadySemaphore );
					break;
				case MSG_ERROR:
					xSemaphoreGive( xReadySemaphore );
					break;
				case MSG_BUSY:
					break;
				case MSG_EXECUTE:
					break;
				case MSG_CONNECT:
					bDisconnected = false;
					break;
				case MSG_DISCONNECT:
                    bIsConnecting  = false;
					eEspConnect( COMMAND_SET );
					bConnected	= false;
					bDisconnected = true;
					break;
				case MSG_REMOTE_CONNECT:
					bRemoteConnect = true;
					break;
				case MSG_GOT_IP:
                    bIsConnecting  = false;
					bConnected = true;
					break;
				case MSG_UNKNOWN:
					break;
				case MSG_CONNECT_ERROR:
                    bIsConnecting  = false;
					eEspConnect( COMMAND_SET );
					bDisconnected = false;
					break;
			}
		}
	}
}
