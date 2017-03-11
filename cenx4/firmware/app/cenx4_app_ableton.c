#include "cenx4_app_ableton.h"
#include "cenx4_app_registry.h"
#include "cenx4_conf.h"
#include "cenx4_io.h"
#include "cenx4_midi.h"


const phi_app_desc_t cenx4_app_ableton_desc = {
    .start = cenx4_app_ableton_start,
    .stop = cenx4_app_ableton_stop,
    .encoder_event = cenx4_app_ableton_encoder_event,
	.btn_event = cenx4_app_ableton_btn_event,
    .midi_sysex = cenx4_app_ableton_midi_sysex,
};

const uint8_t ableton_to_hw_pot_maps[][CENX4_MAX_POTS] = {
    // 4 pots (1 master)
    {
        0, 1, 2, 3,
    },
    // 8 pots (1 master + slave, horizontal layout)
    {
        0, 2, 4, 6,
        1, 3, 5, 7
    },
};

const uint8_t hw_to_ableton_pot_maps[][CENX4_MAX_POTS] = {
    // 4 pots (1 master)
    {
        0, 1, 2, 3,
    },
    // 8 pots (1 master + slave, horizontal layout)
    {
        0, 4, 1, 5,
        2, 6, 3, 7,
    },
};


void cenx4_app_ableton_start(void * _ctx)
{
    cenx4_app_ableton_context_t * ctx = (cenx4_app_ableton_context_t *) _ctx;
    cenx4_ui_t * ui;
    uint8_t i;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    for (i = 0; i < 4; ++i)
    {
    	cenx4_rotencs[i].speed = 1;
    }

    ctx->num_pots = 8; // TODO constant
    switch (ctx->num_pots)
    {
    case 4:
        ctx->ableton_to_hw_pot_map = (const uint8_t *) &(ableton_to_hw_pot_maps[0]);
        ctx->hw_to_ableton_pot_map = (const uint8_t *) &(hw_to_ableton_pot_maps[0]);
        break;
    case 8:
        ctx->ableton_to_hw_pot_map = (const uint8_t *) &(ableton_to_hw_pot_maps[1]);
        ctx->hw_to_ableton_pot_map = (const uint8_t *) &(hw_to_ableton_pot_maps[1]);
        break;
    default:
        chDbgCheck(FALSE);
    }

    // Move our displays into pots mode for test
    for (i = 0; i < 2; ++i)
    {
        ui = cenx4_ui_lock(i);

        ui->dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

        memset(&(ui->state), 0, sizeof(ui->state));

        ui->state.split_pot.pots[0].flags =
                CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
                CENX4_UI_DISPMODE_POT_FLAGS_FILL |
                CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
        chsnprintf(ui->state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %d", (i * 2) + 1);
        ui->state.split_pot.pots[0].val = 1;

        ui->state.split_pot.pots[1].flags = ui->state.split_pot.pots[0].flags;
        chsnprintf(ui->state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %d", (i * 2) + 2);
        ui->state.split_pot.pots[1].val = 1;

        cenx4_ui_unlock(ui);
    }

    // Move our slaves into ableton mode
    cenx4_app_ableton_berry_enter_ableton_mode(ctx, 10);

    // Notify ableton we are ready, in case it's waiting for us
    {
    	struct {
    		uint8_t cmd;
    		cenx4_app_ableton_sysex_resync_t data;
    	} resync_msg = {
			.cmd =  CENX4_APP_ABLETON_SYSEX_RESYNC,
			.data = {
				.num_modules = 2, // TODO const
			},
    	};

    	phi_midi_tx_sysex(PHI_MIDI_PORT_USB, CENX4_MIDI_SYSEX_APP_CMD, &resync_msg, sizeof(resync_msg));
    }
}

void cenx4_app_ableton_stop(void * ctx)
{
    (void) ctx;
}

void cenx4_app_ableton_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    // TODO this ignores node_id at the moment
    cenx4_app_ableton_context_t * ctx = (cenx4_app_ableton_context_t *) _ctx;
    phi_midi_pkt_t pkt;

    if (node_id == 10) {
        encoder_num += 4;
    }

    // Dat shitty mapping
    encoder_num = ctx->hw_to_ableton_pot_map[encoder_num];

    pkt.chn = 2;
    pkt.event = 0xB; // Control Change
    pkt.val1 = encoder_num;
    pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset

    // This is double on purpose. Ableton seems to change value every other message, and this makes the encoder more responsive
    phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
    phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
}

void cenx4_app_ableton_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
	(void) ctx;

	// Setup mode
	if (node_id == 0)
	{
		if ((btn_num == 0) &&
			(event == PHI_BTN_EVENT_HELD) &&
			(param >= 5000))
		{
			phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
			return;
		}

		switch (event)
		{
		case PHI_BTN_EVENT_PRESSED:
			cenx4_rotencs[btn_num].speed = 2;
			break;

		case PHI_BTN_EVENT_RELEASED:
			cenx4_rotencs[btn_num].speed = 1;
			break;

		default:
			break;
		}
	}
}

