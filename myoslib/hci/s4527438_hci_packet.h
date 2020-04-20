
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

#define HCI_PACKET_TYPE_REQUEST         0x01
#define HCI_PACKET_TYPE_RESPONSE        0x02

#define HCI_PACKET_FIELD_TYPE_REQUEST_WITH_LENGTH(length_8_bits)   ((HCI_PACKET_TYPE_REQUEST << 4) | (0x0F & length_8_bits))
#define HCI_PACKET_FIELD_TYPE_RESPONSE_WITH_LENGTH(length_8_bits)   ((HCI_PACKET_TYPE_RESPONSE << 4) | (0x0F & length_8_bits))

#define HCI_PACKET_FIELD_TYPE_LEN_GET_LENGTH(input_8_bits)   ((0x0F & input_8_bits))
#define HCI_PACKET_FIELD_TYPE_LEN_GET_TYPE(input_8_bits)   ((0xF0 & input_8_bits)>>4)

#endif /* S4527438_HCI_PACKET_H_ */
