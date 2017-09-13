#ifndef PHI_LIB_PHI_MIDI_H_
#define PHI_LIB_PHI_MIDI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "phi_lib/phi_lib.h"

#if HAL_USE_USB
#include "phi_lib/phi_usb_midi.h"
#endif


#define PHI_MIDI_SYSEX_MAX_LEN   295  // (256 + 2) * 8 / 7 // (256 data + 2 crc)

typedef enum phi_midi_port_e
{
    PHI_MIDI_PORT_INVALID  = 0,

	PHI_MIDI_PORT_USB1     = (1 << 0),
	PHI_MIDI_PORT_USB2     = (1 << 1),
	PHI_MIDI_PORT_USB3     = (1 << 2),
	PHI_MIDI_PORT_USB4     = (1 << 3),

    PHI_MIDI_PORT_INTERNAL1 = (1 << 4),
	PHI_MIDI_PORT_INTERNAL2 = (1 << 5),
	PHI_MIDI_PORT_INTERNAL3 = (1 << 6),
	PHI_MIDI_PORT_INTERNAL4 = (1 << 7),

    PHI_MIDI_PORT_SERIAL1  = (1 << 7),
	PHI_MIDI_PORT_SERIAL2  = (1 << 8),
	PHI_MIDI_PORT_SERIAL3  = (1 << 9),
	PHI_MIDI_PORT_SERIAL4  = (1 << 10),

	PHI_MIDI_PORT_CAN      = (1 << 11),

    PHI_MIDI_PORT_ALL      = 0xFFFFFFFF,

} phi_midi_port_t;

typedef union phi_midi_pkt_s
{
    struct
    {
        uint8_t cin_cable;
        uint8_t b[3];
    };

    struct {
        union
        {
            struct {
                uint8_t type:4;
                uint8_t cable:4;
            };
            uint8_t type_cable;
        };

        union
        {
            struct
            {
                uint8_t chn:4;
                uint8_t event:4;
            };
            uint8_t chn_event;
        };

        uint8_t val1;
        uint8_t val2;
    };
} phi_midi_pkt_t;


typedef enum phi_midi_sysex_cmd_e
{
    PHI_MIDI_SYSEX_CMD_INVALID = 0,
    PHI_MIDI_SYSEX_CMD_ECHO,
    PHI_MIDI_SYSEX_CMD_DEV_INFO,
    PHI_MIDI_SYSEX_CMD_USER = 32,
} phi_midi_sysex_cmd_t;

typedef struct phi_midi_sysex_dev_info_s
{
    uint32_t dev_id;
    uint32_t hw_sw_ver;
    uint8_t uid[16];
    uint32_t reserved[4];
} phi_midi_sysex_dev_info_t;

typedef void (* phi_midi_in_handler_f)(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
typedef void (* phi_midi_in_sysex_f)(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);
typedef void (* phi_midi_get_dev_info_f)(phi_midi_sysex_dev_info_t * dev_info);
typedef void (* phi_midi_tx_pkt_f)(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
typedef void (* phi_midi_tx_sysex_f)(phi_midi_port_t port, const uint8_t * data, size_t len);

typedef struct phi_midi_cfg_s
{
    phi_midi_in_handler_f   in_handler;
    phi_midi_in_sysex_f     in_sysex;
    phi_midi_get_dev_info_f get_dev_info;
    phi_midi_tx_pkt_f       tx_pkt;
    phi_midi_tx_sysex_f     tx_sysex;
} phi_midi_cfg_t;

void phi_midi_init(const phi_midi_cfg_t * cfg);
void phi_midi_rx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void phi_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt);
void phi_midi_tx_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);

void phi_midi_rx_handle_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif
