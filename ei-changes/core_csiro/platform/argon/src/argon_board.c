/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 */

/* Includes -------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"

/* Board Layout */
#include "application.h"
#include "argon.h"
#include "board.h"
#include "nrf52840.h"

/* System Services */
#include "adc.h"
#include "crc.h"
#include "device_constants.h"
#include "device_nvm.h"
#include "gpio.h"
#include "i2c.h"
#include "leds.h"
#include "rtc.h"
#include "spi.h"
#include "tdf.h"
#include "temp.h"
#include "uart.h"
#include "usb.h"
#include "watchdog.h"

/* Communication */
#include "bluetooth.h"
#include "log.h"
#include "unified_comms.h"
#include "unified_comms_bluetooth.h"
#include "unified_comms_gatt.h"
#include "unified_comms_serial.h"

/* Peripherals */
#include "esp_wifi.h"
/* Data Loggers */
#include "bluetooth_logger.h"
#include "serial_logger.h"

#include "nrf_drv_clock.h"
#include "nrf_delay.h"

/* Private Defines ------------------------------------------*/
// clang-format off

#define I2C_INSTANCE    TWIM1
#define SPI_INSTANCE    SPIM0

// clang-format off
// clang-format on

/* Type Definitions -----------------------------------------*/

/* Function Declarations ------------------------------------*/

void prvBoardLowPowerInit(void);
void prvBoardServicesInit(void);
void prvBoardPrintIdentifiers(void);
void prvBoardLedsInit(void);
void prvBoardNvmInit(void);
void prvBoardSerialInit(void);
void prvBoardSerialWifiATCmdInit(void);
void prvBoardInterfaceInit(void);
void prvBoardPinsInit(void);
void prvBoardBluetoothInit(void);
void prvBoardPeripheralInit(void);
void prvBoardLoggersInit(void);

/* Private Variables ----------------------------------------*/

/* Create UART Driver, 4 buffers of 128 bytes each, 64 byte receive stream */
UART_MODULE_CREATE(SERIAL_OUTPUT, NRF_UARTE0, UARTE0_UART0_IRQHandler, UNUSED_IRQ, 4, 512, 64);
UART_MODULE_CREATE(WIFI_UART, NRF_UARTE1, UARTE1_IRQHandler, UNUSED, 4, 512, 64);

/* Create Watchdog Timer: The handler is assigned during initialisation, set to NULL as placeholder */
WATCHDOG_MODULE_CREATE(WDT_IRQHandler, WDT_IRQn, NULL);
I2C_MODULE_CREATE(NRF_I2C, I2C_INSTANCE);
ADC_MODULE_CREATE(ADC, ADC_INSTANCE, UNUSED);
SPI_MODULE_CREATE(NRF52_SPI, SPI_INSTANCE, SPIM0_TWIM0_IRQHandler);
/* TODO: SPI for flash */
/* SPI_MODULE_CREATE( FLASH_SPI, FLASH_SPI_INSTANCE, SPIM0_TWIM0_IRQHandler ); */

xWatchdogModule_t *pxWatchdog = &WATCHDOG_MODULE_GET(WDT_IRQHandler);
xUartModule_t *pxUartOutput = &UART_MODULE_GET(SERIAL_OUTPUT);
xUartModule_t *pxSerialWifi   = &UART_MODULE_GET(WIFI_UART);
xI2CModule_t *pxI2C = &I2C_MODULE_GET(NRF_I2C);
xAdcModule_t *pxAdc = &ADC_MODULE_GET(ADC);
xSpiModule_t *pxSpi = &SPI_MODULE_GET(NRF52_SPI);
/* TODO: SPI for flash */
/*SPI_MODULE_CREATE( FLASH_SPI, FLASH_SPI_INSTANCE, SPIM0_TWIM0_IRQHandler );*/

/* TODO: flash Init */
/* Flash Memory */
/*
xMX25rHardware_t xMX25rHardware = {
    .pxInterface = &SPI_MODULE_GET( FLASH_SPI ),
    .xSpiConfig  = { .xCsGpio = FLASH_CS_GPIO, 0, 0, 0, 0 }
};
xFlashDevice_t xMX25rDevice = {
    .xSettings        = { 0 },
    .pxImplementation = &xMX25rDriver,
    .xCommandQueue  = NULL,
    .pcName           = "MX25R",
    .pxHardware       = (xFlashDefaultHardware_t *) &xMX25rHardware
};

xFlashDevice_t *const  pxOnboardFlash = &xMX25rDevice;
*/

/* ESP AT Structures */
static xEspAtInit_t xEspAtInit = {
  .pxUart       = &UART_MODULE_GET(WIFI_UART),
  .xEspEnable   = WIFI_ENABLE_PIN,
  .xEspBootMode = ESP_BOOT_MODE_PIN,
  .xEspHostWk   = ESP_HOST_WK_PIN,
};

