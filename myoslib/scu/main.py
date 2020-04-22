import hci
import led
import time

# LED 
glb_led_ctrl = led.LED_CTRL()

#HCI
glb_hci =   hci.HCI_UART()

while True:
    if glb_hci.any():
        glb_hci.startReceive()

    time.sleep(1)

