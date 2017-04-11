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
    // 12 pots (1 master + 2 slaves, horizontal layout)
    {
        0, 2, 4, 6, 8, 10,
        1, 3, 5, 7, 9, 11,
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
    // 12 pots (1 master + 2 slaves, horizontal layout)
    {
    	0, 6, 1, 7, 2, 8,
		3, 9, 4, 10, 5, 11,
    },
};


void cenx4_app_ableton_start(void * _ctx)
{
    cenx4_app_ableton_context_t * ctx = (cenx4_app_ableton_context_t *) _ctx;
    cenx4_ui_t * ui;
    uint8_t i;
    uint8_t mod_num;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    for (i = 0; i < 4; ++i)
    {
    	cenx4_rotencs[i].speed = 1;
    }

    // Load mod_num_to_uid from cfg
    cenx4_app_cfg_get_node_id_to_mod_num_map(
  		ctx->node_id_to_mod_num,
		PHI_ARRLEN(cenx4_app_cfg.cur.mod_num_to_uid)
	);

    cenx4_app_log("Ableton!");

    // Construct a reverse map
    memset(ctx->mod_num_to_node_id, CENX4_APP_CFG_INVALID_MODULE_NUM, sizeof(ctx->mod_num_to_node_id));

    for (i = 0; i < CENX4_APP_CFG_MAX_MODULES; ++i)
    {
		if (ctx->node_id_to_mod_num[i] != CENX4_APP_CFG_INVALID_MODULE_NUM)
		{
			chDbgCheck(ctx->node_id_to_mod_num[i] < CENX4_APP_CFG_MAX_MODULES);
			ctx->mod_num_to_node_id[ctx->node_id_to_mod_num[i]] = (i == 0) ? 0 : i + PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID - 1;
		}
    }


    // See what we can learn about our module configuration.
    // We start with one module because we (the master) act as one
    ctx->num_modules = 0;

    for (mod_num = 0; mod_num < CENX4_APP_CFG_MAX_MODULES; ++mod_num)
    {
    	// Short-circuit if we're testing ourselves
    	if (ctx->mod_num_to_node_id[mod_num] == 0)
    	{
    		ctx->num_modules++;
    		continue;
    	}

    	if (ctx->mod_num_to_node_id[mod_num] == CENX4_APP_CFG_INVALID_MODULE_NUM)
    	{
    		// We don't allow "holes" in our module numbers. Once we encounter an invalid module number
    		// (effectively an unused entry in the table) we expect all others to also be unused
    		break;
    	}

    	// Check to see if the module is there, and if it's a CENX4
    	phi_can_msg_data_sysinfo_t si;
    	uint32_t bytes;
    	msg_t ret;
    	memset(&si, 0, sizeof(si));
    	ret = phi_can_xfer(&cenx4_can, 1, PHI_CAN_MSG_ID_SYSINFO, ctx->mod_num_to_node_id[mod_num], NULL, 0, (uint8_t *)&si, sizeof(si), &bytes, MS2ST(100));
    	if (ret != MSG_OK)
    	{
    		cenx4_app_log_fmt("M%dR%d", mod_num, ret);
    		goto err_bad_cfg;
    	}

    	if (sizeof(si) != bytes)
		{
			cenx4_app_log_fmt("M%dB%d", mod_num, bytes);
			goto err_bad_cfg;
		}

		if ((si.dev_id != PHI_DEV_ID('C', 'N', 'X', '4') && (si.dev_id != PHI_DEV_ID('B', 'E', 'R', 'Y')))) // TODO
		{
			cenx4_app_log_fmt("M%dDevId?", mod_num);
			cenx4_app_log_fmt("%x", si.dev_id);
    		goto err_bad_cfg;
    	}

    	// All going according to plan
    	ctx->num_modules++;
    }

    // Verify we don't have the "holes" mentioned aboive
    for (; mod_num < CENX4_APP_CFG_MAX_MODULES; ++mod_num)
    {
    	if (ctx->mod_num_to_node_id[mod_num] != CENX4_APP_CFG_INVALID_MODULE_NUM)
    	{
    		cenx4_app_log_fmt("Hole?%d", mod_num);
    		goto err_bad_cfg;
    	}
    }

    if ((ctx->num_modules >= 1) && (ctx->num_modules <= 3))
    {
    	ctx->ableton_to_hw_pot_map = (const uint8_t *) &(ableton_to_hw_pot_maps[ctx->num_modules - 1]);
		ctx->hw_to_ableton_pot_map = (const uint8_t *) &(hw_to_ableton_pot_maps[ctx->num_modules - 1]);
    }
    else
    {
    	cenx4_app_log_fmt("NumM?%d", ctx->num_modules);
        goto err_bad_cfg;
    }

    cenx4_app_log("CfgOk!");

    // Move our displays into pots mode
    uint8_t base = (ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(0)] * 4) + 1;
    for (i = 0; i < 2; ++i)
    {
        ui = cenx4_ui_lock(i);

        memset(&(ui->state), 0, sizeof(ui->state));

        if (ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(0)] != CENX4_APP_CFG_INVALID_MODULE_NUM)
        {
			ui->dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

			ui->state.split_pot.pots[0].flags =
					CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
					CENX4_UI_DISPMODE_POT_FLAGS_FILL |
					CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
			chsnprintf(ui->state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %02d", base + (i * 2));
			ui->state.split_pot.pots[0].val = 1;

			ui->state.split_pot.pots[1].flags = ui->state.split_pot.pots[0].flags;
			chsnprintf(ui->state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %02d", base + (i * 2) + 1);
			ui->state.split_pot.pots[1].val = 1;
        }
        else
        {
			ui->dispmode = CENX4_UI_DISPMODE_LOGO;

        }

        cenx4_ui_unlock(ui);
    }

    // Move our slaves into ableton mode
    for (mod_num = 0; mod_num < ctx->num_modules; ++mod_num)
    {
    	if (ctx->mod_num_to_node_id[mod_num] != 0)
    	{
    		cenx4_app_ableton_enter_ableton_mode(ctx, ctx->mod_num_to_node_id[mod_num]);
    	}
    }

    // Notify Ableton we are ready, in case it's waiting for us
    cenx4_app_ableton_send_resync(ctx);

    return;

err_bad_cfg:
	phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
}