xSerialModule_t xHCIOutput = {
    .pxImplementation = &xUartBackend,
    .pvContext        = &UART_MODULE_GET( SERIAL_OUTPUT )
};

xSerialModule_t xSerialOutput = {
	.pxImplementation = &xUsbBackend,
	.pvContext = NULL};

/* System Structures */
xDeviceConstants_t xDeviceConstants;
xAddress_t xLocalAddress;

/* LED GPIO Pins */
xLEDConfig_t xLEDConfig = {
	.ePolarity = LED_ACTIVE_LOW,
	.xRed = LED_1,
	.xGreen = LED_2,
	.xBlue = LED_3,
	.xYellow = UNUSED_GPIO};

xSerialModule_t *const pxSerialOutput = &xSerialOutput;
xSerialModule_t *const pxHCIOutput = &xHCIOutput;

/* Logger Variables */
TDF_LOGGER_STRUCTURES(SERIAL_LOG, xSerialLog, "SerialLog", (xLoggerDevice_t *)&xSerialLoggerDevice, 100, 0, UINT32_MAX);
TDF_LOGGER_STRUCTURES(BLE_LOG, xBluetoothLog, "BtLog", (xLoggerDevice_t *)&xBluetoothLoggerDevice, CSIRO_BLUETOOTH_MESSAGE_MAX_LENGTH, 0, UINT32_MAX);

/* TODO: init Tdf report via wifi  */
/* TDF_LOGGER_STRUCTURES( WIFI_LOG, xWifiLog, "WifiLog", (xLoggerDevice_t *) &xWifiLoggerDevice, CSIRO_WIFI_MESSAGE_MAX_LENGTH, 0, LOGGER_LENGTH_REMAINING_BLOCKS );*/

LOGS(&xSerialLog_log, &xBluetoothLog_log);
/* TODO: Add Flash log */
/*LOGS( &xSerialLog_log, &xBluetoothLog_log, &xFlashLog_log );*/
TDF_LOGS(&xSerialLog, &xBluetoothLog);
/* TODO: init Tdf report via wifi  */
/* TDF_LOGS(&xSerialLog, &xWifiLog);*/

/*-----------------------------------------------------------*/

void vBoardSetupCore(void)
{
	;
}

/*-----------------------------------------------------------*/

void vBoardInit(void)
{
	/* Enable clocks before initialising the USB driver */
	configASSERT(nrf_drv_clock_init() == 0);
	/* Initialise USB first due to softdevice ordering requirements */
	vUsbInit();
	/* Initialise the bluetooth stack as the first action */
	eBluetoothInit();
	/* Let application define log levels */
	vApplicationSetLogLevels();
	/* Initialise board into low power state */
	prvBoardLowPowerInit();
	/* Output board identifiers */
	prvBoardPrintIdentifiers();
	/* System services init */
	prvBoardServicesInit();
}

/*-----------------------------------------------------------*/

void prvBoardLowPowerInit(void)
{
	/* Initialise GPIO */
	prvBoardPinsInit();
	/* Initialise LEDs */
	prvBoardLedsInit();
	/* Initialise UART first so we have eLog Functionality */
	prvBoardSerialInit();
	/* Initialise Non-Volatile Memory */
	prvBoardNvmInit();
	/* Initialise Shared Interfaces */
	prvBoardInterfaceInit();
	/* Initialise Bluetooth */
	prvBoardBluetoothInit();
	/* Wait a bit before initialising devices */
	vTaskDelay(pdMS_TO_TICKS(200));
	/* Sensor, Memory, Radio Initialisation */
	prvBoardPeripheralInit();
	/* Initialise Logger Structures */
	prvBoardLoggersInit();
    /* TODO : Init wifi AT cmd */
    prvBoardSerialWifiATCmdInit();
}

/*-----------------------------------------------------------*/

void prvBoardPrintIdentifiers(void)
{
	xDeviceConstants_t *pxC = &xDeviceConstants;
	xBluetoothAddress_t xLocalBtAddress;
	uint32_t ulResetCount;

	/* Read Identifiers */
	bDeviceConstantsRead(pxC);
	vBluetoothGetLocalAddress(&xLocalBtAddress);
	eNvmReadData(NVM_RESET_COUNT, &ulResetCount);
	xLocalAddress = xAddressUnpack(xLocalBtAddress.pucAddress);

	eLog(LOG_APPLICATION, LOG_APOCALYPSE, "\r\n\tApp        : %d.%d\r\n", APP_MAJOR, APP_MINOR);
	eLog(LOG_APPLICATION, LOG_APOCALYPSE, "\tMAC ADDR   : %:6R\r\n", xLocalBtAddress.pucAddress);
	eLog(LOG_APPLICATION, LOG_APOCALYPSE, "\tReset Count: %d\r\n", ulResetCount);
}

