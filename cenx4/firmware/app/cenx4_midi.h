#ifndef CENX4_MIDI_H_
#define CENX4_MIDI_H_

#include "phi_lib/phi_midi.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CENX4_MIDI_SYSEX_START_BOOTLOADER = PHI_MIDI_SYSEX_CMD_USER, // 32
	CENX4_MIDI_SYSEX_RESERVED33,
	CENX4_MIDI_SYSEX_RESERVED34,
	CENX4_MIDI_SYSEX_RESERVED35,
	CENX4_MIDI_SYSEX_RESERVED36,
	CENX4_MIDI_SYSEX_RESERVED37,
	CENX4_MIDI_SYSEX_RESERVED38,
	CENX4_MIDI_SYSEX_RESERVED39,
	CENX4_MIDI_SYSEX_RESERVED40,


	// ableton app commands
    CENX4_MIDI_SYSEX_APP_ABLETON_SET_POT_VALUE, // 41
    CENX4_MIDI_SYSEX_APP_ABLETON_SET_POT_TEXT,  // 42
	CENX4_MIDI_SYSEX_APP_ABLETON_SET_POT_ALL,   // 43
	CENX4_MIDI_SYSEX_APP_ABLETON_RESYNC,        // 44

	// ableton2 app commands
	CENX4_MIDI_SYSEX_APP_ABLETON2_SET_TITLE,	// 41
};

void cenx4_midi_init(void);
void cenx4_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void cenx4_midi_in_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);
void cenx4_midi_get_dev_info(phi_midi_sysex_dev_info_t * dev_info);
void cenx4_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void cenx4_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CENX4_MIDI_H_ */
