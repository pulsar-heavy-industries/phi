#ifndef CENX4_APP_SETUP_H_
#define CENX4_APP_SETUP_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"

extern const phi_app_desc_t cenx4_app_setup_desc;

#define cenx4_app_setup_node_id_to_idx(node_id) (node_id ? (node_id - PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + 1) : 0)

typedef enum {
	CENX4_APP_SETUP_ACTION_EXIT = 0,
	CENX4_APP_SETUP_ACTION_SAVE,
	CENX4_APP_SETUP_ACTION_TEST,
	CENX4_APP_SETUP_ACTION_BOOTLOADER,
	CENX4_APP_SETUP_NUM_ACTIONS,
} cenx4_app_setup_action_t;

typedef struct cenx4_app_setup_context_s
{
	// Currently selected action
	cenx4_app_setup_action_t cur_action;

	// Value of test pot for each of the modules
	uint8_t test_pot_val[PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1]; // Extra device for our state

    // Current map of node id -> module number
    uint8_t node_id_to_mod_num[PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1];

    // Is configuration in a saveable state?
    bool cfg_ok;

} cenx4_app_setup_context_t;

void cenx4_app_setup_start(void * ctx);
void cenx4_app_setup_stop(void * ctx);
void cenx4_app_setup_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_setup_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);

msg_t cenx4_app_setup_berry_enter_setup_mode(cenx4_app_setup_context_t * ctx, uint8_t node_id);
msg_t cenx4_app_setup_berry_update_ui(cenx4_app_setup_context_t * ctx, uint8_t node_id);

#endif /* CENX4_APP_SETUP_H_ */