/*-----------------------------------------------------------*/

void prvBoardServicesInit(void)
{
	/* Setup the serial byte handler */
	vUsbSetByteHandler(fnBoardSerialHandler());

    static xSerialReceiveArgs_t xArgs;

    /* Start our serial handler thread */
    xArgs.pxUart    = pxUartOutput;
    xArgs.fnHandler = fnBoardHCIHandler();
    configASSERT( xTaskCreate( vSerialReceiveTask, "Ser Recv", configMINIMAL_STACK_SIZE, &xArgs, tskIDLE_PRIORITY + 1, NULL ) == pdPASS );

	/* Setup our Unified Comms interfaces */
	vUnifiedCommsInit(&xSerialComms);
	vUnifiedCommsInit(&xBluetoothComms);
	vUnifiedCommsInit(&xGattComms);
	/* Device by default are ordinary nodes */
	xSerialComms.fnReceiveHandler = NULL;
	xBluetoothComms.fnReceiveHandler = NULL;
	xGattComms.fnReceiveHandler = NULL;
}

/*-----------------------------------------------------------*/

void prvBoardPinsInit(void)
{
	vGpioInit();

	vGpioSetup(SPIM0_SS_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH);
	vGpioSetup(SPIM0_MISO_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH);
	vGpioSetup(SPIM0_MOSI_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH);
	vGpioSetup(SPIM0_SCK_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH);

	vGpioSetup(TWIM1_SDA_PIN, GPIO_DISABLED, GPIO_DISABLED_NOPULL);
	vGpioSetup(TWIM1_SCL_PIN, GPIO_DISABLED, GPIO_DISABLED_NOPULL);

	/* Enable PCB Antenna, signals appear to be inverted from block diagram */
	vGpioSetup(SKY13351_CTRL_1, GPIO_PUSHPULL, GPIO_PUSHPULL_LOW);
	vGpioSetup(SKY13351_CTRL_2, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH);
    /* Setup Wi-Fi Pins */
    //vGpioSetup(ESP_BOOT_MODE_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_LOW);
    //vGpioSetup(WIFI_ENABLE_PIN, GPIO_OPENDRAIN, GPIO_OPENDRAIN_LOW);
	vGpioSetup( WIFI_ENABLE_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH );
    //vGpioSetup(WIFI_RTS_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH);
}

/*-----------------------------------------------------------*/

void prvBoardLedsInit(void)
{
	vLedsInit(&xLEDConfig);
}

/*-----------------------------------------------------------*/

void prvBoardSerialInit(void)
{
	/* TODO: Figure out how to increase while retaining reliable serial recv */
	pxUartOutput->xPlatform.pxTimer = NRF_TIMER1;
	pxUartOutput->ulBaud = 115200;
	pxUartOutput->xPlatform.xRx = UART0_RX_PIN;
	pxUartOutput->xPlatform.xTx = UART0_TX_PIN;
	pxUartOutput->xPlatform.xRts = UART0_RTS_PIN;
	pxUartOutput->xPlatform.xCts = UNUSED_GPIO;

	eUartInit(pxUartOutput, true);
}

/* TODO: For wifi AT command UART port */
void prvBoardSerialWifiATCmdInit(void)
{
    /* Setup the WiFi UART */
    pxSerialWifi->xPlatform.pxTimer  = NRF_TIMER2;
    pxSerialWifi->ulBaud             = 115200;
    pxSerialWifi->xPlatform.xRx      = WIFI_RX_PIN;
    pxSerialWifi->xPlatform.xTx      = WIFI_TX_PIN;
    pxSerialWifi->xPlatform.xRts     = WIFI_RTS_PIN;
    pxSerialWifi->xPlatform.xCts     = UNUSED_GPIO;
    // pxSerialWifi->ulTrailingWindowMs = 625; /* ulTrailingWindowMs = 12.5 x Mili Seconds  */
	eUartInit(pxSerialWifi, false);
    vEspInit(&xEspAtInit);

#if 0
	vGpioSetup( WIFI_ENABLE_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH );

	/* TODO: Figure out how to increase while retaining reliable serial recv */
	pxWifiOutput->xPlatform.pxTimer = NRF_TIMER2;
	pxWifiOutput->ulBaud = 115200;
	pxWifiOutput->xPlatform.xRx = WIFI_UART_RX_PIN;
	pxWifiOutput->xPlatform.xTx = WIFI_UART_TX_PIN;
	pxWifiOutput->xPlatform.xRts = WIFI_UART_RTS_PIN;
	pxWifiOutput->xPlatform.xCts = UNUSED_GPIO;

	eUartInit(pxWifiOutput, false);
#endif
}

