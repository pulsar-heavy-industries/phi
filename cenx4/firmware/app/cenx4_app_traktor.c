#include "cenx4_app_traktor.h"
#include "cenx4_app_registry.h"
#include "cenx4_conf.h"
#include "../../../hyperion/firmware/app/hyperion_app_slave_can.h"

const phi_app_desc_t cenx4_app_traktor_desc = {
    .start = cenx4_app_traktor_start,
    .stop = cenx4_app_traktor_stop,
	.tick = cenx4_app_traktor_tick,
	.tick_interval = MS2ST(10),
    .encoder_event = cenx4_app_traktor_encoder_event,
	.btn_event = cenx4_app_traktor_btn_event,
	.pot_event = cenx4_app_traktor_pot_event,
	.midi_cc = cenx4_app_traktor_midi_cc,
};


void cenx4_app_traktor_start(void * _ctx)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
    uint8_t i;
    uint8_t mod_num;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    // Load mod_num_to_uid from cfg
	cenx4_app_cfg_get_node_id_to_mod_num_map(
		HYPERION_DEV_ID,
		ctx->node_id_to_mod_num,
		PHI_ARRLEN(cenx4_app_cfg.cur.hyperion_mod_num_to_uid)
	);

	cenx4_app_log("Traktor!");

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

		if (si.dev_id != HYPERION_DEV_ID)
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

	cenx4_app_log("CfgOk!");

	// Configure displays
    cenx4_app_traktor_reconfigure_displays(ctx);

    for (mod_num = 0; mod_num < ctx->num_modules; ++mod_num)
    {
    	if (ctx->mod_num_to_node_id[mod_num] != 0)
    	{
    		cenx4_app_traktor_update_slave_display(ctx, ctx->mod_num_to_node_id[mod_num]);
    	}
    }

    return;

err_bad_cfg:
	phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
}

void cenx4_app_traktor_stop(void * ctx)
{

}

void cenx4_app_traktor_tick(void * _ctx)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
	cenx4_ui_t * ui;

	switch (ctx->mode)
	{
	case CENX4_APP_TRAKTOR_MODE_DEFAULT:
		ui = cenx4_ui_lock(0);
		ui->state.split_pot.pots[0].val = ctx->master_level;
		ui->state.split_pot.pots[1].val = ctx->monitor_level;
		cenx4_ui_unlock(ui);
		break;

	case CENX4_APP_TRAKTOR_MODE_BROWSE:
		break;
	}
}

void cenx4_app_traktor_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
    phi_midi_pkt_t pkt;

    const uint8_t encoder_to_cc_map[][CENX4_MAX_POTS] = {
    	// CENX4_APP_TRAKTOR_MODE_DEFAULT
    	{
    		CENX4_APP_TRAKTOR_MIDI_CC_MASTER_LEVEL,
    		CENX4_APP_TRAKTOR_MIDI_CC_MONITOR_LEVEL,
    		CENX4_APP_TRAKTOR_MIDI_CC_UNUSED,
    		CENX4_APP_TRAKTOR_MIDI_CC_UNUSED,
    	},
		// CENX4_APP_TRAKTOR_MODE_BROWSE
		{
			CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_SELECT_PAGE,
			CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_SELECT,
			CENX4_APP_TRAKTOR_MIDI_CC_UNUSED,
			CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_TREE_SELECT,
		},
    };

    chDbgCheck(ctx->mode < PHI_ARRLEN(encoder_to_cc_map));

	if (node_id == 0)
	{
		pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER;
		pkt.event = 0xB; // Control Change
		pkt.val1 = encoder_to_cc_map[ctx->mode][encoder_num];
		pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset
		phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
	}
	else
	{
		uint8_t mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];
		if (mod_num != CENX4_APP_CFG_INVALID_MODULE_NUM)
		{
			pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER + 1 + mod_num;
			pkt.event = 0xB; // Control Change
			pkt.val1 = 20 + encoder_num;
			pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset
			phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
		}
	}
}

