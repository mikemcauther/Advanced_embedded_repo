# Machine is a MicroPython library.
from machine import Pin
import time

# Get PB14 and treat it as a GPIO Output pin
led_pin_1 = Pin('PB14', Pin.OUT)
led_pin_2 = Pin('PA5', Pin.OUT)
value = 1

# Toggle the pin every one second
while 1:
    if value == 1:
        led_pin_1.value(1)
        led_pin_2.value(0)
        value = 2
    else:
        led_pin_1.value(0)
        led_pin_2.value(1)
        value = 1

    # Somewhat similar to HAL_Delay()
    time.sleep(1)
