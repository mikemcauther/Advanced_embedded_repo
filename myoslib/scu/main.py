import hci
import led
import select

# LED 
glb_led_ctrl = led.LED_CTRL()

#HCI
glb_hci =   hci.HCI_UART()

while True:
    select.select([glb_hci], [], [])
    if glb_hci.any():
        glb_hci.startReceive()

