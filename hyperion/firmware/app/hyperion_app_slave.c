#include "hyperion_app_slave.h"
#include "hyperion_ui.h"

const phi_app_desc_t hyperion_app_slave_desc = {
    .start = hyperion_app_slave_start,
    .stop = hyperion_app_slave_stop,
    .encoder_event = hyperion_app_slave_encoder_event,
	.btn_event = hyperion_app_slave_btn_event,
	.pot_event = hyperion_app_slave_pot_event,
	.can_cmd = hyperion_app_slave_can_cmd,
};


void hyperion_app_slave_start(void * _ctx)
{
    hyperion_app_slave_context_t * ctx = (hyperion_app_slave_context_t *) _ctx;
    hyperion_ui_t * ui;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    ui = hyperion_ui_lock();
    memset(&(ui->state), 0, sizeof(ui->state));
    ui->dispmode = HYPERION_UI_DISPMODE_LOGO;
    hyperion_ui_unlock(ui);
}

void hyperion_app_slave_stop(void * ctx)
{
    (void) ctx;
}

void hyperion_app_slave_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
	phi_can_msg_data_io_encoder_event_t msg = {
        .encoder_num = encoder_num,
        .val_change = val_change,
    };

    phi_can_xfer(
        &hyperion_can,
        PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_ID_IO_ENCODER_EVENT,
        PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
        (const uint8_t *) &msg,
        sizeof(msg),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
}

void hyperion_app_slave_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
	phi_can_msg_data_io_btn_event_t msg = {
		.btn_num = btn_num,
		.event = event,
		.param = param,
    };

    phi_can_xfer(
        &hyperion_can,
        PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_ID_IO_BTN_EVENT,
        PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
        (const uint8_t *) &msg,
        sizeof(msg),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
}

void hyperion_app_slave_pot_event(void * ctx, uint8_t node_id, uint8_t pot_num, uint8_t val)
{
	phi_can_msg_data_io_pot_event_t msg = {
		.pot_num = pot_num,
		.val = val,
    };

    phi_can_xfer(
        &hyperion_can,
        PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_ID_IO_POT_EVENT,
        PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
        (const uint8_t *) &msg,
        sizeof(msg),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
}

void hyperion_app_slave_can_cmd(void * ctx, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t xfer_id, const uint8_t * data, size_t len)
{
	switch (msg_id)
	{
#if 0
	case PHI_CAN_MSG_ID_HYPERION_SET_DISPMODE:
		{
			const hyperion_can_handle_set_dispmode_t * msg = (hyperion_can_handle_set_dispmode_t *) data;

			if ((len != sizeof(*msg)) ||
				(msg->disp >= HYPERION_UI_NUM_DISPS) ||
				(msg->dispmode >= HYPERION_UI_NUM_DISPMODES))
			{
				break;
			}

			hyperion_ui_t * ui = hyperion_ui_lock(msg->disp);
			memset(&(ui->state), 0, sizeof(ui->state));
			ui->dispmode = msg->dispmode;
			hyperion_ui_unlock(ui);
		}
		break;

	case PHI_CAN_MSG_ID_HYPERION_SET_DISPMODE_STATE:
		{
			const hyperion_can_handle_set_dispmode_state_t * msg = (hyperion_can_handle_set_dispmode_state_t *) data;

			if ((len != sizeof(*msg)) ||
				(msg->disp >= HYPERION_UI_NUM_DISPS))
			{
				break;
			}

			hyperion_ui_t * ui = hyperion_ui_lock(msg->disp);
			memcpy(&(ui->state), &(msg->state), sizeof(ui->state));
			hyperion_ui_unlock(ui);
		}
		break;

	case PHI_CAN_MSG_ID_HYPERION_SET_SPLIT_POT_VAL:
		{
		    const hyperion_can_handle_set_split_pot_val_t * msg = (hyperion_can_handle_set_split_pot_val_t *) data;

		    if ((len != sizeof(*msg)) ||
		        (msg->disp >= HYPERION_UI_NUM_DISPS) ||
		        (msg->pot >= 2))
		    {
		        return;
		    }

		    hyperion_ui_t * ui = hyperion_ui_lock(msg->disp);
		    if (ui->dispmode == HYPERION_UI_DISPMODE_SPLIT_POT)
		    {
		        ui->state.split_pot.pots[msg->pot].val = msg->val;
		    }
		    hyperion_ui_unlock(ui);
		}
		break;

	case PHI_CAN_MSG_ID_HYPERION_SET_SPLIT_POT_TEXT:
		{
			const hyperion_can_handle_set_split_pot_text_t * msg = (hyperion_can_handle_set_split_pot_text_t *) data;

		    if ((len != sizeof(*msg)) ||
		        (msg->disp >= HYPERION_UI_NUM_DISPS) ||
		        (msg->pot >= 2))
		    {
		        return;
		    }

		    hyperion_ui_t * ui = hyperion_ui_lock(msg->disp);
		    if (ui->dispmode == HYPERION_UI_DISPMODE_SPLIT_POT)
		    {
		        memcpy(ui->state.split_pot.pots[msg->pot].text_top, msg->text_top, HYPERION_UI_MAX_LINE_TEXT_LEN);
		        memcpy(ui->state.split_pot.pots[msg->pot].text_bottom, msg->text_bottom, HYPERION_UI_MAX_LINE_TEXT_LEN);
		    }
		    hyperion_ui_unlock(ui);
		}
		break;
#endif
	}
}
