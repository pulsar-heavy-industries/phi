#include "cenx4_app_ableton2.h"
#include "cenx4_app_registry.h"
#include "cenx4_conf.h"
#include "cenx4_midi.h"


const phi_app_desc_t cenx4_app_ableton2_desc = {
    .start = cenx4_app_ableton2_start,
    .stop = cenx4_app_ableton2_stop,
	.tick = cenx4_app_ableton2_tick,
	.tick_interval = MS2ST(10),
    .encoder_event = cenx4_app_ableton2_encoder_event,
	.btn_event = cenx4_app_ableton2_btn_event,
	.pot_event = cenx4_app_ableton2_pot_event,
	.midi_cc = cenx4_app_ableton2_midi_cc,
	.midi_sysex = cenx4_app_ableton2_midi_sysex,
};


void cenx4_app_ableton2_start(void * _ctx)
{
    cenx4_app_ableton2_context_t * ctx = (cenx4_app_ableton2_context_t *) _ctx;
    uint8_t i;
    uint8_t mod_num;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    strcpy(ctx->ui_default.text_top, "WaitHost...");

    // Load mod_num_to_uid from cfg
	cenx4_app_cfg_get_node_id_to_mod_num_map(
		HYPERION_DEV_ID,
		ctx->node_id_to_mod_num,
		PHI_ARRLEN(cenx4_app_cfg.cur.hyperion_mod_num_to_uid)
	);

	cenx4_app_log("Ableton2!");

	// Init default hyperions state
	for (i = 0; i < CENX4_APP_CFG_MAX_MODULES; ++i)
	{
		strcpy(ctx->hyperions[i].title, "-");
	}

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
    cenx4_app_ableton2_reconfigure_displays(ctx);

    for (mod_num = 0; mod_num < ctx->num_modules; ++mod_num)
    {
    	if (ctx->mod_num_to_node_id[mod_num] != 0)
    	{
    		cenx4_app_ableton2_update_slave_display(ctx, ctx->mod_num_to_node_id[mod_num]);
    	}
    }

    // Notify Ableton we are ready, in case it's waiting for us
    cenx4_app_ableton2_send_resync(ctx);

    return;

err_bad_cfg:
	phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
}

void cenx4_app_ableton2_stop(void * ctx)
{

}

void cenx4_app_ableton2_tick(void * _ctx)
{
    cenx4_app_ableton2_context_t * ctx = (cenx4_app_ableton2_context_t *) _ctx;
//	 cenx4_ui_t * ui;

	switch (ctx->mode)
	{
	case CENX4_APP_ABLETON2_MODE_DEFAULT:
//		ui = cenx4_ui_lock(0);
//		ui->state.split_pot.pots[0].val = ctx->master_level;
//		ui->state.split_pot.pots[1].val = ctx->monitor_level;
//		cenx4_ui_unlock(ui);
		break;
	}
}

void cenx4_app_ableton2_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    cenx4_app_ableton2_context_t * ctx = (cenx4_app_ableton2_context_t *) _ctx;
    phi_midi_pkt_t pkt;

	if (node_id == 0)
	{
		if (ctx->mode == CENX4_APP_ABLETON2_MODE_DEFAULT)
		{
			pkt.chn = CENX4_APP_ABLETON2_MIDI_CH_MASTER;
			pkt.event = 0xB; // Control Change
			pkt.val1 = 20 + encoder_num;
			pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset
			phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
		}
	}
	else
	{
		uint8_t mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];
		if (mod_num != CENX4_APP_CFG_INVALID_MODULE_NUM)
		{
			pkt.chn = CENX4_APP_ABLETON2_MIDI_CH_MASTER + 1 + mod_num;
			pkt.event = 0xB; // Control Change
			pkt.val1 = 20 + encoder_num;
			pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset
			phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
		}
	}
}

