#include "phi_lib/phi_app_mgr.h"

#include "cenx4_can.h"
#include "cenx4_conf.h"
#include "cenx4_ui.h"
#include "cenx4_io.h"


static const CANConfig cancfg_500k = {
	.mcr =
		CAN_MCR_TXFP | // Transmit FIFO priority -  Priority driven by the request order (chronologically)
		CAN_MCR_ABOM, //  Automatic bus-off management
	.btr =
		CAN_BTR_SJW(0) |
        CAN_BTR_TS1(12) | CAN_BTR_TS2(1) | CAN_BTR_BRP(2), // 750k @ 36MHz

};


static const phi_can_msg_handler_t can_handlers[] = {
	// Built-in stuff
	PHI_CAN_BUILTIN_MSG_HANDLERS,
	{PHI_CAN_MSG_ID_RESET, cenx4_can_handle_reset, NULL},
	{PHI_CAN_MSG_ID_START_BOOTLOADER, cenx4_can_handle_start_bootloader, NULL},

	// SLAVE->MASTER commands that are not app-specific
	{PHI_CAN_MSG_ID_IO_ENCODER_EVENT, cenx4_can_handle_encoder_event, NULL},
	{PHI_CAN_MSG_ID_IO_BTN_EVENT, cenx4_can_handle_btn_event, NULL},
	{PHI_CAN_MSG_ID_IO_POT_EVENT, cenx4_can_handle_pot_event, NULL},

	// MASTER->SLAVE commands that are not app-specific
	{PHI_CAN_MSG_ID_CENX4_SET_ROTENC_SPEED, cenx4_can_handle_set_rotenc_speed, NULL},
};

static const phi_can_config_t can1_cfg = {
	.drv = &CAND1,
	.drv_cfg = &cancfg_500k,
	.handlers = can_handlers,
	.default_handler = {
		.handler = cenx4_can_handle_unknown_cmd,
	},
	.n_handlers = sizeof(can_handlers) / sizeof(can_handlers[0]),
	.dev_id = CENX4_DEV_ID,
	.hw_sw_ver = CENX4_HW_SW_VER,
};



phi_can_t cenx4_can;


volatile unsigned int loops=0;
//volatile unsigned int can_errs=0;


void cenx4_can_init(void)
{
	if (cenx4_is_master)
	{
		phi_can_init(&cenx4_can, &can1_cfg, PHI_CAN_AUTO_ID_ALLOCATOR_NODE);

		palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(9));
		palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(9) | PAL_STM32_OSPEED_HIGHEST);
	}
	else
	{
		phi_can_init(&cenx4_can, &can1_cfg, PHI_CAN_AUTO_ID);

		palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(9));
		palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(9) | PAL_STM32_OSPEED_HIGHEST);

	   {
		   int tries = 0;
		   msg_t ret;
		   cenx4_ui_t * ui;

		   for (;;)
		   {
			   ++tries;
			   ui = cenx4_ui_lock(0);
			   chsnprintf(ui->state.boot.misc_text, sizeof(ui->state.boot.misc_text)-1, "AutoId #%d", tries);
			   cenx4_ui_unlock(ui);

			   ret = phi_can_auto_get_id(&cenx4_can);
			   if (ret == MSG_OK)
			   {
				   break;
			   }
		   }
	   }
   }
}

void cenx4_can_handle_reset(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
	NVIC_SystemReset();
}

void cenx4_can_handle_start_bootloader(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
	const WDGConfig wdgcfg = {
		STM32_IWDG_PR_64,
		STM32_IWDG_RL(1),
		STM32_IWDG_WIN_DISABLED
	};

	wdgStart(&WDGD1, &wdgcfg);
}

void cenx4_can_handle_encoder_event(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    const phi_can_msg_data_io_encoder_event_t * msg = (phi_can_msg_data_io_encoder_event_t *) data;

    (void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

    if (len != sizeof(*msg))
    {
        return;
    }

    if (cenx4_is_master)
    {
    	phi_app_mgr_notify_encoder_event(src, msg->encoder_num, msg->val_change);
    }
}

void cenx4_can_handle_btn_event(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    const phi_can_msg_data_io_btn_event_t * msg = (phi_can_msg_data_io_btn_event_t *) data;

    (void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

    if (len != sizeof(*msg))
    {
        return;
    }

    if (cenx4_is_master)
    {
    	phi_app_mgr_notify_btn_event(src, msg->btn_num, msg->event, msg->param);
    }
}

void cenx4_can_handle_pot_event(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    const phi_can_msg_data_io_pot_event_t * msg = (phi_can_msg_data_io_pot_event_t *) data;

    (void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

    if (len != sizeof(*msg))
    {
        return;
    }

    if (cenx4_is_master)
    {
    	phi_app_mgr_notify_pot_event(src, msg->pot_num, msg->val);
    }
}

void cenx4_can_handle_set_rotenc_speed(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    const cenx4_can_handle_set_rotenc_speed_t * msg = (cenx4_can_handle_set_rotenc_speed_t *) data;

    (void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

    if ((len != sizeof(*msg)) ||
    	(msg->rotenc >= PHI_ARRLEN(cenx4_rotencs)))
    {
        return;
    }

    cenx4_rotencs[msg->rotenc].speed = msg->speed;
}

void cenx4_can_handle_unknown_cmd(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
	// Forward unknown events to current app
	phi_app_mgr_notify_can_cmd(prio, msg_id, src, chan_id, data, len);
}

msg_t cenx4_send_set_rotenc_speed(uint8_t node_id, uint8_t rotenc, uint8_t speed)
{
	msg_t ret;
	cenx4_can_handle_set_rotenc_speed_t msg = {
		.rotenc = rotenc,
		.speed = speed,
	};

	ret = phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST,
		PHI_CAN_MSG_ID_CENX4_SET_ROTENC_SPEED,
        node_id,
        (const uint8_t *) &msg,
        sizeof(msg),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );

    return ret;
}