void cenx4_app_ableton_midi_sysex(void * _ctx, phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    cenx4_app_ableton_context_t * ctx = (cenx4_app_ableton_context_t *) _ctx;
    cenx4_app_ableton_sysex_cmd_t app_cmd = (cenx4_app_ableton_sysex_cmd_t) ((uint8_t *) data)[0];

    (void) port;

    chDbgCheck(cmd == CENX4_MIDI_SYSEX_APP_CMD);

    // TODO counters for all the failures here

    if (data_len < 1)
    {
        return;
    }

    data = &(((uint8_t *)data)[1]);
    --data_len;

    switch (app_cmd)
    {
    case CENX4_APP_ABLETON_SYSEX_SET_POT_VALUE:
        if (data_len == sizeof(cenx4_app_ableton_sysex_set_pot_value_t))
        {
            cenx4_app_ableton_midi_sysex_set_pot_value(ctx, (cenx4_app_ableton_sysex_set_pot_value_t *) data);
        }
        break;

    case CENX4_APP_ABLETON_SYSEX_SET_POT_TEXT:
        if (data_len == sizeof(cenx4_app_ableton_sysex_set_pot_text_t))
        {
            cenx4_app_ableton_midi_sysex_set_pot_text(ctx, (cenx4_app_ableton_sysex_set_pot_text_t *) data);
        }
        break;

    case CENX4_APP_ABLETON_SYSEX_INVALID:
    default:
        break;
    }
}

msg_t cenx4_app_ableton_berry_enter_ableton_mode(cenx4_app_ableton_context_t * ctx, uint8_t node_id)
{
    msg_t ret;

    cenx4_can_handle_set_dispmode_t dispmode;

    // Put display 0 in split pot mode
    dispmode.disp = 0;
    dispmode.dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

    ret = phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST,
        PHI_CAN_MSG_ID_CENX4_SET_DISPMODE,
        node_id,
        (const uint8_t *) &dispmode,
        sizeof(dispmode),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
    if (MSG_OK != ret)
    {
        return ret;
    }

    // Put display 1 in split pot mode
    dispmode.disp = 1;
    dispmode.dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

    ret = phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST,
        PHI_CAN_MSG_ID_CENX4_SET_DISPMODE,
        node_id,
        (const uint8_t *) &dispmode,
        sizeof(dispmode),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
    if (MSG_OK != ret)
    {
        return ret;
    }

    // Set text
    ret = cenx4_app_ableton_berry_update_ui(ctx, node_id);
    if (MSG_OK != ret)
    {
        return ret;
    }

    // Success
    return MSG_OK;
}

