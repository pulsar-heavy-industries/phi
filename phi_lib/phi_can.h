#ifndef PHI_LIB_PHI_CAN_H_
#define PHI_LIB_PHI_CAN_H_

#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_btn.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Misc consts
 *****************************************************************************/
#define PHI_CAN_MSG_POOL_SIZE					32
#define PHI_CAN_MAX_XFERS        				4
#define PHI_CAN_MAX_MSG_DATA_LEN    			140 // we have 7 bytes of data per can msg so we want to this to be divisible by 7
#define PHI_CAN_VERSION 						0x100a
#define PHI_CAN_DEFAULT_TIMEOUT 				MS2ST(100)
#define PHI_CAN_PRIO_LOWEST         			0x1F
#define PHI_CAN_PRIO_HIGHEST        			0
#define PHI_CAN_MAX_NODE_ID						0x7F
#define PHI_CAN_PROCESS_RX_THREAD_STACK_SIZE	512
#define PHI_CAN_SANITY_TESTS                    1

/******************************************************************************
 * Auto-id allocation
 *****************************************************************************/
#ifndef PHI_CAN_ENABLE_ALLOCATOR
#define PHI_CAN_ENABLE_ALLOCATOR FALSE
#endif

#define PHI_CAN_AUTO_ID_UNIQ_ID_LEN      16
#define PHI_CAN_AUTO_ID_PRIO             1
#define PHI_CAN_AUTO_ID_OP_PART1         1
#define PHI_CAN_AUTO_ID_OP_PART2         2
#define PHI_CAN_AUTO_ID_OP_PART3         3
#define PHI_CAN_AUTO_ID                  0 // Node id to use for auto-allocating nodes
#define PHI_CAN_AUTO_ID_ALLOCATOR_NODE   1 // Node id of the allocator node

#define PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS       32
#define PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID   10


/******************************************************************************
 * Commands
 *****************************************************************************/
enum {
	PHI_CAN_MSG_ID_SYSINFO = 1,
	PHI_CAN_MSG_ID_ECHO,
	PHI_CAN_MSG_ID_AUTO_ID_ACK,

	// Bootloader commands (implemented inside bootloader)
	PHI_CAN_MSG_BL_START,
	PHI_CAN_MSG_BL_DATA,
	PHI_CAN_MSG_BL_DONE,

	// Up to the app to implement a handler for this
	PHI_CAN_MSG_ID_RESET,
	PHI_CAN_MSG_ID_START_BOOTLOADER,
	PHI_CAN_MSG_ID_IO_ENCODER_EVENT,
	PHI_CAN_MSG_ID_IO_BTN_EVENT,
	PHI_CAN_MSG_ID_IO_POT_EVENT,

	PHI_CAN_MSG_ID_USER = 100,
};

#pragma pack(1)

typedef struct phi_can_msg_data_sysinfo_s {
	uint32_t can_version;
	uint32_t dev_id;
	uint32_t hw_sw_ver;
	uint32_t reserved[4];
} phi_can_msg_data_sysinfo_t;

typedef struct {
    uint8_t encoder_num;
    int8_t val_change;
} phi_can_msg_data_io_encoder_event_t;

typedef struct {
	uint8_t btn_num;
	phi_btn_event_t event;
	uint32_t param;
} phi_can_msg_data_io_btn_event_t;

typedef struct {
    uint8_t pot_num;
    uint8_t val;
} phi_can_msg_data_io_pot_event_t;

#pragma pack()

struct phi_can_s;

typedef struct phi_can_msg_handler_s
{
	uint8_t msg_id;
	void (*handler)(struct phi_can_s * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t xfer_id, const uint8_t * data, size_t len);
	void * context;
} phi_can_msg_handler_t;

#define PHI_CAN_BUILTIN_MSG_HANDLERS \
	{PHI_CAN_MSG_ID_SYSINFO, phi_can_handle_sysinfo, NULL}, \
	{PHI_CAN_MSG_ID_ECHO, phi_can_handle_echo, NULL}


/******************************************************************************
 * Transfer-related data structures
 *****************************************************************************/
#pragma pack(1)
typedef union phi_can_msg_xfer_desc_u {
	uint8_t val;
	struct {
		uint8_t start : 1;
		uint8_t end : 1;
		uint8_t toggle_or_is_reply : 1; // toggle when start=0, is_reply when start=1
		uint8_t chan_id : 5;
	};
} phi_can_msg_xfer_desc_t;

typedef struct phi_can_eid_u
{
	uint8_t prio;
	uint8_t msg_id;
	uint8_t dst;
	uint8_t src;
	uint8_t reserved;
	uint8_t is_reply;
} phi_can_eid_t;

#define EID_UNPACK(val) { \
	.prio     = (val >> 24) & 0x1F, \
	.msg_id   = (val >> 16) & 0xFF, \
	.dst      = (val >> 9)  & 0x7F, \
	.src      = (val >> 2)  & 0x7F, \
	.reserved = (val >> 1)  & 0x01, \
	.is_reply = (val >> 0)  & 0x01, \
}

#define EID_PACK(eid) ((uint32_t)( \
	(eid.prio     & 0x1F) << 24 | \
	(eid.msg_id   & 0xFF) << 16 | \
	(eid.dst      & 0x7F) << 9  | \
	(eid.src      & 0x7F) << 2  | \
	(eid.reserved & 0x01) << 1  | \
	(eid.is_reply & 0x01) << 0   \
))

