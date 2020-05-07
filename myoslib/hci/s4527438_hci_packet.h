
/**
 **************************************************************
 * @file mylib/s4527438_hci_packet.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib hci driver
 * REFERENCE:
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 ***************************************************************
 */

#ifndef S4527438_HCI_PACKET_H_
#define S4527438_HCI_PACKET_H_


#define MAX_PACKET_BUFFER       256

#define HCI_PACKET_FIELD_PREAMBLE     0xAA

#define HCI_PACKET_TYPE_REQUEST             0x01
#define HCI_PACKET_TYPE_RESPONSE            0x02
#define HCI_PACKET_TYPE_CONTINUOUS          0x03

#define HCI_PACKET_FIELD_TYPE_REQUEST_WITH_LENGTH(length_8_bits)   ((HCI_PACKET_TYPE_REQUEST & 0x0F) | ((0x0F & length_8_bits) << 4))
#define HCI_PACKET_FIELD_TYPE_RESPONSE_WITH_LENGTH(length_8_bits)   ((HCI_PACKET_TYPE_RESPONSE & 0x0F) | ((0x0F & length_8_bits) << 4))

#define HCI_PACKET_FIELD_TYPE_LEN_GET_LENGTH(input_8_bits)   ((0xF0 & input_8_bits) >> 4)
#define HCI_PACKET_FIELD_TYPE_LEN_GET_TYPE(input_8_bits)   (0x0F & input_8_bits)

#define HCI_PACKET_INTERNAL_DATA_FIELD_ARRAY_0_FOR_PACKET_ROUTE 0
#define HCI_PACKET_INTERNAL_DATA_FIELD_ARRAY_1_FOR_WORD_INFO    1
#define HCI_PACKET_INTERNAL_DATA_FIELD_ARRAY_2_FOR_WORD_INFO    1

#define HCI_PACKET_DATA_FIELD_NUM_WORD_AND_WORD_SIZE(num_word,word_size)   ((word_size & 0x0F) | ((0x0F & num_word) << 4))
#define HCI_PACKET_DATA_LOW_GET_WORD_SIZE(input)   (0x0F & input)
#define HCI_PACKET_DATA_HIGH_GET_NUM_WORD(input)   ((0xF0 & input) >> 4)

#define HCI_PACKET_DATA_I2C_ADDR_WRITE_HTS221       0xBE
#define HCI_PACKET_DATA_I2C_ADDR_READ_HTS221       0xBF

#define HCI_PACKET_DATA_I2C_ADDR_WRITE_LIS3MDL       0x3C
#define HCI_PACKET_DATA_I2C_ADDR_READ_LIS3MDL       0x3D

#define HCI_PACKET_DATA_I2C_ADDR_WRITE_LPS22HB       0xBA
#define HCI_PACKET_DATA_I2C_ADDR_READ_LPS22HB       0xBB

#define HCI_PACKET_DATA_I2C_ADDR_WRITE_LSM6DSL       0xD4
#define HCI_PACKET_DATA_I2C_ADDR_READ_LSM6DSL       0xD5

#define HCI_PACKET_DATA_I2C_ADDR_WRITE_VL53L0X       0x52
#define HCI_PACKET_DATA_I2C_ADDR_READ_VL53L0X       0x53

#define HCI_PACKET_DATA_I2C_ADDR_WRITE_M24SR64_Y       0xAC
#define HCI_PACKET_DATA_I2C_ADDR_READ_M24SR64_Y       0xAD

#define HCI_PACKET_DATA_I2C_ADDR_WRITE_STSAFE_A100       0x40
#define HCI_PACKET_DATA_I2C_ADDR_READ_STSAFE_A100       0x41

#define HCI_PACKET_DATA_I2C_SID_MAX       5
#define HCI_PACKET_DATA_I2C_SID_MIN       1

typedef struct xHCIDataField_t
{
    uint8_t  ucSID;
    uint8_t  ucI2CAddress;
    uint8_t  ucI2CRegAddr;
    uint8_t  ucI2CRegValue;
} ATTR_PACKED xHCIDataField_t;

#endif /* S4527438_HCI_PACKET_H_ */
