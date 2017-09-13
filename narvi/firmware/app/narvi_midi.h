#ifndef NARVI_MIDI_H_
#define NARVI_MIDI_H_

#include "phi_lib/phi_midi.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    NARVI_MIDI_SYSEX_START_BOOTLOADER = PHI_MIDI_SYSEX_CMD_USER, // 32
	NARVI_MIDI_SYSEX_RESERVED33,
	NARVI_MIDI_SYSEX_RESERVED34,
	NARVI_MIDI_SYSEX_RESERVED35,
	NARVI_MIDI_SYSEX_RESERVED36,
	NARVI_MIDI_SYSEX_RESERVED37,
	NARVI_MIDI_SYSEX_RESERVED38,
	NARVI_MIDI_SYSEX_RESERVED39,
	NARVI_MIDI_SYSEX_RESERVED40,


	// ableton app commands
    NARVI_MIDI_SYSEX_APP_ABLETON_SET_POT_VALUE, // 41
    NARVI_MIDI_SYSEX_APP_ABLETON_SET_POT_TEXT,  // 42
	NARVI_MIDI_SYSEX_APP_ABLETON_SET_POT_ALL,   // 43
	NARVI_MIDI_SYSEX_APP_ABLETON_RESYNC,        // 44
};

void narvi_midi_init(void);
void narvi_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void narvi_midi_in_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);
void narvi_midi_get_dev_info(phi_midi_sysex_dev_info_t * dev_info);
void narvi_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void narvi_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* NARVI_MIDI_H_ */