void cenx4_app_traktor_btn_event(void * _ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
    phi_midi_pkt_t pkt;

	if (node_id == 0)
	{
		switch (btn_num)
		{
		// Browse mode (toggle)
		case 1:
			if (event == PHI_BTN_EVENT_PRESSED)
			{
				pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER;
				pkt.event = 0xB; // Control Change
				pkt.val1 = CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_MODE;
				pkt.val2 = 1;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
			}
		    break;

		// Shift1
		case 2:
			if (event == PHI_BTN_EVENT_PRESSED)
			{
				ctx->shift1_mode = TRUE;

				pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER;
				pkt.event = 0xB; // Control Change
				pkt.val1 = CENX4_APP_TRAKTOR_MIDI_CC_SHIFT1;
				pkt.val2 = 1;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
			}
			else if (event == PHI_BTN_EVENT_RELEASED)
			{
				ctx->shift1_mode = FALSE;

				pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER;
				pkt.event = 0xB; // Control Change
				pkt.val1 = CENX4_APP_TRAKTOR_MIDI_CC_SHIFT1;
				pkt.val2 = 0;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
			}
			break;

		case 3:
			switch (ctx->mode)
			{
			case CENX4_APP_TRAKTOR_MODE_DEFAULT:
				break;

			case CENX4_APP_TRAKTOR_MODE_BROWSE:
				pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER;
				pkt.event = 0xB; // Control Change
				pkt.val1 = CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_TREE_EXPAND;
				pkt.val2 = 1;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
				break;
			}
		}
	}
	else
	{
		chDbgCheck(node_id >= PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID);

		uint8_t mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];
		if (mod_num != CENX4_APP_CFG_INVALID_MODULE_NUM)
		{
			pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER + 1 + mod_num;
			pkt.event = 0xB; // Control Change
			pkt.val1 = btn_num + 1;
			switch (event)
			{
			case PHI_BTN_EVENT_PRESSED:
				pkt.val2 = 0x7f;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
				break;

			case PHI_BTN_EVENT_RELEASED:
				pkt.val2 = 0;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
				break;

			default:
				break;
			}
		}
	}
}

void cenx4_app_traktor_pot_event(void * _ctx, uint8_t node_id, uint8_t pot_num, uint8_t val)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
	uint8_t mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];
	phi_midi_pkt_t pkt;

	chDbgCheck(node_id >= PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID);

	if (mod_num != CENX4_APP_CFG_INVALID_MODULE_NUM)
	{
		pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER + 1 + mod_num;
		pkt.event = 0xB; // Control Change
		pkt.val1 = pot_num + 30;
		pkt.val2 = val >> 1;
		phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
	}
}

