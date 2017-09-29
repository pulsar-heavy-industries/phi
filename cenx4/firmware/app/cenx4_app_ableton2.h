#ifndef CENX4_APP_ABLETON2_H_
#define CENX4_APP_ABLETON2_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"
#include "cenx4_can.h"
#include "cenx4_app_cfg.h"
#include "../../../hyperion/firmware/app/hyperion_app_slave_can.h"

#define CENX4_APP_ABLETON2_MIDI_CH_MASTER	0


#pragma pack(1)

typedef struct {
	uint8_t mod_num;
	char title[HYPERION_UI_MAX_LINE_TEXT_LEN];
} cenx4_app_ableton2_sysex_set_title_t;


typedef enum
{
	CENX4_APP_ABLETON2_MODE_DEFAULT = 0,
} cenx4_app_ableton2_mode_t;

extern const phi_app_desc_t cenx4_app_ableton2_desc;

typedef struct cenx4_app_ableton2_context_s
{
	cenx4_app_ableton2_mode_t mode;

    // Number of modules available
    uint8_t num_modules;

    // Map of node id -> module number
    uint8_t node_id_to_mod_num[CENX4_APP_CFG_MAX_MODULES];

    // Reverse map (module number -> node id)
    uint8_t mod_num_to_node_id[CENX4_APP_CFG_MAX_MODULES];

    // Hyperion modules state
    struct {
    	char title[HYPERION_UI_MAX_LINE_TEXT_LEN];
    	uint8_t buttons_state[8];
    } hyperions[CENX4_APP_CFG_MAX_MODULES];

} cenx4_app_ableton2_context_t;

/**********************************************************************************************/


void cenx4_app_ableton2_start(void * ctx);
void cenx4_app_ableton2_stop(void * ctx);
void cenx4_app_ableton2_tick(void * ctx);
void cenx4_app_ableton2_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_ableton2_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void cenx4_app_ableton2_pot_event(void * ctx, uint8_t node_id, uint8_t pot_num, uint8_t val);
void cenx4_app_ableton2_midi_cc(void * ctx, phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val);
void cenx4_app_ableton2_midi_sysex(void * ctx, phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);
void cenx4_app_ableton2_reconfigure_displays(cenx4_app_ableton2_context_t * ctx);
msg_t cenx4_app_ableton2_update_slave_display(cenx4_app_ableton2_context_t * ctx, uint8_t node_id);

void cenx4_app_ableton2_midi_sysex_set_title(cenx4_app_ableton2_context_t * ctx, cenx4_app_ableton2_sysex_set_title_t * data);

#endif /* CENX4_APP_ABLETON2_H_ */
