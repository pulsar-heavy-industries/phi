#include "cenx4_app_slave.h"
#include "cenx4_conf.h"


const phi_app_desc_t cenx4_app_slave_desc = {
    .start = cenx4_app_slave_start,
    .stop = cenx4_app_slave_stop,
    .encoder_event = cenx4_app_slave_encoder_event,
	.btn_event = cenx4_app_slave_btn_event,
	.can_cmd = cenx4_app_slave_can_cmd,
};


void cenx4_app_slave_start(void * _ctx)
{
    cenx4_app_slave_context_t * ctx = (cenx4_app_slave_context_t *) _ctx;
    cenx4_ui_t * ui;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    ui = cenx4_ui_lock(0);
    ui->dispmode = CENX4_UI_DISPMODE_LOGO;
    cenx4_ui_unlock(ui);

    ui = cenx4_ui_lock(1);
    ui->dispmode = CENX4_UI_DISPMODE_BOOT;
    strcpy(ui->state.boot.misc_text, "SLAVE");
    cenx4_ui_unlock(ui);
}

void cenx4_app_slave_stop(void * ctx)
{
    (void) ctx;
}

void cenx4_app_slave_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
	cenx4_can_handle_encoder_event_t msg = {
        .encoder_num = encoder_num,
        .val_change = val_change,
    };

    phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_ID_CENX4_ENCODER_EVENT,
        PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
        (const uint8_t *) &msg,
        sizeof(msg),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
}

void cenx4_app_slave_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
	cenx4_can_handle_btn_event_t msg = {
		.btn_num = btn_num,
		.event = event,
		.param = param,
    };

    phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_ID_CENX4_BTN_EVENT,
        PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
        (const uint8_t *) &msg,
        sizeof(msg),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
}

void cenx4_app_slave_can_cmd(void * ctx, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t xfer_id, const uint8_t * data, size_t len)
{
	chDbgCheck(cenx4_is_master == FALSE);

	switch (msg_id)
	{
	case PHI_CAN_MSG_ID_CENX4_SET_DISPMODE:
		{
			const cenx4_can_handle_set_dispmode_t * msg = (cenx4_can_handle_set_dispmode_t *) data;

			if ((len != sizeof(*msg)) ||
				(msg->disp >= CENX4_UI_NUM_DISPS) ||
				(msg->dispmode >= CENX4_UI_NUM_DISPMODES))
			{
				break;
			}

			cenx4_ui_t * ui = cenx4_ui_lock(msg->disp);
			memset(&(ui->state), 0, sizeof(ui->state));
			ui->dispmode = msg->dispmode;
			cenx4_ui_unlock(ui);
		}
		break;

	case PHI_CAN_MSG_ID_CENX4_SET_DISPMODE_STATE:
		{
			const cenx4_can_handle_set_dispmode_state_t * msg = (cenx4_can_handle_set_dispmode_state_t *) data;

			if ((len != sizeof(*msg)) ||
				(msg->disp >= CENX4_UI_NUM_DISPS))
			{
				break;
			}

			cenx4_ui_t * ui = cenx4_ui_lock(msg->disp);
			memcpy(&(ui->state), &(msg->state), sizeof(ui->state));
			cenx4_ui_unlock(ui);
		}
		break;

	case PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_VAL:
		{
		    const cenx4_can_handle_set_split_pot_val_t * msg = (cenx4_can_handle_set_split_pot_val_t *) data;

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
		break;

	case PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_TEXT:
		{
			const cenx4_can_handle_set_split_pot_text_t * msg = (cenx4_can_handle_set_split_pot_text_t *) data;

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
		break;
	}
}
