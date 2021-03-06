#ifndef CENX4_CAN_H_
#define CENX4_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_btn.h"
#include "cenx4_ui.h"

enum {
	// MASTER->SLAVE commands
	PHI_CAN_MSG_ID_CENX4_UPDATE_DISPLAY = PHI_CAN_MSG_ID_USER + 1,
	PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_VAL,
	PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_TEXT,
	PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT,
	PHI_CAN_MSG_ID_CENX4_SET_ROTENC_SPEED,
};

#pragma pack (1)

typedef struct {
	uint8_t disp;
	cenx4_ui_dispmode_t dispmode;
	cenx4_ui_dispmode_state_t state;
} cenx4_can_handle_update_display_state_t;

typedef struct {
    uint8_t disp;
    uint8_t pot;
    uint8_t val;
} cenx4_can_handle_set_split_pot_val_t;

typedef struct {
    uint8_t disp;
    uint8_t pot;
    char text_top[CENX4_UI_MAX_LINE_TEXT_LEN];
    char text_bottom[CENX4_UI_MAX_LINE_TEXT_LEN];
} cenx4_can_handle_set_split_pot_text_t;

typedef struct {
    uint8_t disp;
    uint8_t pot;
    uint8_t val;
    char text_top[CENX4_UI_MAX_LINE_TEXT_LEN];
    char text_bottom[CENX4_UI_MAX_LINE_TEXT_LEN];
} cenx4_can_handle_set_split_pot_t;

typedef struct {
	uint8_t rotenc;
	uint8_t speed;
} cenx4_can_handle_set_rotenc_speed_t;

#pragma pack ()

///

extern phi_can_t cenx4_can;

void cenx4_can_init(void);

void cenx4_can_handle_reset(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void cenx4_can_handle_start_bootloader(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void cenx4_can_handle_encoder_event(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void cenx4_can_handle_btn_event(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void cenx4_can_handle_pot_event(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void cenx4_can_handle_set_rotenc_speed(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void cenx4_can_handle_unknown_cmd(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);

msg_t cenx4_send_set_rotenc_speed(uint8_t node_id,  uint8_t rotenc, uint8_t speed);

#ifdef __cplusplus
}
#endif

#endif /* CENX4_CAN_H_ */
