#include "phi_lib/phi_btn.h"

void phi_btn_init(phi_btn_t * btn)
{
    memset(btn, 0, sizeof(*btn));
}

void phi_btn_update_state(phi_btn_t * btn, systime_t now, bool is_pressed)
{
    btn->cur_state = is_pressed;
    btn->cur_state_time = now;
}

void phi_btn_process(phi_btn_t * btns, uint8_t n_btns, phi_btn_event_callback_f callback, systime_t debounce, systime_t hold_threshold, systime_t hold_interval)
{
    phi_btn_t * btn;
    int i;

    for (i = 0; i < n_btns; ++i)
    {
        btn = &(btns[i]);

        // Test for button pressed and store the down time
        if (btn->cur_state &&
            !btn->is_pressed &&
            (btn->cur_state_time - btn->last_released) > debounce)
        {
            btn->last_pressed = btn->cur_state_time;
            callback(i, PHI_BTN_EVENT_PRESSED, 0);
        }

        // Test for button release and store the up time
        if (!btn->cur_state &&
            btn->is_pressed &&
            (btn->cur_state_time - btn->last_pressed) > debounce)
        {
            callback(i, PHI_BTN_EVENT_RELEASED, btn->ignore_release ? 1 : 0);
            btn->last_released = btn->cur_state;
            btn->ignore_release = FALSE;
        }

        // Test for button held down for longer than the hold time
        if (btn->is_pressed &&
            btn->cur_state &&
            (btn->cur_state_time - btn->last_pressed) > hold_threshold)
        {
            if ((btn->cur_state_time - btn->last_held) > hold_interval)
            {
                callback(i, PHI_BTN_EVENT_HELD, ST2MS(btn->cur_state_time - btn->last_pressed));
                btn->last_held = btn->cur_state_time;
                btn->ignore_release = TRUE;
            }
        }

        btn->is_pressed = btn->cur_state;
    }
}
