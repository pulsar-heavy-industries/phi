#include "phi_lib/phi_rotenc.h"

void phi_rotenc_init(phi_rotenc_t * rotenc)
{
    memset(rotenc, 0, sizeof(*rotenc));
    rotenc->state = 0xf;
}

void phi_rotenc_update_state(phi_rotenc_t * rotenc, uint8_t new_state)
{
    int32_t delta;

    rotenc->last_state = rotenc->active_state;
    rotenc->active_state = new_state;

    // De-accelerate on every update cycle
    if (rotenc->accel)
    {
        --rotenc->accel;
    }

    // See if state actually changed
    if (rotenc->last_state == rotenc->active_state)
    {
        return;
    }

    // Check for increment
    if ((rotenc->state == 0x04) && // 01 00
        ((rotenc->last_action == PHI_ROTENC_LAST_ACTION_INC) || (rotenc->accel <= 200)))
    {
        rotenc->last_action = PHI_ROTENC_LAST_ACTION_INC;

        if ((((int32_t) rotenc->accel) - ((int32_t) rotenc->prev_accel)) > 20)
        {
            rotenc->accel = rotenc->prev_accel + 20;
        }

        if (rotenc->speed)
        {
            uint32_t speed = rotenc->accel & 0xc0;
            delta = speed >> (7 - rotenc->speed);
            if (delta == 0) {
                delta = 1;
            }

            rotenc->value = phi_min(rotenc->value + delta, 70);
        }
        else
        {
            ++rotenc->value;
        }

        rotenc->prev_accel = rotenc->accel;
        rotenc->accel = 0xff;
    }

    // Check for decrement
    if ((rotenc->state == 0x08) && // 10 00
        ((rotenc->last_action == PHI_ROTENC_LAST_ACTION_DEC) || (rotenc->accel <= 200)))
    {
        rotenc->last_action = PHI_ROTENC_LAST_ACTION_DEC;

        if ((((int32_t) rotenc->accel) - ((int32_t) rotenc->prev_accel)) > 20)
        {
            rotenc->accel = rotenc->prev_accel + 20;
        }

        if (rotenc->speed)
        {
            uint32_t speed = rotenc->accel & 0xc0;
            delta = speed >> (7 - rotenc->speed);
            if (delta == 0) {
                delta = 1;
            }

            rotenc->value = phi_max(rotenc->value - delta, -70);
        }
        else
        {
            --rotenc->value;
        }

        rotenc->prev_accel = rotenc->accel;
        rotenc->accel = 0xff;
    }
}

void phi_rotenc_process(phi_rotenc_t * rotencs, uint8_t n_rotencs, phi_rotenc_callback_f callback)
{
    uint8_t i;
    int32_t value;

    for (i = 0; i < n_rotencs; ++i)
    {
        value = rotencs[i].value;
        rotencs[i].value = 0;

        if (value)
        {
            callback(i, value);
        }
    }
}
