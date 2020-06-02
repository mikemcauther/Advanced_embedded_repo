# WI-FI
## Purpose

Test the ESP32 AT mode with the nrf52840 chip and attempt to establish a connection over a wifi network and send data.

## Operation Summary

Connects to the specified network in the wifi_test.c file. This network connection will contantly be maintained, if it fails, a request to reconnect will continuously be sent in order to reconnnect.

Use the eEspTestAT() function to send through random test commands if you're trying to do something else with the ESP AT mode.

Once connected for the first time it will attempt to send through some data.

Change the following values in argon-ncp-firmware (see Resources) - I recommend the following:
* const auto UART_CONF_TX_PIN  = 17;
* const auto UART_CONF_RX_PIN  = 16;
* const auto UART_CONF_RTS_PIN = 18;
* const auto UART_CONF_CTS_PIN = 19;
* See argon.h for what these should be connected to on the argon board

### Gatt Characteristics

None

## Resources

* [ESP32 AT Instruction Set Examples](https://www.espressif.com/sites/default/files/documentation/esp32_at_instruction_set_and_examples_en.pdf)
* [Internal ESP32 Firmware Used on Argon](https://github.com/particle-iot/argon-ncp-firmware)
* [Argon Default Firmware](https://github.com/particle-iot/device-os)
* [Argon block diagram and pinout](https://docs.particle.io/datasheets/wi-fi/argon-datasheet/)

## Known Issues / Limitations

* If the network never connects after startup it will not re-attempt to connect.
* System doesn't seem to accept commands instantly after startup. If you send any it will not send a response back until you send another command and the second command will be blocked as the esp is 'busy'.
* The internal ESP seems to have a different program loaded and will send a different response at 115200Hz compared to the external (may have been wiped?).
* The ESP32 AP command firmware is ineffient as it require strings to be sent for commands
* Time taken to achieve IP address is approximately 6.5 seconds
