#ifndef PHI_APP_MGR_H_
#define PHI_APP_MGR_H_

#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_midi.h"
#include "phi_lib/phi_btn.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* phi_app_start_f)(void * ctx);
typedef void (* phi_app_stop_f)(void * ctx);
typedef void (* phi_app_mgr_tick_f)(void * ctx);
typedef void (* phi_app_encoder_event_f)(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
typedef void (* phi_app_btn_event_f)(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
typedef void (* phi_app_pot_event_f)(void * ctx, uint8_t node_id, uint8_t pot_num, uint8_t val);
typedef void (* phi_app_mgr_midi_cc_f)(void * ctx, phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val);
typedef void (* phi_app_mgr_midi_sysex_f)(void * ctx, phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);

typedef struct phi_app_desc_s
{
    phi_app_start_f start;
    phi_app_start_f stop;
    phi_app_mgr_tick_f tick;
    phi_app_encoder_event_f encoder_event;
    phi_app_btn_event_f btn_event;
    phi_app_pot_event_f pot_event;
    phi_app_mgr_midi_cc_f midi_cc;
    phi_app_mgr_midi_sysex_f midi_sysex;
    systime_t tick_interval;
} phi_app_desc_t;

void phi_app_mgr_init(void);
void phi_app_mgr_switch_app(const phi_app_desc_t * desc, void * ctx);
void phi_app_mgr_notify_encoder_event(uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void phi_app_mgr_notify_btn_event(uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void phi_app_mgr_notify_pot_event(uint8_t node_id, uint8_t pot_num, uint8_t val);
void phi_app_mgr_notify_midi_cc(phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val);
void phi_app_mgr_notify_midi_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* PHI_APP_MGR_H_ */
