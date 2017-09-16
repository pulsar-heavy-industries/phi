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

static THD_WORKING_AREA(usb_midi_thread_wa, 512 + PHI_MIDI_SYSEX_MAX_LEN);
/*__attribute__((noreturn)) */static THD_FUNCTION(usb_midi_thread, arg)
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

static THD_WORKING_AREA(serial_midi_thread_wa, 512 + PHI_MIDI_SYSEX_MAX_LEN);
/*__attribute__((noreturn)) */static THD_FUNCTION(serial_midi_thread, arg)
{
	// Based on https://github.com/axoloti/axoloti/blob/master/firmware/serial_midi.c
	const int8_t status_len[16] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		3, // 0x80=note off, 3 bytes
	    3, // 0x90=note on, 3 bytes
	    3, // 0xa0=poly pressure, 3 bytes
	    3, // 0xb0=control change, 3 bytes
	    2, // 0xc0=program change, 2 bytes
	    2, // 0xd0=channel pressure, 2 bytes
	    3, // 0xe0=pitch bend, 3 bytes
	    -1 // 0xf0=other things. may vary.
	};
	const int8_t sys_len[16] = {
		-1, // 0xf0=sysex start. may vary
	    2, // 0xf1=MIDI Time Code. 2 bytes
	    3, // 0xf2=MIDI Song position. 3 bytes
	    2, // 0xf3=MIDI Song Select. 2 bytes.
	    1, // 0xf4=undefined
	    1, // 0xf5=undefined
	    1, // 0xf6=TUNE Request
	    1, // 0xf7=sysex end.
	    1, // 0xf8=timing clock. 1 byte
	    1, // 0xf9=proposed measure end?
	    1, // 0xfa=start. 1 byte
	    1, // 0xfb=continue. 1 byte
	    1, // 0xfc=stop. 1 byte
	    1, // 0xfd=undefined
	    1, // 0xfe=active sensing. 1 byte
	    3 // 0xff= not reset, but a META-EVENT, which is always 3 bytes
	    };

	(void)arg;
	chRegSetThreadName("midi");

	uint8_t data;
	int8_t len;
	unsigned char extra_bytes_received = 0xff;
	unsigned char extra_bytes_needed = 0xff;
	phi_midi_pkt_t pkt;
	chDbgCheck(sizeof(pkt) == 4);

	while (1)
	{
		data = sdGet(&SD6);

		// Status byte
		if (data & 0x80) {
			len = status_len[data >> 4];
			if (len == -1) {
				len = sys_len[data - 0xF0];
				if (len == 1) {
		        	pkt.type = 0xFF; // TODO! - this probably breaks sysex fwding
		        	pkt.b[0] = data;
		        	pkt.b[1] = 0;
		        	pkt.b[2] = 0;
		        	phi_midi_rx_pkt(PHI_MIDI_PORT_SERIAL1, &pkt);
				}
				else
				{
					pkt.b[0] = data;
					extra_bytes_needed = len - 1;
					extra_bytes_received = 0;
				}
			}
			else
			{
				pkt.b[0] = data;
				extra_bytes_needed = len - 1;
				extra_bytes_received = 0;
			}
		} else { // Not status
			if (extra_bytes_received == 0) {
				pkt.b[1]  = data;
		        if (extra_bytes_needed == 1) {
		        	// 2 byte packet
		        	pkt.type = 0xFF; // TODO! - this probably breaks sysex fwding
		        	pkt.b[2] = 0;
		        	phi_midi_rx_pkt(PHI_MIDI_PORT_SERIAL1, &pkt);
		        	extra_bytes_received = 0;
		        } else {
		        	extra_bytes_received++;
		        }
			} else if (extra_bytes_received == 1) {
				pkt.b[2]  = data;
				if (extra_bytes_needed == 2) {
		        	// 3 byte packet
					pkt.type = 0xFF; // TODO! - this probably breaks sysex fwding
		        	phi_midi_rx_pkt(PHI_MIDI_PORT_SERIAL1, &pkt);

		        	extra_bytes_received = 0;
				}
			}
		}
	}
}
void narvi_midi_init(void)
{
    phi_midi_init(&narvi_midi_cfg);

    mduObjectInit(&MDU1);
    mduStart(&MDU1, &midiusbcfg);

    chThdCreateStatic(usb_midi_thread_wa, sizeof(usb_midi_thread_wa), NORMALPRIO + 1, usb_midi_thread, NULL);
    chThdCreateStatic(serial_midi_thread_wa, sizeof(serial_midi_thread_wa), NORMALPRIO + 2, serial_midi_thread, NULL);
}

void narvi_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	(void) port;

	switch (port)
	{
		// Forward: USB2->EXT
		case PHI_MIDI_PORT_USB2:
			phi_midi_tx_pkt(PHI_MIDI_PORT_SERIAL1, pkt);
			break;

		// Forward: EXT->USB
		case PHI_MIDI_PORT_SERIAL1:
			phi_midi_tx_pkt(PHI_MIDI_PORT_USB2, pkt);
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
	// Based on table from http://www.usb.org/developers/docs/devclass_docs/midi10.pdf page 16
	uint8_t pkt_sizes[] = {
		3, 3, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1,
	};

	if (port & PHI_MIDI_PORT_USB1) phi_usb_midi_send3(&MDU1, 1, pkt->chn_event, pkt->val1, pkt->val2);
	if (port & PHI_MIDI_PORT_USB2) phi_usb_midi_send3(&MDU1, 2, pkt->chn_event, pkt->val1, pkt->val2);
	if (port & PHI_MIDI_PORT_SERIAL1) sdWrite(&SD6, (uint8_t *) pkt->b, pkt_sizes[pkt->type]);
}

void narvi_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len)
{
	if (port & PHI_MIDI_PORT_USB1) phi_usb_midi_send_sysex(&MDU1, 1, data, len);
	if (port & PHI_MIDI_PORT_USB2) phi_usb_midi_send_sysex(&MDU1, 2, data, len);
}
