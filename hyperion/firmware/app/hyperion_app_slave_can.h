/*
 * hyperion_app_slave_can.h
 *
 *  Created on: Apr 8, 2017
 *      Author: eran
 */

#ifndef HYPERION_APP_SLAVE_CAN_H_
#define HYPERION_APP_SLAVE_CAN_H_

#include "phi_lib/phi_can.h"

enum
{
	HYPERION_APP_SLAVE_CAN_MSG_ID_SET_BTN_LED = PHI_CAN_MSG_ID_USER,
	HYPERION_APP_SLAVE_CAN_MSG_ID_SET_LED_BAR,
};

typedef struct {
    uint8_t btn_num;
    uint8_t led;
} hyperion_app_slave_msg_set_btn_led_t;

typedef struct {
    uint8_t val;
    uint8_t fill;
} hyperion_app_slave_msg_set_led_bar_t;


#endif /* HYPERION_APP_SLAVE_CAN_H_ */