#pragma pack()

#define PACK_XFER_ID(src, dst, chan_id, is_reply) ( \
	((src      & 0x7F) << 13) | \
	((dst      & 0x7F) << 6)  | \
	((chan_id  & 0x1F) << 1)  | \
	((is_reply & 0x01) << 0) \
)

#define XFER_ID_GET_CHAN(v) ((v >> 1) & 0x1F)

typedef struct phi_can_xfer_s {
	uint32_t xfer_id;
	uint8_t rx_data[PHI_CAN_MAX_MSG_DATA_LEN];
	uint8_t rx_len;

	uint8_t toggle;
	uint16_t expected_crc;
	systime_t last_seen_at;

	binary_semaphore_t rx_done;
} phi_can_xfer_t;

/******************************************************************************
 * CAN
 *****************************************************************************/
typedef struct phi_can_config_s {
	CANDriver * drv;
	const CANConfig * drv_cfg;
	const phi_can_msg_handler_t * handlers;
	const phi_can_msg_handler_t default_handler;
	uint32_t n_handlers;
	uint32_t dev_id;
	uint32_t hw_sw_ver;
} phi_can_config_t;

typedef struct phi_can_s {
	const phi_can_config_t * cfg;
	uint8_t node_id;

	phi_can_xfer_t xfers[PHI_CAN_MAX_XFERS];

	uint8_t next_chan_id;
	mutex_t xfer_lock;
	semaphore_t xfer_sem;

	uint32_t stat_read_errs;
	uint32_t stat_process_rx;
	uint32_t stat_process_rx_err;
	uint32_t stat_process_rx_timeout;
	uint32_t stat_process_rx_dropped_multi_reqs;

	CANRxFrame bufs[PHI_CAN_MSG_POOL_SIZE];
	msg_t free_bufs_q[PHI_CAN_MSG_POOL_SIZE];
	mailbox_t free_bufs;
	msg_t rx_bufs_q[PHI_CAN_MSG_POOL_SIZE];
	mailbox_t rx_bufs;

	thread_t * process_rx_thread;
	THD_WORKING_AREA(process_rx_thread_wa, PHI_CAN_PROCESS_RX_THREAD_STACK_SIZE);

	thread_t * read_thread;
	THD_WORKING_AREA(read_thread_wa, 128);

#if PHI_CAN_ENABLE_ALLOCATOR
	uint8_t auto_parts_received;
	systime_t auto_last_at;
	uint8_t auto_alloc_table[PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS][PHI_CAN_AUTO_ID_UNIQ_ID_LEN];
	uint8_t auto_alloc_num_devs;
	uint8_t auto_alloc_cur_uid[PHI_CAN_AUTO_ID_UNIQ_ID_LEN];
#endif
} phi_can_t;

bool phi_can_init(phi_can_t * can, const phi_can_config_t * cfg, uint8_t node_id);
void phi_can_set_node_id(phi_can_t * can, uint8_t node_id);

/*
 * send small message, expect small reply:
 * 1) sender allocates xfer_id
 * 2) sender sets rx bufs in xfer struct to hold reply data
 * 3) SEND DATA:
 *
 *
 * reciver:
 * - get incoming message
 * - if start bit is set, look for xfer id in xfers table
 *    - found: this is a reply to something that is pending
 *    - not found: this is an incoming message, must receive into internal buf because
 *      no one is waiting for this
 */

msg_t phi_can_xfer(
	phi_can_t * can,
	uint8_t prio,
	uint8_t msg_id,
	uint8_t dst,
	const uint8_t * req_buf,
	size_t req_len,
	uint8_t * resp_buf,
	uint32_t max_resp_len,
	uint32_t * out_resp_len,
	systime_t timeout);


phi_can_xfer_t * phi_can_alloc_xfer(phi_can_t * can, bool lock, bool is_reply, uint8_t node_id, uint8_t chan_id, systime_t timeout);
void phi_can_free_xfer(phi_can_t * can, bool lock, phi_can_xfer_t * xfer);
void phi_can_handle_incoming_msg(phi_can_t * can, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, uint8_t data_len);
msg_t phi_can_send_reply(phi_can_t * can, uint8_t prio, uint8_t msg_id, uint8_t dst, uint8_t chan_id, const uint8_t * data, uint8_t data_len, systime_t timeout);
msg_t phi_can_send_internal(phi_can_t * can, bool is_reply, uint8_t chan_id, uint8_t prio, uint8_t msg_id, uint8_t dst, const uint8_t * data, uint8_t data_len, systime_t timeout);

// message handlers
void phi_can_handle_sysinfo(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void phi_can_handle_echo(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);

// auto id allocation
msg_t phi_can_auto_get_id(phi_can_t * can);
void phi_can_auto_get_dev_uid(phi_can_t * can, uint8_t * uid);
void phi_can_auto_handle_incoming_msg(phi_can_t * can, uint8_t prio, uint16_t msg_id, uint8_t dsr, uint8_t chan_id, const uint8_t * data);

#ifdef __cplusplus
}
#endif

#endif /* PHI_LIB_PHI_CAN_H_ */