void cenx4_app_ableton_stop(void * ctx)
{
    (void) ctx;
}

void cenx4_app_ableton_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    cenx4_app_ableton_context_t * ctx = (cenx4_app_ableton_context_t *) _ctx;
    phi_midi_pkt_t pkt;
    uint8_t mod_num;

    // See if this is a node id we're working with
    mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];
    if (mod_num == CENX4_APP_CFG_INVALID_MODULE_NUM)
    {
    	return;
    }

    encoder_num += mod_num * 4;

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

    case CENX4_APP_ABLETON_SYSEX_SET_POT_ALL:
        if (data_len == sizeof(cenx4_app_ableton_sysex_set_pot_all_t))
        {
            cenx4_app_ableton_midi_sysex_set_pot_all(ctx, (cenx4_app_ableton_sysex_set_pot_all_t *) data);
        }
        break;

    case CENX4_APP_ABLETON_SYSEX_RESYNC:
    	cenx4_app_ableton_send_resync(ctx);
    	break;

    case CENX4_APP_ABLETON_SYSEX_INVALID:
    default:
        break;
    }
}

msg_t cenx4_app_ableton_enter_ableton_mode(cenx4_app_ableton_context_t * ctx, uint8_t node_id)
{
    msg_t ret;

    // Set text
    ret = cenx4_app_ableton_update_ui(ctx, node_id);
    if (MSG_OK != ret)
    {
        return ret;
    }

    // Success
    return MSG_OK;
}

