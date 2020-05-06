import pyb
from pyb import UART
from machine import I2C
                                                    
class I2C_SENSOR():

    def __init__(self,payload,payload_len,i2c_obj):
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

        self._is_write = ((~((payload[self.I2C_ADDR_IDX] & self.I2C_ADDR_LOW_1_MASK))) & self.I2C_ADDR_LOW_1_MASK)
        self._i2c_obj = i2c_obj

        self._buf = bytearray([0x00])

        self.LSM6DSL_ADDR = 0x6A
        self.LSM6DSL_X_LOW_REG = 0x28
        self.LSM6DSL_X_HIGH_REG = 0x29

        self.LSM6DSL_Y_LOW_REG = 0x2A
        self.LSM6DSL_Y_HIGH_REG = 0x2B

        self.LSM6DSL_Z_LOW_REG = 0x2C
        self.LSM6DSL_Z_HIGH_REG = 0x2D

        self.LSM6DSL_GYRO_X_LOW_REG = 0x22
        self.LSM6DSL_GYRO_X_HIGH_REG = 0x23

        self.LSM6DSL_GYRO_Y_LOW_REG = 0x24
        self.LSM6DSL_GYRO_Y_HIGH_REG = 0x25

        self.LSM6DSL_GYRO_Z_LOW_REG = 0x26
        self.LSM6DSL_GYRO_Z_HIGH_REG = 0x27

        self._lsm6dsl_x_buf = bytearray([0x00,0x00])
        self._lsm6dsl_y_buf = bytearray([0x00,0x00])
        self._lsm6dsl_z_buf = bytearray([0x00,0x00])

        self._lsm6dsl_gyro_x_buf = bytearray([0x00,0x00])
        self._lsm6dsl_gyro_y_buf = bytearray([0x00,0x00])
        self._lsm6dsl_gyro_z_buf = bytearray([0x00,0x00])


        self._replay_packet_size = 0


        if self._is_write:
            #print('[I2C_SENSOR] sid = {}, i2c_address = {}, reg_addr = {}, reg_value = {}'.format(self._sid,self._i2c_address,self._reg_addr,self._reg_value))
            self._buf = bytearray([self._reg_value])
            self._i2c_obj.writeto_mem(self._i2c_address, self._reg_addr, self._buf, addrsize=8)
            self._replay_packet_size = 0
        else:
            if self._i2c_address == self.LSM6DSL_ADDR:
                if self._reg_value == ord('a'):

                    # ACC X LOW
                    self._reg_addr = bytearray([self.LSM6DSL_X_LOW_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_x_buf[0] = self._buf[0]

                    self.self_string()
                    # ACC X HIGH
                    self._reg_addr = bytearray([self.LSM6DSL_X_HIGH_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_x_buf[1] = self._buf[0]

                    self.self_string()
                    # ACC Y LOW
                    self._reg_addr = bytearray([self.LSM6DSL_Y_LOW_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_y_buf[0] = self._buf[0]
                    self.self_string()

                    # ACC Y HIGH
                    self._reg_addr = bytearray([self.LSM6DSL_Y_HIGH_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_y_buf[1] = self._buf[0]
                    self.self_string()

                    # ACC Z LOW
                    self._reg_addr = bytearray([self.LSM6DSL_Z_LOW_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_z_buf[0] = self._buf[0]
                    self.self_string()

                    # ACC Z HIGH
                    self._reg_addr = bytearray([self.LSM6DSL_Z_HIGH_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_z_buf[1] = self._buf[0]
                    self.self_string()

                    # GYRO X LOW
                    self._reg_addr = bytearray([self.LSM6DSL_GYRO_X_LOW_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_gyro_x_buf[0] = self._buf[0]
                    self.self_string()

                    # GYRO X HIGH
                    self._reg_addr = bytearray([self.LSM6DSL_GYRO_X_HIGH_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_gyro_x_buf[1] = self._buf[0]
                    self.self_string()

                    # GYRO Y LOW
                    self._reg_addr = bytearray([self.LSM6DSL_GYRO_Y_LOW_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_gyro_y_buf[0] = self._buf[0]
                    self.self_string()

                    # GYRO Y HIGH
                    self._reg_addr = bytearray([self.LSM6DSL_GYRO_Y_HIGH_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_gyro_y_buf[1] = self._buf[0]
                    self.self_string()

                    # GYRO Z LOW
                    self._reg_addr = bytearray([self.LSM6DSL_GYRO_Z_LOW_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_gyro_z_buf[0] = self._buf[0]
                    self.self_string()

                    # GYRO Z HIGH
                    self._reg_addr = bytearray([self.LSM6DSL_GYRO_Z_HIGH_REG])[0]
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_gyro_z_buf[1] = self._buf[0]
                    self.self_string()

                    self._replay_packet_size = 12
                    return
                if self._reg_value == ord('x'):
                    self._reg_addr = bytearray([self.LSM6DSL_X_LOW_REG])[0]
                    #self._i2c_obj.readfrom_mem_into(self._i2c_address, self.LSM6DSL_X_LOW_REG, self._buf,addrsize=8)
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_x_buf[0] = self._buf[0]
                    self.self_string()

                    self._reg_addr = bytearray([self.LSM6DSL_X_HIGH_REG])[0]
                    #self._i2c_obj.readfrom_mem_into(self._i2c_address, self.LSM6DSL_X_HIGH_REG, self._buf,addrsize=8)
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_x_buf[1] = self._buf[0]
                    self.self_string()

                    self._replay_packet_size = 2
                    return

                if self._reg_value == ord('y'):
                    self._reg_addr = bytearray([self.LSM6DSL_Y_LOW_REG])[0]
                    #self._i2c_obj.readfrom_mem_into(self._i2c_address, self.LSM6DSL_Y_LOW_REG, self._buf,addrsize=8)
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_y_buf[0] = self._buf[0]
                    self.self_string()

                    self._reg_addr = bytearray([self.LSM6DSL_Y_HIGH_REG])[0]
                    #self._i2c_obj.readfrom_mem_into(self._i2c_address, self.LSM6DSL_Y_HIGH_REG, self._buf,addrsize=8)
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_y_buf[1] = self._buf[0]
                    self.self_string()

                    self._replay_packet_size = 2
                    return

                if self._reg_value == ord('z'):
                    self._reg_addr = bytearray([self.LSM6DSL_Z_LOW_REG])[0]
                    #self._i2c_obj.readfrom_mem_into(self._i2c_address, self.LSM6DSL_Z_LOW_REG, self._buf,addrsize=8)
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_z_buf[0] = self._buf[0]
                    self.self_string()

                    self._reg_addr = bytearray([self.LSM6DSL_Z_HIGH_REG])[0]
                    #self._i2c_obj.readfrom_mem_into(self._i2c_address, self.LSM6DSL_Z_HIGH_REG, self._buf,addrsize=8)
                    self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
                    self._lsm6dsl_z_buf[1] = self._buf[0]
                    self.self_string()

                    self._replay_packet_size = 2
                    return

            self._i2c_obj.readfrom_mem_into(self._i2c_address, self._reg_addr, self._buf,addrsize=8)
            self._replay_packet_size = 1

    def construct_reply_data_payload(self):
        if self._replay_packet_size == 1:
            return self._buf
        if self._i2c_address == self.LSM6DSL_ADDR:
            if self._replay_packet_size == 2:
                if self._reg_value == ord('x'):
                    return self._lsm6dsl_x_buf
                if self._reg_value == ord('y'):
                    return self._lsm6dsl_y_buf
                if self._reg_value == ord('z'):
                    return self._lsm6dsl_z_buf
            if self._replay_packet_size > 2:
                if self._reg_value == ord('a'):
                    buffer = bytearray([])

                    buffer += self._lsm6dsl_x_buf
                    buffer += self._lsm6dsl_y_buf
                    buffer += self._lsm6dsl_z_buf

                    buffer += self._lsm6dsl_gyro_x_buf
                    buffer += self._lsm6dsl_gyro_y_buf
                    buffer += self._lsm6dsl_gyro_z_buf
                    return buffer


    def self_string(self):
        #print('[I2C_SENSOR] sid = {}, i2c_address = {}, reg_addr = {}, reg_value = {}'.format(self._sid,self._i2c_address,self._reg_addr,self._reg_value))
        #print('[I2C_SENSOR] result = {}'.format(self._buf))
        return

class HCI_UART(UART):                                   


    def __init__(self):                             
        #super().__init__(2, 115200)
        super().__init__(4, 115200)
        self.init(115200, bits=8, parity=None, stop=1)

        self._i2c = I2C(2,freq=400000)

        self.HCI_HEADER = 0xAA
        self.HCI_TYPE_REQUEST = 0x01
        self.HCI_TYPE_RESPONSE = 0x02

        self.HCI_SECOND_BYTE_HIGH_4_MASK = 0xF0
        self.HCI_SECOND_BYTE_LOW_4_MASK = 0x0F

        self._buf = bytearray([0x00])
        self._buf_mview = memoryview(self._buf)
        self._curSize = 0
        self._payload_size = 0
        buffer = bytearray([])
                                                    
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

                        # Execute I2C
                        temp_i2c = I2C_SENSOR(self._payload,self._payload_size,self._i2c)
                        temp_i2c.self_string()

                        # Send Reply
                        self.construct_reply_package(temp_i2c)
                        self.write(self._replay_byte)

                        self._curSize = 0
                else:
                    self._curSize = 0
                if self._curSize == 0:
                    break

    def construct_reply_package(self,temp_i2c):
        self._replay_byte = bytearray([self.HCI_HEADER])
        self._replay_byte += bytearray([(self.HCI_TYPE_RESPONSE & 0x0F) | ((temp_i2c._replay_packet_size & 0x0F) << 4)])
        if temp_i2c._replay_packet_size != 0:
            self._replay_byte += temp_i2c.construct_reply_data_payload()