void cenx4_app_ableton2_btn_event(void * _ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
    cenx4_app_ableton2_context_t * ctx = (cenx4_app_ableton2_context_t *) _ctx;
    phi_midi_pkt_t pkt;

	if (node_id == 0)
	{
		if ((btn_num == 0) &&
			(event == PHI_BTN_EVENT_HELD) &&
			(param >= 5000))
		{
			phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
			return;
		}

		if (ctx->mode == CENX4_APP_ABLETON2_MODE_DEFAULT)
		{
			pkt.chn = CENX4_APP_ABLETON2_MIDI_CH_MASTER;
			pkt.event = 0xB; // Control Change
			pkt.val1 = btn_num + 1;
			switch (event)
			{
			case PHI_BTN_EVENT_PRESSED:
				pkt.val2 = 0x7f;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
				break;

			case PHI_BTN_EVENT_RELEASED:
				pkt.val2 = 0;
				phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
				break;

			default:
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
			pkt.chn = CENX4_APP_ABLETON2_MIDI_CH_MASTER + 1 + mod_num;
			pkt.event = 0xB; // Control Change
			pkt.val1 = btn_num + 1;

			// Fake toggle button for the white buttons
			if (btn_num < 8)
			{
				if (event == PHI_BTN_EVENT_PRESSED) {
					ctx->hyperions[mod_num].buttons_state[btn_num] = !ctx->hyperions[mod_num].buttons_state[btn_num];

					pkt.val2 = ctx->hyperions[mod_num].buttons_state[btn_num] ? 0x7f : 0;
					phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
				}
			}
			else
			{
				switch (event)
				{
				case PHI_BTN_EVENT_PRESSED:
					pkt.val2 = 0x7f;
					phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
					break;

				case PHI_BTN_EVENT_RELEASED:
					pkt.val2 = 0;
					phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
					break;

				default:
					break;
				}
			}
		}
	}
}

void cenx4_app_ableton2_pot_event(void * _ctx, uint8_t node_id, uint8_t pot_num, uint8_t val)
{
    cenx4_app_ableton2_context_t * ctx = (cenx4_app_ableton2_context_t *) _ctx;
	uint8_t mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];
	phi_midi_pkt_t pkt;

	chDbgCheck(node_id >= PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID);

	if (mod_num != CENX4_APP_CFG_INVALID_MODULE_NUM)
	{
		pkt.chn = CENX4_APP_ABLETON2_MIDI_CH_MASTER + 1 + mod_num;
		pkt.event = 0xB; // Control Change
		pkt.val1 = pot_num + 30;
		pkt.val2 = val >> 1;
		phi_midi_tx_pkt(PHI_MIDI_PORT_USB1, &pkt);
	}
}

