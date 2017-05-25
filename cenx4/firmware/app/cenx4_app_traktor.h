#ifndef CENX4_APP_TRAKTOR_H_
#define CENX4_APP_TRAKTOR_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"
#include "cenx4_can.h"
#include "cenx4_app_cfg.h"

#define CENX4_APP_TRAKTOR_MIDI_CH_MASTER	0

#define CENX4_APP_TRAKTOR_MIDI_CC_MASTER_VU_LEFT			1
#define CENX4_APP_TRAKTOR_MIDI_CC_MASTER_VU_RIGHT   		2
#define CENX4_APP_TRAKTOR_MIDI_CC_MASTER_LEVEL				3
#define CENX4_APP_TRAKTOR_MIDI_CC_MONITOR_LEVEL				4
#define CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_MODE				5
#define CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_SELECT				6
#define CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_SELECT_PAGE		7
#define CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_TREE_SELECT		8
#define CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_TREE_EXPAND		9
#define CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_PREVIEW_TOGGLE		10
#define CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_PREVIEW_SEEK       11
#define CENX4_APP_TRAKTOR_MIDI_CC_SHIFT1					12

#define CENX4_APP_TRAKTOR_MIDI_CC_UNUSED			63

typedef enum
{
	CENX4_APP_TRAKTOR_MODE_DEFAULT = 0,
	CENX4_APP_TRAKTOR_MODE_BROWSE,
} cenx4_app_traktor_mode_t;

extern const phi_app_desc_t cenx4_app_traktor_desc;

typedef struct cenx4_app_traktor_context_s
{
	cenx4_app_traktor_mode_t mode;
	uint8_t master_l, master_r;
	uint8_t master_level, monitor_level;
	bool_t browse_mode;
	bool_t shift1_mode;

    // Number of modules available
    uint8_t num_modules;

    // Map of node id -> module number
    uint8_t node_id_to_mod_num[CENX4_APP_CFG_MAX_MODULES];

    // Reverse map (module number -> node id)
    uint8_t mod_num_to_node_id[CENX4_APP_CFG_MAX_MODULES];

    // Gains
    uint8_t mod_num_to_gain[CENX4_APP_CFG_MAX_MODULES];

    // Loop lengths
    uint8_t mod_num_to_loop_len[CENX4_APP_CFG_MAX_MODULES];

} cenx4_app_traktor_context_t;


/**********************************************************************************************/

/* interfacing with a hyperion module over CAN */


/**********************************************************************************************/


void cenx4_app_traktor_start(void * ctx);
void cenx4_app_traktor_stop(void * ctx);
void cenx4_app_traktor_tick(void * ctx);
void cenx4_app_traktor_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_traktor_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void cenx4_app_traktor_pot_event(void * ctx, uint8_t node_id, uint8_t pot_num, uint8_t val);
void cenx4_app_traktor_midi_cc(void * ctx, phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val);
void cenx4_app_traktor_reconfigure_displays(cenx4_app_traktor_context_t * ctx);
void cenx4_app_traktor_render_browse_left(cenx4_ui_t * ui, void * _ctx);
void cenx4_app_traktor_render_browse_right(cenx4_ui_t * ui, void * _ctx);
void cenx4_app_traktor_render_vu_meters(cenx4_ui_t * ui, void * _ctx);
msg_t cenx4_app_traktor_update_slave_display(cenx4_app_traktor_context_t * ctx, uint8_t node_id);

#endif /* CENX4_APP_TRAKTOR_H_ */