void cenx4_app_traktor_midi_cc(void * _ctx, phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
    uint8_t node_id;

    switch (ch)
    {
    case CENX4_APP_TRAKTOR_MIDI_CH_MASTER:
    	switch (cc)
    	{
    	case CENX4_APP_TRAKTOR_MIDI_CC_MASTER_VU_LEFT:
    		ctx->master_l = val << 1;
    		break;

    	case CENX4_APP_TRAKTOR_MIDI_CC_MASTER_VU_RIGHT:
			ctx->master_r = val << 1;
			break;

    	case CENX4_APP_TRAKTOR_MIDI_CC_MASTER_LEVEL:
    		ctx->master_level = val << 1;
    		break;

    	case CENX4_APP_TRAKTOR_MIDI_CC_MONITOR_LEVEL:
    		ctx->monitor_level = val << 1;
    		break;

    	case CENX4_APP_TRAKTOR_MIDI_CC_BROWSE_MODE:
    		ctx->mode = val ? CENX4_APP_TRAKTOR_MODE_BROWSE : CENX4_APP_TRAKTOR_MODE_DEFAULT;
    		cenx4_app_traktor_reconfigure_displays(ctx);
    		break;
    	}
    	break;

	case CENX4_APP_TRAKTOR_MIDI_CH_MASTER + 1:
	case CENX4_APP_TRAKTOR_MIDI_CH_MASTER + 2:
	case CENX4_APP_TRAKTOR_MIDI_CH_MASTER + 3:
	case CENX4_APP_TRAKTOR_MIDI_CH_MASTER + 4:
		{
			const uint8_t mod_num = ch - CENX4_APP_TRAKTOR_MIDI_CH_MASTER - 1;
			node_id = ctx->mod_num_to_node_id[mod_num];
			if (node_id == 0xff)
			{
				break;
			}

			// CC 1 to 8 are button LEDs
			if ((cc >= 1) && (cc <= 8))
			{
				hyperion_app_slave_msg_set_btn_led_t set_btn_led = {
					.btn_num = cc - 1,
					.led = val,
				};

				phi_can_xfer(
					&cenx4_can,
					PHI_CAN_PRIO_LOWEST,
					PHI_CAN_MSG_ID_HYPERION_SET_BTN_LED,
					node_id,
					(const uint8_t *) &set_btn_led,
					sizeof(set_btn_led),
					NULL,
					0,
					NULL,
					PHI_CAN_DEFAULT_TIMEOUT
				);
			}

			// CC 20 and 21 are the encoders
			if (cc == 20)
			{
				ctx->mod_num_to_gain[mod_num] = val * 2;
				cenx4_app_traktor_update_slave_display(ctx, node_id);
			}

			// CC 60 is bar graph
			if (cc == 60)
			{
				hyperion_app_slave_msg_set_led_bar_t led_bar = {
					.val = phi_lib_map(val, 0, 127, 0, 10),
					.fill = 1,
				};

				phi_can_xfer(
					&cenx4_can,
					PHI_CAN_PRIO_LOWEST,
					PHI_CAN_MSG_ID_HYPERION_SET_LED_BAR,
					node_id,
					(const uint8_t *) &led_bar,
					sizeof(led_bar),
					NULL,
					0,
					NULL,
					PHI_CAN_DEFAULT_TIMEOUT
				);
			}
		}
		break;
    }
}