/*-----------------------------------------------------------*/

void prvBoardNvmInit(void)
{
	eModuleError_t eResult;
	uint32_t ulResetCount;

	/* Initialise NVM */
	eResult = eNvmInit();
	if (eResult != ERROR_NONE)
	{
		eLog(LOG_APPLICATION, LOG_APOCALYPSE, "Failed to initialise NVM\r\n");
	}

	/* Increment reset count */
	eResult = eNvmIncrementData(NVM_RESET_COUNT, &ulResetCount);
	if (eResult != ERROR_NONE)
	{
		eLog(LOG_APPLICATION, LOG_ERROR, "Failed to increment reset count\r\n");
	}
}

/*-----------------------------------------------------------*/

void prvBoardInterfaceInit(void)
{
	/* Setup the uart interface channel */
	pxSpi->xPlatform.xMosi = SPIM0_MOSI_PIN;
	pxSpi->xPlatform.xMiso = SPIM0_MISO_PIN;
	pxSpi->xPlatform.xSclk = SPIM0_SCK_PIN;

	/* Setup the I2C interface pins */
	pxI2C->xPlatform.xSda = TWIM1_SDA_PIN;
	pxI2C->xPlatform.xScl = TWIM1_SCL_PIN;

	/* Initialise Interfaces */
	vCrcInit();
	vRtcInit();
	eSpiInit(pxSpi);
	eI2CInit(pxI2C);
	vWatchdogInit(pxWatchdog);
	vAdcInit(pxAdc);
	vTempInit();
}

/*-----------------------------------------------------------*/

void prvBoardLoggersInit(void)
{
	eTdfLoggerConfigure(&xNullLog, LOGGER_CONFIG_INIT_DEVICE, NULL);

	eTdfLoggerConfigure(&xSerialLog, LOGGER_CONFIG_INIT_DEVICE, 0);
	eTdfLoggerConfigure(&xSerialLog, LOGGER_CONFIG_COMMIT_ONLY_USED_BYTES, 0);

	eTdfLoggerConfigure(&xBluetoothLog, LOGGER_CONFIG_INIT_DEVICE, 0);
	eTdfLoggerConfigure(&xBluetoothLog, LOGGER_CONFIG_COMMIT_ONLY_USED_BYTES, 0);
}

/*-----------------------------------------------------------*/

void prvBoardBluetoothInit(void)
{
	eModuleError_t eGattInit(void);
	/* Initialise GATT Table */
	eGattInit();

	xGattLocalCharacteristic_t xPlatform = {
		.usCharacteristicHandle = gattdb_model_number_string,
		.pucData = (uint8_t *)"argon",
	};
	xGattLocalCharacteristic_t xApplication = {
		.usCharacteristicHandle = gattdb_firmware_revision_string,
		.pucData = (uint8_t *)STRINGIFY(APP_MAJOR) "." STRINGIFY(APP_MINOR),
	};
	xPlatform.usDataLen = ulStrLen(xPlatform.pucData);
	xApplication.usDataLen = ulStrLen(xApplication.pucData);
	eBluetoothWriteLocalCharacteristic(&xPlatform);
	eBluetoothWriteLocalCharacteristic(&xApplication);

	/* Setup Bluetooth with TX power value in NVM, or 8 dBm if it does not yet exist in NVM */
	int32_t lTxPower;
	int32_t lTxPowerDefault = 8;
	configASSERT(eNvmReadDataDefault(NVM_BLUETOOTH_TX_POWER_DBM, &lTxPower, &lTxPowerDefault) == ERROR_NONE);
	lTxPower = cBluetoothSetTxPower((int8_t)lTxPower);
	eLog(LOG_APPLICATION, LOG_VERBOSE, "Bluetooth TX Power set to %ddBm\r\n", lTxPower);
}

/*-----------------------------------------------------------*/

void prvBoardPeripheralInit(void)
{
	return;
}

/*-----------------------------------------------------------*/

void vBoardWatchdogPeriodic(void)
{
	vWatchdogPeriodic(pxWatchdog);
}

/*-----------------------------------------------------------*/

eModuleError_t eBoardAdcRecalibrate(void)
{
	return eAdcRecalibrate(pxAdc);
}

/*-----------------------------------------------------------*/

uint32_t ulBoardAdcSample(xGpio_t xGpio, eAdcResolution_t eResolution, eAdcReferenceVoltage_t eReferenceVoltage)
{
	return ulAdcSample(pxAdc, xGpio, eResolution, eReferenceVoltage);
}

/*-----------------------------------------------------------*/
