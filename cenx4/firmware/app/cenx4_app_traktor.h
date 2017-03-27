#ifndef CENX4_APP_TRAKTOR_H_
#define CENX4_APP_TRAKTOR_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"
#include "cenx4_can.h"
#include "cenx4_app_cfg.h"

#define CENX4_APP_TRAKTOR_MIDI_CH_MASTER	0

#define CENX4_APP_TRAKTOR_MIDI_CC_MASTER_VU_LEFT	1
#define CENX4_APP_TRAKTOR_MIDI_CC_MASTER_VU_RIGHT   2
#define CENX4_APP_TRAKTOR_MIDI_CC_MASTER_LEVEL		3
#define CENX4_APP_TRAKTOR_MIDI_CC_MONITOR_LEVEL		4

extern const phi_app_desc_t cenx4_app_traktor_desc;

typedef struct cenx4_app_traktor_context_s
{
	uint8_t master_l, master_r;
	uint8_t master_level, monitor_level;
} cenx4_app_traktor_context_t;


void cenx4_app_traktor_start(void * ctx);
void cenx4_app_traktor_stop(void * ctx);
void cenx4_app_traktor_tick(void * ctx);
void cenx4_app_traktor_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_traktor_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void cenx4_app_traktor_midi_cc(void * ctx, phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val);
void cenx4_app_traktor_render_custom(cenx4_ui_t * ui, void * _ctx);

#endif /* CENX4_APP_TRAKTOR_H_ */
