#include "phi_lib/phi_app_mgr.h"

#include "cenx4_can.h"
#include "cenx4_conf.h"
#include "cenx4_ui.h"


static const CANConfig cancfg_500k = {
	.mcr =
		CAN_MCR_TXFP | // Transmit FIFO priority -  Priority driven by the request order (chronologically)
		CAN_MCR_ABOM, //  Automatic bus-off management
	.btr =
		CAN_BTR_SJW(0) |
        CAN_BTR_TS1(12) | CAN_BTR_TS2(1) | CAN_BTR_BRP(2), // 750k @ 36MHz

};


static const phi_can_msg_handler_t can_handlers[] = {
	// Bultin stuff
	PHI_CAN_BUILTIN_MSG_HANDLERS,

	// UI
	{PHI_CAN_MSG_ID_CENX4_ENCODER_EVENT, cenx4_can_handle_encoder_event, NULL},
#if 0
	{PHI_CAN_MSG_ID_CENX4_SET_DISPMODE, cenx4_can_handle_set_dispmode, NULL},
	{PHI_CAN_MSG_ID_CENX4_SET_DISPMODE_STATE, cenx4_can_handle_set_dispmode_state, NULL},
	{PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_VAL, cenx4_can_handle_set_split_pot_val, NULL},
	{PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_TEXT, cenx4_can_handle_set_split_pot_text, NULL},
#endif
	// ...

};

static const phi_can_config_t can1_cfg = {
	.drv = &CAND1,
	.drv_cfg = &cancfg_500k,
	.handlers = can_handlers,
	.n_handlers = sizeof(can_handlers) / sizeof(can_handlers[0]),
	.dev_id = CENX4_DEV_ID,
	.hw_sw_ver = CENX4_HW_SW_VER,
};



phi_can_t cenx4_can;


volatile unsigned int loops=0;
//volatile unsigned int can_errs=0;


void cenx4_can_init(void)
{
   //phi_can_init(&cenx4_can, &can1_cfg, PHI_CAN_AUTO_ID);
    phi_can_init(&cenx4_can, &can1_cfg, PHI_CAN_AUTO_ID_ALLOCATOR_NODE);

   palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(9));
   palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(9) | PAL_STM32_OSPEED_HIGHEST);

#if 0

   {
	   int tries = 0;
	   char buf[16];
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

	   ui->state.boot.misc_text[0] = 0;
   }



   phi_can_msg_data_sysinfo_t si;
   uint32_t bytes;
   while (false)
   {
	   memset(&si, 0, sizeof(si));
	   bytes = 0;
	   msg_t ret = phi_can_xfer(&cenx4_can, 0, PHI_CAN_MSG_ID_SYSINFO, 2, NULL, 0, (uint8_t *)&si, sizeof(si), &bytes, MS2ST(100)); //TIME_INFINITE);
	   if ((ret != MSG_OK) || (sizeof(si) != bytes)) {
		   continue;
	   }
	   ++loops;

	   chDbgAssert(ret == MSG_OK, "WTF");
	   chDbgAssert(si.dev_id == 0x42455259, "WTF");
	   chDbgAssert((si.hw_sw_ver == 0x0202) || (si.hw_sw_ver == 0x0101), "WTF");
	   chDbgAssert(si.can_version == PHI_CAN_VERSION, "WTF");
	   //chThdSleepMilliseconds(500);
   }
#endif
}

void cenx4_can_handle_encoder_event(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    const cenx4_can_handle_encoder_event_t * msg = (cenx4_can_handle_encoder_event_t *) data;

    (void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

    if (len != sizeof(*msg))
    {
        return;
    }

    phi_app_mgr_notify_encoder_event(src, msg->encoder_num, msg->val_change);
}

void cenx4_can_handle_set_dispmode(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
	const cenx4_can_handle_set_dispmode_t * msg = (cenx4_can_handle_set_dispmode_t *) data;

	(void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

	if ((len != sizeof(*msg)) ||
		(msg->disp >= CENX4_UI_NUM_DISPS) ||
		(msg->dispmode >= CENX4_UI_NUM_DISPMODES))
	{
		return;
	}

	cenx4_ui_t * ui = cenx4_ui_lock(msg->disp);
	memset(&(ui->state), 0, sizeof(ui->state));
	ui->dispmode = msg->dispmode;
	cenx4_ui_unlock(ui);

	// Ack
	// phi_can_send_reply(can, prio, msg_id, src, chan_id, NULL, 0, PHI_CAN_DEFAULT_TIMEOUT);
}

void cenx4_can_handle_set_dispmode_state(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
	const cenx4_can_handle_set_dispmode_state_t * msg = (cenx4_can_handle_set_dispmode_state_t *) data;

	(void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

	if ((len != sizeof(*msg)) ||
		(msg->disp >= CENX4_UI_NUM_DISPS))
	{
		return;
	}

	cenx4_ui_t * ui = cenx4_ui_lock(msg->disp);
	memcpy(&(ui->state), &(msg->state), sizeof(ui->state));
	cenx4_ui_unlock(ui);

	// Ack
	// phi_can_send_reply(can, prio, msg_id, src, chan_id, NULL, 0, PHI_CAN_DEFAULT_TIMEOUT);
}


void cenx4_can_handle_set_split_pot_val(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    const cenx4_can_handle_set_split_pot_val_t * msg = (cenx4_can_handle_set_split_pot_val_t *) data;

    (void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

    if ((len != sizeof(*msg)) ||
        (msg->disp >= CENX4_UI_NUM_DISPS) ||
        (msg->pot >= 2))
    {
        return;
    }

    cenx4_ui_t * ui = cenx4_ui_lock(msg->disp);
    if (ui->dispmode == CENX4_UI_DISPMODE_SPLIT_POT)
    {
        ui->state.split_pot.pots[msg->pot].val = msg->val;
    }
    cenx4_ui_unlock(ui);
}


void cenx4_can_handle_set_split_pot_text(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    const cenx4_can_handle_set_split_pot_text_t * msg = (cenx4_can_handle_set_split_pot_text_t *) data;

    (void) can; (void) context; (void) prio; (void) msg_id; (void) src; (void) chan_id;

    if ((len != sizeof(*msg)) ||
        (msg->disp >= CENX4_UI_NUM_DISPS) ||
        (msg->pot >= 2))
    {
        return;
    }

    cenx4_ui_t * ui = cenx4_ui_lock(msg->disp);
    if (ui->dispmode == CENX4_UI_DISPMODE_SPLIT_POT)
    {
        memcpy(ui->state.split_pot.pots[msg->pot].text_top, msg->text_top, CENX4_UI_MAX_LINE_TEXT_LEN);
        memcpy(ui->state.split_pot.pots[msg->pot].text_bottom, msg->text_bottom, CENX4_UI_MAX_LINE_TEXT_LEN);
    }
    cenx4_ui_unlock(ui);
}
