#ifndef PHI_LIB_PHI_BTN_H_
#define PHI_LIB_PHI_BTN_H_

#include "phi_lib/phi_lib.h"

typedef enum phi_btn_event_e
{
    PHI_BTN_EVENT_INVALID = 0,
    PHI_BTN_EVENT_PRESSED,
    PHI_BTN_EVENT_HELD,
    PHI_BTN_EVENT_RELEASED,
} phi_btn_event_t;

typedef void (* phi_btn_event_callback_f)(uint8_t btn, phi_btn_event_t event, uint32_t param);

typedef struct phi_btn_s
{
    bool is_pressed;
    systime_t last_pressed;
    systime_t last_released;
    systime_t last_held;
    bool ignore_release; // whether to ignore the button release because press+hold was triggered

    bool cur_state;
    systime_t cur_state_time;
} phi_btn_t;

void phi_btn_init(phi_btn_t * btn);
void phi_btn_update_state(phi_btn_t * btn, systime_t now, bool is_pressed);
void phi_btn_process(phi_btn_t * btns, uint8_t n_btns, phi_btn_event_callback_f callback, systime_t debounce, systime_t hold_threshold, systime_t hold_interval);

#endif /* PHI_LIB_PHI_BTN_H_ */
