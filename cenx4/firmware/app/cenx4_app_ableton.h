#ifndef CENX4_APP_ABLETON_H_
#define CENX4_APP_ABLETON_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"
#include "cenx4_can.h"
#include "cenx4_app_cfg.h"

typedef enum
{
    CENX4_APP_ABLETON_SYSEX_INVALID = 0,
    CENX4_APP_ABLETON_SYSEX_SET_POT_VALUE,
    CENX4_APP_ABLETON_SYSEX_SET_POT_TEXT,
	CENX4_APP_ABLETON_SYSEX_RESYNC,
} cenx4_app_ableton_sysex_cmd_t;

#pragma pack(1)

typedef struct
{
    uint8_t pot;
    uint8_t val;
} cenx4_app_ableton_sysex_set_pot_value_t;

typedef struct
{
    uint8_t pot;
    char text_top[CENX4_UI_MAX_LINE_TEXT_LEN];
    char text_bottom[CENX4_UI_MAX_LINE_TEXT_LEN];
} cenx4_app_ableton_sysex_set_pot_text_t;

typedef struct
{
	uint8_t num_modules;
} cenx4_app_ableton_sysex_resync_t;

#pragma pack()

extern const phi_app_desc_t cenx4_app_ableton_desc;

typedef struct cenx4_app_ableton_context_s
{
    // Number of modules available
    uint8_t num_modules;

    // Map of node id -> module number
    uint8_t node_id_to_mod_num[CENX4_APP_CFG_MAX_MODULES];

    // Reverse map (module number -> node id)
    uint8_t mod_num_to_node_id[CENX4_APP_CFG_MAX_MODULES];

    // Map of pots from ableton side to hardware side
    const uint8_t * ableton_to_hw_pot_map;
    const uint8_t * hw_to_ableton_pot_map;

} cenx4_app_ableton_context_t;

void cenx4_app_ableton_start(void * ctx);
void cenx4_app_ableton_stop(void * ctx);
void cenx4_app_ableton_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_ableton_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void cenx4_app_ableton_midi_sysex(void * ctx, phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);

msg_t cenx4_app_ableton_berry_enter_ableton_mode(cenx4_app_ableton_context_t * ctx, uint8_t node_id);
msg_t cenx4_app_ableton_berry_update_ui(cenx4_app_ableton_context_t * ctx, uint8_t node_id);

void cenx4_app_ableton_midi_sysex_set_pot_value(cenx4_app_ableton_context_t * ctx, cenx4_app_ableton_sysex_set_pot_value_t * data);
void cenx4_app_ableton_midi_sysex_set_pot_text(cenx4_app_ableton_context_t * ctx, cenx4_app_ableton_sysex_set_pot_text_t * data);


#endif /* CENX4_APP_ABLETON_H_ */
