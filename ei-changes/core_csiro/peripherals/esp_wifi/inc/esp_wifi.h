/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the FreeRTOS operating system.
 *
 * Filename: esp32.h
 * Creation_Date: 23/09/2019
 * Author: Anton Schieber <anton.schieber@data61.csiro.au>
 *
 * Communication functionality for the esp32 chipset
 * 
 */
#ifndef __CSIRO_CORE_PERIPHERALS_ESP32
#define __CSIRO_CORE_PERIPHERALS_ESP32
/* Includes -------------------------------------------------*/

#include "gpio.h"
#include "uart.h"

/* Forward Declarations -------------------------------------*/

/* Module Defines -------------------------------------------*/
// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/

typedef struct xEspAtInit_t
{
	xUartModule_t *pxUart; /**< UART Interface */
	xGpio_t		   xEspEnable;
	xGpio_t		   xEspBootMode;
	xGpio_t		   xEspHostWk;
} xEspAtInit_t;

typedef enum eEspConnection_t {
	WIFI_CONNECTED	= 0x00,
	WIFI_DISCONNECTED = 0x01,
	WIFI_NO_RESPONSE  = 0x02,
} eEspConnection_t;

typedef enum eEspCommand_t {
	COMMAND_TEST	= 0x00, /**< Test Command */
	COMMAND_QUERY   = 0x01, /**< Query Command */
	COMMAND_SET		= 0x02, /**< Set Command */
	COMMAND_EXECUTE = 0x03, /**< Set Command */
} eEspCommand_t;

typedef enum eEspWifiMode_t {
	MODE_NULL		  = 0x00, /**<  Wi-Fi RF will be disabled */
	MODE_STATION	  = 0x01, /**< Station mode  */
	MODE_SOFT_AP	  = 0x02, /**< SoftAP mode  */
	MODE_STATION_SOFT = 0x03, /**< SoftAP+Station mode */
} eEspWifiMode_t;

typedef struct xWifiNetwork_t
{
	char pcSSID[33];	 /**< Wifi SSID */
	char pcBSSID[7];	 /**< Wifi BSSID */
	char pcPassword[65]; /**< Wifi Password */
} xWifiNetwork_t;

typedef struct xWifiConnection_t
{
	char pcProtocol[4];   /**< Wifi Protocol */
	char pcConnectIP[16]; /**< Remote IP */
	char pcPort[5];		  /**< Remote Port */
} xWifiConnection_t;

/* Function Declarations ------------------------------------*/

/**@brief Initialise esp32 and associated software
 *
 * @param[in] pxInit				Initialisation settings
 */
void vEspInit( xEspAtInit_t *pxInit );

/**@brief Turn ESP on and wait for startup
 *
 * @param[in] bResetNetwork			Set true to reset SSID, BSSID and Password
 */
void vEspWifiOn( bool bResetNetwork );

/**@brief Turn ESP off
 *
 */
void vEspWifiOff( void );

/**@brief Set the type of connection setup i.e. Access point or Station
 *
 * @param[in] eQueryType			Specify type of send
 * @param[in] eMode					Specify wifi mode
 * 
 * @retval	::ERROR_NONE			Queried successfully
 * @retval	::ERROR_TIMEOUT			Failed to properly respond on serial interface
 */
eModuleError_t eEspSetMode( eEspCommand_t eQueryType, eEspWifiMode_t eMode );

/**@brief Connect to a wifi network
 *
 * @param[in] eQueryType			Specify type of send
 * 
 * @retval	::ERROR_NONE			Queried successfully
 * @retval	::ERROR_TIMEOUT			Failed to properly respond on serial interface
 */
eModuleError_t eEspConnect( eEspCommand_t eQueryType );

/**@brief Send a string over wifi
 *
 * @param[in] eQueryType			Specify type of send
 * @param[in] pucData				Data to be sent
 * @param[in] ucDataLen				Length of data to be sent
 * 
 * @retval	::ERROR_NONE			Queried successfully
 * @retval	::ERROR_TIMEOUT			Failed to properly respond on serial interface
 */
eModuleError_t eEspSendData( eEspCommand_t eQueryType, uint8_t *pucData, uint8_t ucDataLen );

/**@brief Specify the server to connect to for data transmission.
 *
 * @param[in] eQueryType			Specify type of send
 * 
 * @retval	::ERROR_NONE			Queried successfully
 * @retval	::ERROR_TIMEOUT			Failed to properly respond on serial interface
 */
eModuleError_t eEspSetClient( eEspCommand_t eQueryType );

/**@brief Used to send a test command to ESP
 *
 * @retval	::ERROR_NONE			Queried successfully
 * @retval	::ERROR_TIMEOUT			Failed to properly respond on serial interface
 */
eModuleError_t eEspTestAT( void );

/**@brief Get the status of wifi connection
 *
 * @retval	::WIFI_CONNECTED			Wifi connection found
 * @retval	::WIFI_DISCONNECTED			Wifi connection lost
 * @retval	::WIFI_NO_RESPONSE			ESP Started and connection failed
 */
eEspConnection_t eGetConnectionStatus( void );

#endif /* __CSIRO_CORE_PERIPHERALS_ESP32 */
