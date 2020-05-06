/** 
 **************************************************************
 * @file mylib/s4527438_cli_hci.c
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib cli hci driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"

#include "FreeRTOS_CLI.h"

#include "s4527438_os_hci.h"
#include "s4527438_lib_log.h"
#include "s4527438_hci_packet.h"

#include "memory_operations.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint8_t convert_2_char_to_1_hex_byte(unsigned char *input_2_char);
static void string_to_hex(char *input_string,unsigned char input_length,uint8_t *output_buffer,unsigned char output_length);

static BaseType_t prvReadI2CRegGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvWriteI2CRegGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvLSM6DSLGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );


CLI_Command_Definition_t xReadI2CRegGetSys = {  /* Structure that defines the "i2creg r <sid> <regaddr>" command line command. */
    "i2creg",
    "i2creg: i2creg \r\n",
    prvReadI2CRegGetSysCommand,
    3
};

CLI_Command_Definition_t xWriteI2CRegGetSys = {  /* Structure that defines the "i2creg w <sid> <regaddr> <regval>" command line command. */
    "i2creg",
    "i2creg: i2creg \r\n",
    prvWriteI2CRegGetSysCommand,
    4
};

CLI_Command_Definition_t xLSM6DSLGetSys = {  /* Structure that defines the " lsm6dsl r x" command line command. */
    "lsm6dsl",
    "lsm6dsl: lsm6dsl \r\n",
    prvLSM6DSLGetSysCommand,
    2
};

void s4527438_cli_hci_init(void) {
    /* Register CLI commands */
    FreeRTOS_CLIRegisterCommand(&xReadI2CRegGetSys);
    FreeRTOS_CLIRegisterCommand(&xWriteI2CRegGetSys);
    FreeRTOS_CLIRegisterCommand(&xLSM6DSLGetSys);
}

static uint8_t convert_2_char_to_1_hex_byte(unsigned char *input_2_char) {
    uint8_t result = 0;
    unsigned char target_char = '0';
    unsigned char i = 0;

    i = 0;
    while(1) {
        if( i >= 2 ) {
            break;
        }
        result *= 16;
        target_char = (*input_2_char);
        if( target_char >= '0' && target_char <= '9' ) {
            result += (target_char - '0');
        } else {
            if( target_char >= 'A' && target_char <= 'F' ) {
                result += (target_char - 'A') + 10;
            } else if( target_char >= 'a' && target_char <= 'f' ) {
                result += (target_char - 'a') + 10;
            }
        }
        input_2_char++;
        i++;
    }
    return result;
}

static void string_to_hex(char *input_string,unsigned char input_length,uint8_t *output_buffer,unsigned char output_length) {
    unsigned char j = 0;
#if  1
    char *input_string_end = input_string;

    input_string = input_string + input_length - 2;

    if( strcmp(input_string_end,"0x") == 0 ) {
        input_string_end += 2;
    }

    while(1) {
        if( (input_string - input_string_end) < 0 ) {
            break;
        }
        if( j >= output_length ) {
            break;
        }
        output_buffer[j] = convert_2_char_to_1_hex_byte(input_string);
        input_string -= 2;
        j++;
    }
#endif
}

static BaseType_t prvReadI2CRegGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;

    uint8_t  sid = 0;
    uint8_t I2CRegAddr = 0;

    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string == NULL ) {
        return returnedValue;
    }
    if( cCmd_string[0] != 'r' ) {
        return returnedValue;
    }

    /* Get parameters 2 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lParam_len);

    if( cCmd_string != NULL ) {
        int i = 0;
        uint8_t numeric_string[4];

        memset(numeric_string,0x00,sizeof(numeric_string));
        for( i = 0; i < lParam_len ;i++) {
            if( cCmd_string[i] < '0'
                || cCmd_string[i] > '9' ) {
                return returnedValue;
            }
            numeric_string[i] = cCmd_string[i];
        }
        sid = atoi(numeric_string);
        if( sid < HCI_PACKET_DATA_I2C_SID_MIN || sid > HCI_PACKET_DATA_I2C_SID_MAX ){
            return returnedValue;
        }
    } else {
        return returnedValue;
    }

    /* Get parameters 3 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 3, &lParam_len);

    if( cCmd_string != NULL ) {
        string_to_hex(cCmd_string,lParam_len,&I2CRegAddr,sizeof(I2CRegAddr));
    } else {
        return returnedValue;
    }

    /* Build Message Packet */
    s4527438_os_hci_read_reg_cmd(sid, I2CRegAddr, 0,1,1);

    return returnedValue;
}