void cenx4_app_ableton2_midi_cc(void * _ctx, phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val)
{
    cenx4_app_ableton2_context_t * ctx = (cenx4_app_ableton2_context_t *) _ctx;
    uint8_t node_id;

    switch (ch)
    {
    case CENX4_APP_ABLETON2_MIDI_CH_MASTER:
    	break;

	case CENX4_APP_ABLETON2_MIDI_CH_MASTER + 1:
	case CENX4_APP_ABLETON2_MIDI_CH_MASTER + 2:
	case CENX4_APP_ABLETON2_MIDI_CH_MASTER + 3:
	case CENX4_APP_ABLETON2_MIDI_CH_MASTER + 4:
		{
			const uint8_t mod_num = ch - CENX4_APP_ABLETON2_MIDI_CH_MASTER - 1;
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

				ctx->hyperions[mod_num].buttons_state[set_btn_led.btn_num] = set_btn_led.led ? 1 : 0;

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
#if 0
			if (cc == 20)
			{
				ctx->mod_num_to_gain[mod_num] = val * 2;
				cenx4_app_ableton2_update_slave_display(ctx, node_id);
			}

			if (cc == 21)
			{
				ctx->mod_num_to_loop_len[mod_num] = val;
				cenx4_app_ableton2_update_slave_display(ctx, node_id);
			}
#endif

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

void cenx4_app_ableton2_midi_sysex(void * _ctx, phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    cenx4_app_ableton2_context_t * ctx = (cenx4_app_ableton2_context_t *) _ctx;

	switch (cmd)
	{
	case CENX4_MIDI_SYSEX_APP_ABLETON2_SET_TITLE:
		if (data_len == sizeof(cenx4_app_ableton2_sysex_set_title_t))
		{
			cenx4_app_ableton2_midi_sysex_set_title(ctx, (const cenx4_app_ableton2_sysex_set_title_t *) data);
		}
		break;

	case CENX4_MIDI_SYSEX_APP_ABLETON2_UPDATE_UI_DEFAULT:
		if (data_len == sizeof(cenx4_app_ableton2_sysex_update_ui_default_t))
		{
			cenx4_app_ableton2_midi_sysex_update_ui_default(ctx, (const cenx4_app_ableton2_sysex_update_ui_default_t *) data);
		}
		break;

	default:
		break;
	}
}

void cenx4_app_ableton2_reconfigure_displays(cenx4_app_ableton2_context_t * ctx)
{
	cenx4_ui_t * ui;

	/* Move display 0 */
	ui = cenx4_ui_lock(0);
	memset(&(ui->state), 0, sizeof(ui->state));

	switch (ctx->mode)
	{
	case CENX4_APP_ABLETON2_MODE_DEFAULT:
		ui->state.callback.func = (void (*)(cenx4_ui_t *, void *)) cenx4_app_ableton2_render_default_left;
		ui->state.callback.ctx = ctx;
		ui->dispmode = CENX4_UI_DISPMODE_CALLBACK;
		break;

	default:
		chDbgCheck(FALSE);
	}

	cenx4_ui_unlock(ui);

	/* Move display 1 */
	ui = cenx4_ui_lock(1);
	memset(&(ui->state), 0, sizeof(ui->state));

	switch (ctx->mode)
	{
	case CENX4_APP_ABLETON2_MODE_DEFAULT:
		ui->dispmode = CENX4_UI_DISPMODE_LOGO;
		break;

	default:
		chDbgCheck(FALSE);
	}

	cenx4_ui_unlock(ui);
}

msg_t cenx4_app_ableton2_update_slave_display(cenx4_app_ableton2_context_t * ctx, uint8_t node_id)
{
	hyperion_app_slave_msg_update_display_state_t state;
	const uint8_t mod_num = ctx->node_id_to_mod_num[cenx4_app_cfg_get_mapped_node_id(node_id)];

	memset(&state, 0, sizeof(state));
	state.dispmode = HYPERION_UI_DISPMODE_SPLIT_POT;

	state.state.split_pot.pots[0].flags =
		HYPERION_UI_DISPMODE_POT_FLAGS_FILL |
		HYPERION_UI_DISPMODE_POT_FLAGS_RENDER_VAL;
	chsnprintf(state.state.split_pot.pots[0].text_top, HYPERION_UI_MAX_LINE_TEXT_LEN - 1, "Left");
	state.state.split_pot.pots[0].val = mod_num;

	state.state.split_pot.pots[1].flags = HYPERION_UI_DISPMODE_POT_FLAGS_FILL | HYPERION_UI_DISPMODE_POT_FLAGS_RENDER_VAL;
	chsnprintf(state.state.split_pot.pots[1].text_top, HYPERION_UI_MAX_LINE_TEXT_LEN - 1, "Right");
	state.state.split_pot.pots[1].val = mod_num * 10;

	strncpy(state.state.split_pot.title, ctx->hyperions[mod_num].title, sizeof(state.state.split_pot.title) - 1);

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

void cenx4_app_ableton2_midi_sysex_set_title(cenx4_app_ableton2_context_t * ctx, const cenx4_app_ableton2_sysex_set_title_t * data)
{
	if (data->mod_num >= CENX4_APP_CFG_MAX_MODULES)
	{
		return;
	}

	const uint8_t node_id = ctx->mod_num_to_node_id[data->mod_num];
	if (node_id == 0xff)
	{
		return;
	}

	strncpy(ctx->hyperions[data->mod_num].title, data->title, HYPERION_UI_MAX_LINE_TEXT_LEN - 1);

	cenx4_app_ableton2_update_slave_display(ctx, node_id);
}

void cenx4_app_ableton2_send_resync(cenx4_app_ableton2_context_t * ctx)
{
	phi_midi_tx_sysex(PHI_MIDI_PORT_USB1, CENX4_MIDI_SYSEX_APP_ABLETON2_RESYNC, NULL, 0);
}

void cenx4_app_ableton2_render_default_left(cenx4_ui_t * ui, cenx4_app_ableton2_context_t * ctx)
{
	chDbgCheck(ctx->mode == CENX4_APP_ABLETON2_MODE_DEFAULT);
	cenx4_ui_text(ui, 0, 0, ui->w, 0, justifyCenter, ctx->ui_default.text_top);
	cenx4_ui_text(ui, 0, gdispGGetHeight(ui->g) - 10, ui->w, 0, justifyCenter, ctx->ui_default.text_bottom);
}

void cenx4_app_ableton2_midi_sysex_update_ui_default(cenx4_app_ableton2_context_t * ctx, const cenx4_app_ableton2_sysex_update_ui_default_t * data)
{
	if ((data->text_top[sizeof(data->text_top) - 1] != 0) ||
		(data->text_bottom[sizeof(data->text_bottom) - 1] != 0)) {
		return;
	}

	strcpy(ctx->ui_default.text_top, data->text_top);
	strcpy(ctx->ui_default.text_bottom, data->text_bottom);
}
