import pyb
from pyb import UART
                                                    
class I2C_SENSOR():

    def __init__(self,payload,payload_len):
        self.I2C_IDX = 0
        self.I2C_ADDR_IDX = 1
        self.I2C_REG_ADDR = 2
        self.I2C_REG_VALUE = 3

        self.I2C_ADDR_HIGH_7_MASK = 0xFE
        self.I2C_ADDR_LOW_1_MASK = 0x01

        self._is_write = 0

        self._sid = payload[self.I2C_IDX]
        self._i2c_address = (payload[self.I2C_ADDR_IDX] & self.I2C_ADDR_HIGH_7_MASK) >> 1
        self._reg_addr = payload[self.I2C_REG_ADDR]
        self._reg_value = payload[self.I2C_REG_VALUE]

        self._is_write = ~((payload[self.I2C_ADDR_IDX] & self.I2C_ADDR_LOW_1_MASK))

    def self_string(self):
        print('[I2C_SENSOR] sid = {}, i2c_address = {}, reg_addr = {}, reg_value = {}'.format(self._sid,self._i2c_address,self._reg_addr,self._reg_value))

class HCI_UART(UART):                                   


    def __init__(self):                             
        #super().__init__(2, 115200)
        super().__init__(4, 115200)
        self.init(115200, bits=8, parity=None, stop=1)

        self.HCI_HEADER = 0xAA
        self.HCI_TYPE_REQUEST = 0x01
        self.HCI_TYPE_RESPONSE = 0x02

        self.HCI_SECOND_BYTE_HIGH_4_MASK = 0xF0
        self.HCI_SECOND_BYTE_LOW_4_MASK = 0x0F

        self._buf = bytearray([0x00])
        self._buf_mview = memoryview(self._buf)
        self._curSize = 0
        self._payload_size = 0
                                                    
    def startReceive(self):
        self._curSize = 0
        self._payload_size = 0
        while 1:
            self.readinto(self._buf_mview)
            if (self._buf_mview[0] & self.HCI_HEADER):
                self._curSize = 1
            else:
                if self._curSize == 1:
                    if (self._buf_mview[0] & self.HCI_SECOND_BYTE_LOW_4_MASK) == self.HCI_TYPE_REQUEST:
                        self._payload_size = ((self._buf_mview[0] & self.HCI_SECOND_BYTE_HIGH_4_MASK) >> 4)
                        self._payload = self.read(self._payload_size)
                        temp_i2c = I2C_SENSOR(self._payload,self._payload_size)
                        temp_i2c.self_string()
                        self._curSize = 0
                else:
                    self._curSize = 0
                if self._curSize == 0:
                    break
