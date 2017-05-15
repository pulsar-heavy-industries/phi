/*
 * hyperion_app_slave_can.h
 *
 *  Created on: Apr 8, 2017
 *      Author: eran
 */

#ifndef HYPERION_APP_SLAVE_CAN_H_
#define HYPERION_APP_SLAVE_CAN_H_

#include "hyperion_can.h"
#include "hyperion_ui.h"

#ifndef HYPERION_DEV_ID
#define HYPERION_DEV_ID PHI_DEV_ID('H', 'P', 'R', 'N')
#else
#if HYPERION_DEV_ID != PHI_DEV_ID('H', 'P', 'R', 'N')
#error "WTF"
#endif
#endif

#pragma pack (1)

typedef struct {
	hyperion_ui_dispmode_t dispmode;
	hyperion_ui_dispmode_state_t state;
} hyperion_app_slave_msg_update_display_state_t;

typedef struct {
    uint8_t btn_num;
    uint8_t led;
} hyperion_app_slave_msg_set_btn_led_t;

typedef struct {
    uint8_t val;
    uint8_t fill;
} hyperion_app_slave_msg_set_led_bar_t;


#endif /* HYPERION_APP_SLAVE_CAN_H_ */
