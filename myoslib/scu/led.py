# Machine is a MicroPython library.
from machine import Pin
import time
import pyb

class LED_CTRL():
    def __init__(self):
        # Get PB14 and treat it as a GPIO Output pin
        self._led_pin_1 = Pin('PB14', Pin.OUT)
        self._led_pin_2 = Pin('PA5', Pin.OUT) 
        self._value = 1                       

        self.tim = pyb.Timer(4)              # create a timer object using timer 4
        self.tim.init(freq=1)                # trigger at 2Hz
        self.tim.callback(lambda t:self.toggle())
    def toggle(self):
        if self._value == 1:
            self._led_pin_1.value(1)
            self._led_pin_2.value(0)
            self._value = 2
        else:
            self._led_pin_1.value(0)
            self._led_pin_2.value(1)
            self._value = 1

