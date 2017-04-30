#ifndef CENX4_MIDI_H_
#define CENX4_MIDI_H_

#include "phi_lib/phi_midi.h"
#include "phi_lib/phi_bl_multiimg.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CENX4_BL_MIDI_SYSEX_CMD_BL_START = PHI_MIDI_SYSEX_CMD_USER,
	CENX4_BL_MIDI_SYSEX_CMD_BL_DATA,
	CENX4_BL_MIDI_SYSEX_CMD_BL_DONE,
	CENX4_BL_MIDI_SYSEX_CMD_BL_SERFLASH_START,
	CENX4_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DATA,
	CENX4_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DONE,
	CENX4_BL_MIDI_SYSEX_CMD_BL_UPDATE_SELF_FROM_SERFLASH,
};

void cenx4_midi_init(void);
void cenx4_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void cenx4_midi_in_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);
void cenx4_midi_get_dev_info(phi_midi_sysex_dev_info_t * dev_info);
void cenx4_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void cenx4_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len);

void cenx4_midi_bl_start(phi_midi_port_t port, const void * data, size_t data_len);
void cenx4_midi_bl_data(phi_midi_port_t port, const void * data, size_t data_len);
void cenx4_midi_bl_done(phi_midi_port_t port, const void * data, size_t data_len);

void cenx4_midi_bl_serflash_start(phi_midi_port_t port, const void * data, size_t data_len);
void cenx4_midi_bl_serflash_data(phi_midi_port_t port, const void * data, size_t data_len);
void cenx4_midi_bl_serflash_done(phi_midi_port_t port, const void * data, size_t data_len);
void cenx4_midi_bl_update_self_from_serflash(phi_midi_port_t port, const void * data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* CENX4_MIDI_H_ */
