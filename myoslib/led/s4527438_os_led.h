/** 
 **************************************************************
 * @file mylib/s4527438_os_radio.h
 * @author KO-CHEN CHI - s4527438
 * @date 26032019
 * @brief mylib radio driver
 * REFERENCE: 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 *************************************************************** 
 */

#ifndef S4527438_OS_ATIMER_H_
#define S4527438_OS_ATIMER_H_

typedef enum{
    MY_OS_LIB_LED_CMD_ON,
    MY_OS_LIB_LED_CMD_OFF,
    MY_OS_LIB_LED_CMD_TOGGLE,
    MY_OS_LIB_LED_CMD_NONE,
} LED_Cmd_Enum;

typedef enum{
    MY_OS_LIB_LED_COLOR_RED,
    MY_OS_LIB_LED_COLOR_GREEN,
    MY_OS_LIB_LED_COLOR_BLUE,
    MY_OS_LIB_LED_COLOR_NONE,
} LED_Color_Enum;


void s4527438_os_led_init(void);
void s4527438_os_led_command(LED_Cmd_Enum led_cmd,LED_Color_Enum led_color);
#endif /* S4527438_OS_RADIO_H_ */