msg_t cenx4_app_ableton_update_ui(cenx4_app_ableton_context_t * ctx, uint8_t node_id)
{
	cenx4_can_handle_update_display_state_t state;
    msg_t ret;
    uint8_t base = (ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)] * 4) + 1;

    (void) ctx;

    // Display 0
    memset(&state, 0, sizeof(state));
    state.disp = 0;
    state.dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

    state.state.split_pot.pots[0].flags =
            CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
            CENX4_UI_DISPMODE_POT_FLAGS_FILL |
            CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
    chsnprintf(state.state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %02d", base);
    state.state.split_pot.pots[0].val = 1;

    state.state.split_pot.pots[1].flags =  state.state.split_pot.pots[0].flags;
    chsnprintf(state.state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %02d", base + 1);
    state.state.split_pot.pots[1].val = 1;

    ret = phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST,
		PHI_CAN_MSG_ID_CENX4_UPDATE_DISPLAY,
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
    state.dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

    state.state.split_pot.pots[0].flags =
            CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
            CENX4_UI_DISPMODE_POT_FLAGS_FILL |
            CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
    chsnprintf(state.state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %02d", base + 2);
    state.state.split_pot.pots[0].val = 1;

    state.state.split_pot.pots[1].flags = state.state.split_pot.pots[0].flags;
    chsnprintf(state.state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot? %02d", base + 3);
    state.state.split_pot.pots[1].val = 1;

    ret = phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST,
		PHI_CAN_MSG_ID_CENX4_UPDATE_DISPLAY,
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

void cenx4_app_ableton_send_resync(cenx4_app_ableton_context_t * ctx)
{
	struct {
		uint8_t cmd;
		cenx4_app_ableton_sysex_resync_t data;
	} resync_msg = {
		.cmd =  CENX4_APP_ABLETON_SYSEX_RESYNC,
		.data = {
			.num_modules = ctx->num_modules,
		},
	};

	phi_midi_tx_sysex(PHI_MIDI_PORT_USB, CENX4_MIDI_SYSEX_APP_CMD, &resync_msg, sizeof(resync_msg));
}

void cenx4_app_ableton_midi_sysex_set_pot_value(cenx4_app_ableton_context_t * ctx, cenx4_app_ableton_sysex_set_pot_value_t * data)
{
    cenx4_ui_t * ui;
    uint8_t mod_num, pot_num, node_id;

    if (data->pot >= (ctx->num_modules * 4))
    {
        return;
    }

    // From Ableton pot number to the shitty logic below pot number
    data->pot = ctx->ableton_to_hw_pot_map[data->pot];
    mod_num = data->pot / 4;
    pot_num = data->pot % 4;
    node_id = ctx->mod_num_to_node_id[mod_num];
    chDbgCheck(node_id != 0xff);

    if (node_id == 0)
    {
        ui = cenx4_ui_lock(pot_num / 2);
        ui->state.split_pot.pots[pot_num % 2].val = data->val;
        cenx4_ui_unlock(ui);
    }
    else
    {
        cenx4_can_handle_set_split_pot_val_t msg;

        msg.disp = pot_num / 2;
        msg.pot = pot_num % 2;
        msg.val = data->val;
        phi_can_xfer(
            &cenx4_can,
            PHI_CAN_PRIO_LOWEST,
            PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_VAL,
            node_id,
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
    uint8_t mod_num, pot_num, node_id;

    if (data->pot >= (ctx->num_modules * 4))
    {
        return;
    }

    // From ableton pot number to the shitty logic below pot number
    data->pot = ctx->ableton_to_hw_pot_map[data->pot];
    mod_num = data->pot / 4;
    pot_num = data->pot % 4;
    node_id = ctx->mod_num_to_node_id[mod_num];
    chDbgCheck(node_id != 0xff);

    if (node_id == 0)
    {
        ui = cenx4_ui_lock(pot_num / 2);
        memcpy(&(ui->state.split_pot.pots[pot_num % 2].text_top), data->text_top, CENX4_UI_MAX_LINE_TEXT_LEN);
        memcpy(&(ui->state.split_pot.pots[pot_num % 2].text_bottom), data->text_bottom, CENX4_UI_MAX_LINE_TEXT_LEN);
        cenx4_ui_unlock(ui);
    }
    else
    {
        cenx4_can_handle_set_split_pot_text_t msg;

        msg.disp = pot_num / 2;
        msg.pot = pot_num % 2;
        memcpy(&(msg.text_top), data->text_top, CENX4_UI_MAX_LINE_TEXT_LEN);
        memcpy(&(msg.text_bottom), data->text_bottom, CENX4_UI_MAX_LINE_TEXT_LEN);

        phi_can_xfer(
            &cenx4_can,
            PHI_CAN_PRIO_LOWEST,
            PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT_TEXT,
            node_id,
            (const uint8_t *) &msg,
            sizeof(msg),
            NULL,
            0,
            NULL,
            PHI_CAN_DEFAULT_TIMEOUT
        );
    }
}

void cenx4_app_ableton_midi_sysex_set_pot_all(cenx4_app_ableton_context_t * ctx, cenx4_app_ableton_sysex_set_pot_all_t * data)
{
    cenx4_ui_t * ui;
    uint8_t mod_num, pot_num, node_id;

    if (data->pot >= (ctx->num_modules * 4))
    {
        return;
    }

    // From Ableton pot number to the shitty logic below pot number
    data->pot = ctx->ableton_to_hw_pot_map[data->pot];
    mod_num = data->pot / 4;
    pot_num = data->pot % 4;
    node_id = ctx->mod_num_to_node_id[mod_num];
    chDbgCheck(node_id != 0xff);

    if (node_id == 0)
    {
        ui = cenx4_ui_lock(pot_num / 2);
        ui->state.split_pot.pots[pot_num % 2].val = data->val;
        memcpy(&(ui->state.split_pot.pots[pot_num % 2].text_top), data->text_top, CENX4_UI_MAX_LINE_TEXT_LEN);
        memcpy(&(ui->state.split_pot.pots[pot_num % 2].text_bottom), data->text_bottom, CENX4_UI_MAX_LINE_TEXT_LEN);
        cenx4_ui_unlock(ui);
    }
    else
    {
        cenx4_can_handle_set_split_pot_t msg;

        msg.disp = pot_num / 2;
        msg.pot = pot_num % 2;
        msg.val = data->val;
		memcpy(&(msg.text_top), data->text_top, CENX4_UI_MAX_LINE_TEXT_LEN);
		memcpy(&(msg.text_bottom), data->text_bottom, CENX4_UI_MAX_LINE_TEXT_LEN);

        phi_can_xfer(
            &cenx4_can,
            PHI_CAN_PRIO_LOWEST,
            PHI_CAN_MSG_ID_CENX4_SET_SPLIT_POT,
            node_id,
            (const uint8_t *) &msg,
            sizeof(msg),
            NULL,
            0,
            NULL,
            PHI_CAN_DEFAULT_TIMEOUT
        );
    }
}

