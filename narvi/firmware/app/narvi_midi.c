#include "phi_lib/phi_midi_consts.h"
#include "phi_lib/phi_app_mgr.h"

#include "narvi_midi.h"
#include "usbcfg.h"


const phi_midi_cfg_t narvi_midi_cfg = {
    .in_handler            = narvi_midi_in_handler,
    .in_sysex              = narvi_midi_in_sysex,
    .get_dev_info          = narvi_midi_get_dev_info,
	.tx_pkt                = narvi_midi_tx_pkt,
	.tx_sysex              = narvi_midi_tx_sysex,
	.builtin_cmd_port_mask = PHI_MIDI_PORT_USB1,
};

static THD_WORKING_AREA(midi_thread_wa, 512 + PHI_MIDI_SYSEX_MAX_LEN);
/*__attribute__((noreturn)) */static THD_FUNCTION(midi_thread, arg)
{
	(void)arg;

	chRegSetThreadName("midi");

	phi_midi_pkt_t pkt;
	chDbgCheck(sizeof(pkt) == 4);

	while (1)
	{
		if (!chnReadTimeout(&MDU1, &pkt, 4, TIME_INFINITE)) {
			// USB not active, yuck
			chThdSleepMilliseconds(10);
			continue;
		}

		switch (pkt.cable)
		{
		case 0:
			phi_midi_rx_pkt(PHI_MIDI_PORT_USB1, &pkt);
			break;

		case 1:
			phi_midi_rx_pkt(PHI_MIDI_PORT_USB2, &pkt);
			break;

		default:
			chDbgCheck(FALSE);
			break;
		}
	}
}

void narvi_midi_init(void)
{
    phi_midi_init(&narvi_midi_cfg);

    mduObjectInit(&MDU1);
    mduStart(&MDU1, &midiusbcfg);

    chThdCreateStatic(midi_thread_wa, sizeof(midi_thread_wa), NORMALPRIO + 1, midi_thread, NULL);
}

void narvi_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	(void) port;

	// Based on table from http://www.usb.org/developers/docs/devclass_docs/midi10.pdf page 16
	uint8_t pkt_sizes[] = {
		3, 3, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1,
	};

	switch (port)
	{
		// Forward packets on the external port
		case PHI_MIDI_PORT_USB2:
			sdWrite(&SD6, (uint8_t *) pkt->b, pkt_sizes[pkt->type]);
			break;

		default:
			break;
	}
}

void narvi_midi_in_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    (void) port;
    (void) cmd;
    (void) data;
    (void) data_len;

    switch (cmd)
    {
    case NARVI_MIDI_SYSEX_START_BOOTLOADER:
        {
            const WDGConfig wdgcfg = {
                STM32_IWDG_PR_64,
                STM32_IWDG_RL(1),
            };

            wdgStart(&WDGD1, &wdgcfg);
        }
        break;

    default:
//        phi_app_mgr_notify_midi_sysex(port, cmd, data, data_len);
        break;
    }
}

void narvi_midi_get_dev_info(phi_midi_sysex_dev_info_t * dev_info)
{
    dev_info->dev_id = NARVI_DEV_ID;
    dev_info->hw_sw_ver = NARVI_HW_SW_VER;
    memcpy(&(dev_info->uid[0]), &(dev_info->dev_id), sizeof(uint32_t));
    memcpy(&(dev_info->uid[4]), (void *) STM32_REG_UNIQUE_ID, 12);
}

void narvi_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	if  (port & PHI_MIDI_PORT_USB1)	phi_usb_midi_send3(&MDU1, 1, pkt->chn_event, pkt->val1, pkt->val2);
	if  (port & PHI_MIDI_PORT_USB2)	phi_usb_midi_send3(&MDU1, 2, pkt->chn_event, pkt->val1, pkt->val2);
}

void narvi_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len)
{
	if  (port & PHI_MIDI_PORT_USB1) phi_usb_midi_send_sysex(&MDU1, 1, data, len);
	if  (port & PHI_MIDI_PORT_USB2) phi_usb_midi_send_sysex(&MDU1, 2, data, len);
}