void cenx4_app_traktor_reconfigure_displays(cenx4_app_traktor_context_t * ctx)
{
	cenx4_ui_t * ui;

	/* Move display 0 */
	ui = cenx4_ui_lock(0);

	switch (ctx->mode)
	{
	case CENX4_APP_TRAKTOR_MODE_DEFAULT:
		ui->dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

		memset(&(ui->state), 0, sizeof(ui->state));

		ui->state.split_pot.pots[0].flags =
				CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
				CENX4_UI_DISPMODE_POT_FLAGS_FILL |
				CENX4_UI_DISPMODE_POT_FLAGS_RENDER_VAL |
				CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
		chsnprintf(ui->state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Master");
		ui->state.split_pot.pots[0].val = ctx->master_level;

		ui->state.split_pot.pots[1].flags = ui->state.split_pot.pots[0].flags;
		chsnprintf(ui->state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Headphones");
		ui->state.split_pot.pots[1].val = ctx->monitor_level;
		break;

	case CENX4_APP_TRAKTOR_MODE_BROWSE:
		ui->state.callback.func = cenx4_app_traktor_render_browse;
		ui->state.callback.ctx = ctx;
		ui->dispmode = CENX4_UI_DISPMODE_CALLBACK;
		break;

	default:
		chDbgCheck(FALSE);
	}

	cenx4_ui_unlock(ui);

	/* Move display 1 into custom mode for displaying master VU meters */
	ui = cenx4_ui_lock(1);
	memset(&(ui->state), 0, sizeof(ui->state));
	ui->state.callback.func = cenx4_app_traktor_render_custom_1;
	ui->state.callback.ctx = ctx;
	ui->dispmode = CENX4_UI_DISPMODE_CALLBACK;
	cenx4_ui_unlock(ui);
}

void cenx4_app_traktor_render_browse(cenx4_ui_t * ui, void * _ctx)
{
//    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;

    gdispGClear(ui->g, Black);
	cenx4_ui_text(ui, 0, 0, ui->w, 1, justifyCenter, "Browse");
}

void vertical_vu_helper(cenx4_ui_t * ui, uint8_t n, uint8_t val)
{
	const uint8_t max_vus = 2;
	const coord_t vu_width = 8;
	const coord_t vu_height = ui->h - 10;
	const coord_t space_between_vus = 5;
	const coord_t vu_bar_pad = 3;
	const coord_t vu_bar_max_height = vu_height - (vu_bar_pad * 2);
	const coord_t vu_bar_width = vu_width - 4;
	const coord_t vu_bar_x_offset = (vu_width - vu_bar_width) / 2;
	const coord_t vu_bar_y_offset = (vu_height - vu_bar_max_height) / 2;

	// Max space used by VUs
	const coord_t vus_total_width = (vu_width * max_vus) + (space_between_vus * (max_vus - 1));

	// Where do we start drawing VUs
	const coord_t first_vu_x = (ui->w - vus_total_width) / 2;

	const coord_t vu_x = first_vu_x + ((vus_total_width + space_between_vus) * n);
	const coord_t vu_y = (ui->h - vu_height) / 2;
	const coord_t vu_bar_height = phi_lib_map(val, 0, 0xff, 0, vu_bar_max_height);

	chDbgCheck(n < max_vus);

	gdispGDrawBox(ui->g, vu_x, vu_y, vu_width, vu_height, White);
	gdispGFillArea(ui->g, vu_x + vu_bar_x_offset, vu_y + vu_bar_y_offset + (vu_bar_max_height - vu_bar_height), vu_bar_width, vu_bar_height, White);
}

void cenx4_app_traktor_render_custom_1(cenx4_ui_t * ui, void * _ctx)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;

    gdispGClear(ui->g, Black);

    if (ctx->shift1_mode)
    {
    	cenx4_ui_text(ui, 0, 0, ui->w, 1, justifyCenter, "CUE");
    }
    else
    {
    	vertical_vu_helper(ui, 0, ctx->master_l);
    	vertical_vu_helper(ui, 1, ctx->master_r);
    }
}

msg_t cenx4_app_traktor_update_slave_display(cenx4_app_traktor_context_t * ctx, uint8_t node_id)
{
	hyperion_app_slave_msg_update_display_state_t state;
	const uint8_t mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];

	memset(&state, 0, sizeof(state));
	state.dispmode = HYPERION_UI_DISPMODE_SPLIT_POT;

	state.state.split_pot.pots[0].flags =
		HYPERION_UI_DISPMODE_POT_FLAGS_FILL |
		HYPERION_UI_DISPMODE_POT_FLAGS_RENDER_VAL;
	chsnprintf(state.state.split_pot.pots[0].text_top, HYPERION_UI_MAX_LINE_TEXT_LEN - 1, "Gain");
	state.state.split_pot.pots[0].val = ctx->mod_num_to_gain[mod_num];

	state.state.split_pot.pots[1].flags = state.state.split_pot.pots[0].flags;
	chsnprintf(state.state.split_pot.pots[1].text_top, HYPERION_UI_MAX_LINE_TEXT_LEN - 1, "");
	state.state.split_pot.pots[1].val = 0;

	chsnprintf(state.state.split_pot.title, sizeof(state.state.split_pot.title) - 1, "Deck %c", 'A' + mod_num);

	return phi_can_xfer(
		&cenx4_can,
		PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_ID_HYPERION_UPDATE_DISPLAY,
		node_id,
		(const uint8_t *) &state,
		sizeof(state),
		NULL,
		0,
		NULL,
		PHI_CAN_DEFAULT_TIMEOUT
	);
}