static BaseType_t prvWriteI2CRegGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;

    uint8_t  sid = 0;
    uint8_t I2CRegAddr = 0;
    uint8_t I2CRegValue = 0;

    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string == NULL ) {
        return returnedValue;
    }
    if( cCmd_string[0] != 'w' ) {
        return returnedValue;
    }

    /* Get parameters 2 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lParam_len);

    if( cCmd_string != NULL ) {
        int i = 0;
        uint8_t numeric_string[4];

        memset(numeric_string,0x00,sizeof(numeric_string));
        for( i = 0; i < lParam_len ;i++) {
            if( cCmd_string[i] < '0'
                || cCmd_string[i] > '9' ) {
                return returnedValue;
            }
            numeric_string[i] = cCmd_string[i];
        }
        sid = atoi(numeric_string);
        if( sid < HCI_PACKET_DATA_I2C_SID_MIN || sid > HCI_PACKET_DATA_I2C_SID_MAX ){
            return returnedValue;
        }
    } else {
        return returnedValue;
    }

    /* Get parameters 3 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 3, &lParam_len);

    if( cCmd_string != NULL ) {
        string_to_hex(cCmd_string,lParam_len,&I2CRegAddr,sizeof(I2CRegAddr));
    } else {
        return returnedValue;
    }

    /* Get parameters 4 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 4, &lParam_len);

    if( cCmd_string != NULL ) {
        string_to_hex(cCmd_string,lParam_len,&I2CRegValue,sizeof(I2CRegValue));
    } else {
        return returnedValue;
    }

    /* Build Message Packet */
    s4527438_os_hci_write_reg_cmd(sid, I2CRegAddr, I2CRegValue,1,1);

    return returnedValue;
}

static BaseType_t prvLSM6DSLGetSysCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
 
    long lParam_len;
    const char *cCmd_string;
    // We always need to stop search further, even if the command is not correct
    BaseType_t returnedValue = pdFALSE;
    uint8_t  sid = 1;
    uint8_t I2CRegAddr = 0;

    uint8_t num_word = 1;
    uint8_t word_size = 2;
    /* Get parameters 1 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    if( cCmd_string == NULL ) {
        return returnedValue;
    }
    if( cCmd_string[0] != 'r' ) {
        return returnedValue;
    }

    /* Get parameters 2 from command string */
    cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lParam_len);

    if( cCmd_string == NULL ) {
        return returnedValue;
    }
    if( cCmd_string[0] == 'a' ) {
        num_word = 6;
        word_size = 2;
        s4527438_os_hci_read_reg_cmd(sid, I2CRegAddr, 'a',num_word,word_size);
        /* Build Message Packet */
        //s4527438_os_hci_read_reg_cmd(sid, I2CRegAddr, 'x',num_word,word_size);

        /* Build Message Packet */
        //s4527438_os_hci_read_reg_cmd(sid, I2CRegAddr, 'y',num_word,word_size);

        /* Build Message Packet */
        //s4527438_os_hci_read_reg_cmd(sid, I2CRegAddr, 'z',num_word,word_size);
        
        return returnedValue;
    }
    s4527438_os_hci_read_reg_cmd(sid, I2CRegAddr, cCmd_string[0],num_word,word_size);

    return returnedValue;
}