msg_t cenx4_app_ableton_berry_update_ui(cenx4_app_ableton_context_t * ctx, uint8_t node_id)
{
    cenx4_can_handle_set_dispmode_state_t state;
    msg_t ret;
    uint8_t base = (node_id - PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + 1) + 4;

    (void) ctx;

    // Display 0
    memset(&state, 0, sizeof(state));
    state.disp = 0;

    state.state.split_pot.pots[0].flags =
            CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
            CENX4_UI_DISPMODE_POT_FLAGS_FILL |
            CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
    chsnprintf(state.state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %d", base);
    state.state.split_pot.pots[0].val = 1;

    state.state.split_pot.pots[1].flags =  state.state.split_pot.pots[0].flags;
    chsnprintf(state.state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %d", base + 1);
    state.state.split_pot.pots[1].val = 1;

    ret = phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST,
        PHI_CAN_MSG_ID_CENX4_SET_DISPMODE_STATE,
        node_id,
        (const uint8_t *) &state,
        sizeof(state),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
    if (MSG_OK != ret)
    {
        return ret;
    }

    // Display 1
    memset(&state, 0, sizeof(state));
    state.disp = 1;

    state.state.split_pot.pots[0].flags =
            CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
            CENX4_UI_DISPMODE_POT_FLAGS_FILL |
            CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
    chsnprintf(state.state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %d", base + 2);
    state.state.split_pot.pots[0].val = 1;

    state.state.split_pot.pots[1].flags = state.state.split_pot.pots[0].flags;
    chsnprintf(state.state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %d", base + 3);
    state.state.split_pot.pots[1].val = 1;

    ret = phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST,
        PHI_CAN_MSG_ID_CENX4_SET_DISPMODE_STATE,
        node_id,
        (const uint8_t *) &state,
        sizeof(state),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
    if (MSG_OK != ret)
    {
        return ret;
    }


    return MSG_OK;
}

void cenx4_app_ableton_midi_sysex_set_pot_value(cenx4_app_ableton_context_t * ctx, cenx4_app_ableton_sysex_set_pot_value_t * data)
{
    cenx4_ui_t * ui;

    if (data->pot >= ctx->num_pots)
    {
        return;
    }

    // From ableton pot number to the shitty logic below pot number
    data->pot = ctx->ableton_to_hw_pot_map[data->pot];

    if (data->pot < 4)
    {
        ui = cenx4_ui_lock(data->pot / 2);
        ui->state.split_pot.pots[data->pot % 2].val = data->val;
        cenx4_ui_unlock(ui);
    }
    else
    {
        cenx4_can_handle_set_split_pot_val_t msg;

        data->pot -= 4;

        msg.disp = data->pot / 2;
        msg.pot = data->pot % 2;
        msg.val = data->val;
        phi_can_xfer(
            &cenx4_can,
            PHI_CAN_PRIO_LOWEST,
            PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_VAL,
            10, // TODO jesus
            (const uint8_t *) &msg,
            sizeof(msg),
            NULL,
            0,
            NULL,
            PHI_CAN_DEFAULT_TIMEOUT
        );
    }
}

void cenx4_app_ableton_midi_sysex_set_pot_text(cenx4_app_ableton_context_t * ctx, cenx4_app_ableton_sysex_set_pot_text_t * data)
{
    cenx4_ui_t * ui;

    if (data->pot >= ctx->num_pots)
    {
        return;
    }

    // From ableton pot number to the shitty logic below pot number
    data->pot = ctx->ableton_to_hw_pot_map[data->pot];

    if (data->pot < 4)
    {
        ui = cenx4_ui_lock(data->pot / 2);
        memcpy(&(ui->state.split_pot.pots[data->pot % 2].text_top), data->text_top, CENX4_UI_MAX_LINE_TEXT_LEN);
        memcpy(&(ui->state.split_pot.pots[data->pot % 2].text_bottom), data->text_bottom, CENX4_UI_MAX_LINE_TEXT_LEN);
        cenx4_ui_unlock(ui);
    }
    else
    {
        cenx4_can_handle_set_split_pot_text_t msg;

        data->pot -= 4;
        msg.disp = data->pot / 2;
        msg.pot = data->pot % 2;
        memcpy(&(msg.text_top), data->text_top, CENX4_UI_MAX_LINE_TEXT_LEN);
        memcpy(&(msg.text_bottom), data->text_bottom, CENX4_UI_MAX_LINE_TEXT_LEN);

        phi_can_xfer(
            &cenx4_can,
            PHI_CAN_PRIO_LOWEST,
            PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_TEXT,
            10, // TODO jesus
            (const uint8_t *) &msg,
            sizeof(msg),
            NULL,
            0,
            NULL,
            PHI_CAN_DEFAULT_TIMEOUT
        );
    }
}
