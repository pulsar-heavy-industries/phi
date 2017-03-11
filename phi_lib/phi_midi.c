#include "phi_lib/phi_midi.h"

static const phi_midi_cfg_t * phi_midi_cfg = NULL;

typedef struct phi_midi_sysex_state_s
{
    uint8_t buf[PHI_MIDI_SYSEX_MAX_LEN];
    size_t  len;
} phi_midi_sysex_state_t;
static phi_midi_sysex_state_t sysex_state;

void phi_midi_init(const phi_midi_cfg_t * cfg)
{
    chDbgCheck(cfg && !phi_midi_cfg);
    phi_midi_cfg = cfg;

    memset(&sysex_state, 0, sizeof(sysex_state));
}

void phi_midi_rx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
    size_t sysex_bytes_avail;
    int received;

    switch (pkt->type)
    {
    // SysEx Start Or Continue
    case 0x4:
        if ((pkt->b[0] == 0xf0) && (pkt->b[1] == 0x41) && (pkt->b[2] == 0x12))
        {
            sysex_state.len = 0;
        }
        else
        {
            sysex_bytes_avail = sizeof(sysex_state.buf) - sysex_state.len;
            memcpy(&(sysex_state.buf[sysex_state.len]), &(pkt->b[0]), phi_min(sysex_bytes_avail, 3));
            sysex_state.len += phi_min(sysex_bytes_avail, 3);
        }
        break;

    // SysEx ends
    case 0x5:
    case 0x6:
    case 0x7:
        received = pkt->type - 4;
        if (pkt->b[received - 1] == 0xf7)
        {
            --received;
            sysex_bytes_avail = sizeof(sysex_state.buf) - sysex_state.len;
            memcpy(&(sysex_state.buf[sysex_state.len]), &(pkt->b[0]), phi_min(sysex_bytes_avail, received));
            sysex_state.len += phi_min(sysex_bytes_avail, received);

            int w = 0;
            size_t rd = 0;
            while (rd < sysex_state.len)
            {
                // read the bits byte
                int bits = sysex_state.buf[rd];
                rd++;

                // see how many bytes we will be processing in this chunk
                int chunk = phi_min(7, sysex_state.len - rd);

                for (int i = 0; i < chunk; ++i)
                {
                    sysex_state.buf[w] = sysex_state.buf[rd] | ( (bits & (1 << i)) ? 0x80 : 0 );
                    rd++;
                    w++;
                }
            }
            sysex_state.len = w;

            if (sysex_state.len >= 3) // 2 bytes crc + at least 1 byte data
            {
                uint16_t crc1 = sysex_state.buf[0] | (sysex_state.buf[1] << 8);
                uint16_t crc2 = phi_crc16(&(sysex_state.buf[2]), sysex_state.len - 2);
                sysex_state.len -= 2; // 2 bytes for CRC
                if (crc1 == crc2)
                {
                    phi_midi_rx_handle_sysex(port, sysex_state.buf[2], &(sysex_state.buf[3]), sysex_state.len - 1);
                }
            }
        }

        sysex_state.len = 0;

        break;

    default:
        phi_midi_cfg->in_handler(port, pkt);
        break;
    }
}

// TODO MDU1 shouldnt be used here!
#include "usbcfg.h"

void phi_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
    switch (port)
        {
        case PHI_MIDI_PORT_USB:
            phi_usb_midi_send3(&MDU1, 1, pkt->chn_event, pkt->val1, pkt->val2); // TODO 1? need to come from pkt
            break;

        default:
            chDbgCheck(FALSE);
            break;
        }
}

void phi_midi_tx_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    uint8_t buf[PHI_MIDI_SYSEX_MAX_LEN];
    uint16_t crc = phi_crc16_incremental(phi_crc16(&cmd, 1), data, data_len);
    size_t buf_len, i, w;

    buf[0] = 0; // Bits byte
    buf[1] = crc & 0xFF;
    buf[2] = (crc >> 8) & 0xFF;
    buf[3] = cmd;

    w = 4;

    for (i = 0; i < data_len; ++i)
    {
        chDbgAssert(w < sizeof(buf), "WTF");
        buf[w] = ((const uint8_t *)data)[i];
        w++;

        if ((w % 8) == 0)
        {
            // Leave room for a bits byte
            w++;
        }
    }

    buf_len = w;

    // Fill in bits bytes
    for (w = 0; w < buf_len; )
    {
    	size_t chunk_size = phi_min(7, buf_len - w);
        buf[w] = 0;
        for (i = 0; i < chunk_size; ++i)
        {
            if (buf[w + i + 1] & 0x80)
            {
                buf[w] |= 1 << i;
                buf[w + i + 1] &= ~0x80;
            }
        }
        w += 8;
    }

    switch (port)
    {
    case PHI_MIDI_PORT_USB:
        phi_usb_midi_send_sysex(&MDU1, 1, buf, buf_len); // TODO should go through a callback in phi_midi_cfg
        break;

    default:
        chDbgCheck(FALSE);
        break;
    }
}

void phi_midi_rx_handle_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    switch (cmd)
    {
    case PHI_MIDI_SYSEX_CMD_ECHO:
        phi_midi_tx_sysex(port, cmd, data, data_len);
        break;

    case PHI_MIDI_SYSEX_CMD_DEV_INFO:
        {
            phi_midi_sysex_dev_info_t dev_info;
            memset(&dev_info, 0, sizeof(dev_info));
            phi_midi_cfg->get_dev_info(&dev_info);
            phi_midi_tx_sysex(port, cmd, (const uint8_t *) &dev_info, sizeof(dev_info));
        }
        break;

    default:
        phi_midi_cfg->in_sysex(port, cmd, data, data_len);
        break;
    }
}
