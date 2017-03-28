#include "hyperion_can.h"

static const CANConfig cancfg_500k = {
    .mcr =
        CAN_MCR_TXFP | // Transmit FIFO priority -  Priority driven by the request order (chronologically)
        CAN_MCR_ABOM, //  Automatic bus-off management
    .btr =
        CAN_BTR_SJW(0) |
        CAN_BTR_TS1(10) | CAN_BTR_TS2(1) | CAN_BTR_BRP(3), // 750k @ 42MHz
        //CAN_BTR_LBKM, // Loopback
};


static const phi_can_msg_handler_t can_handlers[] = {
	// Bultin stuff
	PHI_CAN_BUILTIN_MSG_HANDLERS,

	// Bootloader
	// PHI_CAN_BL_MSG_HANDLERS,

	// UI
	// {PHI_CAN_MSG_ID_BERRY_SET_DISPMODE, phi_berry_can_handle_set_dispmode, NULL},
	// {PHI_CAN_MSG_ID_BERRY_SET_DISPMODE_STATE, phi_berry_can_handle_set_dispmode_state, NULL},
};

static const phi_can_config_t can1_cfg = {
	.drv = &CAND1,
	.drv_cfg = &cancfg_500k,
	.handlers = can_handlers,
	.n_handlers = sizeof(can_handlers) / sizeof(can_handlers[0]),
	.dev_id = HYPERION_DEV_ID,
	.hw_sw_ver = HYPERION_HW_SW_VER,
};



phi_can_t hyperion_can;


void hyperion_can_init(void)
{
    phi_can_init(&hyperion_can, &can1_cfg, PHI_CAN_AUTO_ID);


    palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(9));
    palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(9) | PAL_STM32_OSPEED_HIGHEST);


   {
	   int tries = 0;
	   msg_t ret;

	   for (;;)
	   {
		   ++tries;
		   //ui = phi_ui_lock(0);
		   //chsnprintf(ui->state.boot.misc_text[0], sizeof(ui->state.boot.misc_text[0])-1, "AutoId #%d", tries);
		   //phi_ui_unlock(ui);

		   ret = phi_can_auto_get_id(&hyperion_can);
		   if (ret == MSG_OK)
		   {
			   break;
		   }
	   }

	   //ui->state.boot.misc_text[0][0] = 0;
   }
}
