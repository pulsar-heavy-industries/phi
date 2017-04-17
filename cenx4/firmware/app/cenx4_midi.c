#include "phi_lib/phi_midi_consts.h"
#include "phi_lib/phi_app_mgr.h"

#include "cenx4_conf.h"
#include "cenx4_midi.h"
#include "usbcfg.h"


const phi_midi_cfg_t cenx4_midi_cfg = {
    .in_handler   = cenx4_midi_in_handler,
    .in_sysex     = cenx4_midi_in_sysex,
    .get_dev_info = cenx4_midi_get_dev_info,
	.tx_pkt       = cenx4_midi_tx_pkt,
	.tx_sysex     = cenx4_midi_tx_sysex,
};

static THD_WORKING_AREA(midi_thread_wa, 512 + PHI_MIDI_SYSEX_MAX_LEN);
/*__attribute__((noreturn)) */static THD_FUNCTION(midi_thread, arg)
{
	(void)arg;

	chRegSetThreadName("midi");

	uint8_t r[4];
	while (1)
	{
		if (!chnReadTimeout(&MDU1, &r[0], 4, TIME_INFINITE)) {
			// USB not active, yuck
			chThdSleepMilliseconds(10);
			continue;
		}

		phi_midi_rx_pkt(PHI_MIDI_PORT_USB, (const phi_midi_pkt_t *) r);
	}
}

void cenx4_midi_init(void)
{
    phi_midi_init(&cenx4_midi_cfg);

    mduObjectInit(&MDU1);
    mduStart(&MDU1, &midiusbcfg);

    chThdCreateStatic(midi_thread_wa, sizeof(midi_thread_wa), NORMALPRIO + 1, midi_thread, NULL);
}

void cenx4_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	(void) port;

	// broken? midi_usb_MidiSend3(1, pkt->b[0], pkt->b[1], pkt->b[2]);

	switch (pkt->event)
	{
	case 0x09: //MIDI_NOTE_ON:
		// phi_daw_midi_note_on(PHI_MIDI_PORT_USB, pkt->chn, pkt->val1, pkt->val2);
		break;

	case 0x08:// MIDI_NOTE_OFF:
		// phi_daw_midi_note_off(PHI_MIDI_PORT_USB, pkt->chn, pkt->val1, pkt->val2);
		break;

	case 0x0B: //MIDI_CONTROL_CHANGE:
		phi_app_mgr_notify_midi_cc(PHI_MIDI_PORT_USB, pkt->chn, pkt->val1, pkt->val2);
		break;
	}
}

void cenx4_midi_in_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    (void) port;
    (void) cmd;
    (void) data;
    (void) data_len;

    switch (cmd)
    {
    case CENX4_MIDI_SYSEX_START_BOOTLOADER:
        {
            const WDGConfig wdgcfg = {
                STM32_IWDG_PR_64,
                STM32_IWDG_RL(1),
                STM32_IWDG_WIN_DISABLED
            };

            wdgStart(&WDGD1, &wdgcfg);
        }
        break;

    default:
        phi_app_mgr_notify_midi_sysex(port, cmd, data, data_len);
        break;
    }
}

void cenx4_midi_get_dev_info(phi_midi_sysex_dev_info_t * dev_info)
{
    dev_info->dev_id = CENX4_DEV_ID;
    dev_info->hw_sw_ver = CENX4_HW_SW_VER;
    memcpy(&(dev_info->uid[0]), &(dev_info->dev_id), sizeof(uint32_t));
    memcpy(&(dev_info->uid[4]), (void *) STM32_REG_UNIQUE_ID, 12);
}

void cenx4_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	if  (port & PHI_MIDI_PORT_USB)
	{
		phi_usb_midi_send3(&MDU1, 1, pkt->chn_event, pkt->val1, pkt->val2);
	}
}

void cenx4_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len)
{
	if  (port & PHI_MIDI_PORT_USB)
	{
        phi_usb_midi_send_sysex(&MDU1, 1, data, len);
    }
}
