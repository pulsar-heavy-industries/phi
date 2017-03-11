#ifndef PHI_LIB_PHI_ROTENC_H_
#define PHI_LIB_PHI_ROTENC_H_

#include "phi_lib/phi_lib.h"

#define PHI_ROTENC_LAST_ACTION_INC 0
#define PHI_ROTENC_LAST_ACTION_DEC 1

typedef void (* phi_rotenc_callback_f)(uint8_t rotenc, int8_t value);

typedef struct {
    // Dynamic configuration
    uint8_t speed;

    // Internal state
    union {
        struct {
            uint8_t active_state : 2;
            uint8_t last_state : 2;
        };
        uint8_t state : 4;
    };
    uint8_t last_action;
    int8_t value;
    uint8_t accel;
    uint8_t prev_accel;
} phi_rotenc_t;


void phi_rotenc_init(phi_rotenc_t * rotenc);
void phi_rotenc_update_state(phi_rotenc_t * rotenc, uint8_t new_state);
void phi_rotenc_process(phi_rotenc_t * rotencs, uint8_t n_rotencs, phi_rotenc_callback_f callback);

#endif /* PHI_LIB_PHI_ROTENC_H_ */
