#include "cenx4_app_cfg.h"
#include "cenx4_app_registry.h"
#include "cenx4_app_setup.h"
#include "cenx4_can.h"


const phi_app_desc_t cenx4_app_setup_desc = {
    .start = cenx4_app_setup_start,
    .stop = cenx4_app_setup_stop,
    .encoder_event = cenx4_app_setup_encoder_event,
	.btn_event = cenx4_app_setup_btn_event,
};

void cenx4_app_setup_start(void * _ctx)
{
	cenx4_ui_t * ui;
    cenx4_app_setup_context_t * ctx = (cenx4_app_setup_context_t *) _ctx;
    uint32_t i, mod_num;
    uint8_t our_uid[PHI_CAN_AUTO_ID_UNIQ_ID_LEN];
    uint8_t node_id;

#warning MEGA HACK TODO
    cenx4_can.auto_alloc_num_devs = 1;
    cenx4_can.auto_alloc_table[0][0] = 0x59;
    cenx4_can.auto_alloc_table[0][1] = 0x52;
    cenx4_can.auto_alloc_table[0][2] = 0x45;
	cenx4_can.auto_alloc_table[0][3] = 0x42;
	cenx4_can.auto_alloc_table[0][4] = 0x2f;
	cenx4_can.auto_alloc_table[0][5] = 0x00;
	cenx4_can.auto_alloc_table[0][6] = 0x33;
	cenx4_can.auto_alloc_table[0][7] = 0x00;
	cenx4_can.auto_alloc_table[0][8] = 0x0d;
	cenx4_can.auto_alloc_table[0][9] = 0x51;
	cenx4_can.auto_alloc_table[0][10] = 0x4d;
	cenx4_can.auto_alloc_table[0][11] = 0x43;
	cenx4_can.auto_alloc_table[0][12] = 0x34;
	cenx4_can.auto_alloc_table[0][13] = 0x38;
	cenx4_can.auto_alloc_table[0][14] = 0x33;
	cenx4_can.auto_alloc_table[0][15] = 0x20;

    // Basic init
    chDbgCheck(ctx != NULL);
    chDbgCheck(cenx4_can.node_id == PHI_CAN_AUTO_ID_ALLOCATOR_NODE); // setup app can only run on allocator
    memset(ctx, 0, sizeof(*ctx));

    for (i = 0; i < PHI_ARRLEN(ctx->test_pot_val); ++i)
    {
    	ctx->test_pot_val[i] = (i + 1) * 10;
    }

    // Load mod_num_to_uid from cfg
    phi_can_auto_get_dev_uid(&cenx4_can, our_uid);
    for (mod_num = 0; mod_num < PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1; ++mod_num)
    {
    	// If stored UID matches ours, set node id 0
    	if (memcmp(
    		&(cenx4_app_cfg.cur.mod_num_to_uid[mod_num][0]),
			&(our_uid[0]),
			PHI_CAN_AUTO_ID_UNIQ_ID_LEN) == 0)
    	{
    		ctx->node_id_to_mod_num[0] = mod_num;
    		continue;
    	}

    	// Get the node id of the current module
    	for (node_id = 0; node_id < cenx4_can.auto_alloc_num_devs; ++node_id)
    	{
    		if (memcmp(
    			&(cenx4_app_cfg.cur.mod_num_to_uid[mod_num][0]),
				&(cenx4_can.auto_alloc_table[node_id][0]),
				PHI_CAN_AUTO_ID_UNIQ_ID_LEN) == 0)
    		{
    			ctx->node_id_to_mod_num[node_id + 1] = mod_num;
    			break;
    		}
    	}
    }

	// Move our displays into setup mode - display 0
	ui = cenx4_ui_lock(0);

	ui->dispmode = CENX4_UI_DISPMODE_TEXTS;

	memset(&(ui->state), 0, sizeof(ui->state));

	ui->state.text.flags[0] = CENX4_UI_DISPMODE_TEXT_FLAGS_ALIGN_CENTER;
	ui->state.text.flags[3] = CENX4_UI_DISPMODE_TEXT_FLAGS_ALIGN_CENTER | CENX4_UI_DISPMODE_TEXT_FLAGS_BOTTOM;
	strcpy(ui->state.text.lines[0], "Setup");
	strcpy(ui->state.text.lines[1], "MASTER");

	cenx4_ui_unlock(ui);

	// Move our displays into setup mode - display 1
	ui = cenx4_ui_lock(1);

	ui->dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

	memset(&(ui->state), 0, sizeof(ui->state));

	ui->state.split_pot.pots[0].flags =
			CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
			CENX4_UI_DISPMODE_POT_FLAGS_FILL |
			CENX4_UI_DISPMODE_POT_FLAGS_RENDER_VAL |
			CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
	chsnprintf(ui->state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "ModNum");
	ui->state.split_pot.pots[0].val = ctx->node_id_to_mod_num[0];

	ui->state.split_pot.pots[1].flags = ui->state.split_pot.pots[0].flags;
	chsnprintf(ui->state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Test");
	ui->state.split_pot.pots[1].val = ctx->test_pot_val[0];

	cenx4_ui_unlock(ui);

	// Refresh
	cenx4_app_setup_berry_update_ui(ctx, 0);

    // Move our Berry modules into setup mode
	// TODO locks
	for (i = 0; i < cenx4_can.auto_alloc_num_devs; ++i)
	{
		cenx4_app_setup_berry_enter_setup_mode(ctx, PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + i);
	}
}

void cenx4_app_setup_stop(void * ctx)
{
    (void) ctx;
}

void cenx4_app_setup_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    cenx4_app_setup_context_t * ctx = (cenx4_app_setup_context_t *) _ctx;
    uint8_t idx = cenx4_app_setup_node_id_to_idx(node_id);
    chDbgCheck(ctx != NULL);

    // Action knob (on master)
    if ((node_id == 0) && (encoder_num == 1))
    {
    	ctx->cur_action = (uint8_t) (ctx->cur_action + val_change);
    	if (ctx->cur_action == 0xff)
		{
			ctx->cur_action = CENX4_APP_SETUP_NUM_ACTIONS - 1;
		}
		else if (ctx->cur_action >= CENX4_APP_SETUP_NUM_ACTIONS)
		{
			ctx->cur_action = 0;
		}
    }

    // Module number pot
    if (encoder_num == 2)
	{
		ctx->node_id_to_mod_num[idx] += val_change;
		if (ctx->node_id_to_mod_num[idx] == 0xff)
		{
			ctx->node_id_to_mod_num[idx] = cenx4_can.auto_alloc_num_devs;
		}
		else if (ctx->node_id_to_mod_num[idx] > cenx4_can.auto_alloc_num_devs)
		{
			ctx->node_id_to_mod_num[idx] = 0;
		}

		// Lame hack: when module number changes we want to update the main
		// module whether the change came from it or from one of the slaves
		if (node_id != 0)
		{
			cenx4_app_setup_berry_update_ui(ctx, 0);
		}
	}

    // Test pot
    if (encoder_num == 3)
    {
    	ctx->test_pot_val[idx] += val_change;
    }

    // Update UI
    cenx4_app_setup_berry_update_ui(ctx, node_id);
}

void cenx4_app_setup_btn_event(void * _ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
	cenx4_app_setup_context_t * ctx = (cenx4_app_setup_context_t *) _ctx;
	cenx4_ui_t * ui;
	int i;

	(void) param;

	// Action button (bottom left)
	if ((node_id == 0) && (btn_num == 1) && (event == PHI_BTN_EVENT_RELEASED))
	{
		switch (ctx->cur_action)
		{
		case CENX4_APP_SETUP_ACTION_EXIT:
			phi_app_mgr_switch_app(&cenx4_app_ableton_desc, &(cenx4_app_contexts.ableton));
			break;

		case CENX4_APP_SETUP_ACTION_SAVE:
			if (ctx->cfg_ok)
			{
				cenx4_app_cfg_t cfg;
				uint8_t uid[PHI_CAN_AUTO_ID_UNIQ_ID_LEN];

				cenx4_app_cfg_init_default(&cfg);

				phi_can_auto_get_dev_uid(&cenx4_can, uid);
				memcpy(
					&(cfg.cur.mod_num_to_uid[ctx->node_id_to_mod_num[0]]),
					uid,
					PHI_CAN_AUTO_ID_UNIQ_ID_LEN
				);

				for (i = 0; i < cenx4_can.auto_alloc_num_devs; ++i)
				{
					memcpy(
						&(cfg.cur.mod_num_to_uid[ctx->node_id_to_mod_num[i + 1]]),
						cenx4_can.auto_alloc_table[i],
						PHI_CAN_AUTO_ID_UNIQ_ID_LEN
					);
				}

				cenx4_app_cfg_save(&cfg);
			}
			else
			{
				for (i = 0; i < 3; ++i)
				{
					ui = cenx4_ui_lock(0);
					strcpy(ui->state.text.lines[2], "Bad Cfg!");
					cenx4_ui_unlock(ui);
					chThdSleepMilliseconds(200);

					ui = cenx4_ui_lock(0);
					ui->state.text.lines[2][0] = 0;
					cenx4_ui_unlock(ui);
					chThdSleepMilliseconds(200);
				}

				cenx4_app_setup_berry_update_ui(ctx, 0);
			}

			break;

		case CENX4_APP_SETUP_ACTION_TEST:
			phi_app_mgr_switch_app(&cenx4_app_test_desc, &(cenx4_app_contexts.test));
			break;

        case CENX4_APP_SETUP_ACTION_BOOTLOADER:
            {
                const WDGConfig wdgcfg = {
                    STM32_IWDG_PR_64,
                    STM32_IWDG_RL(1),
                    STM32_IWDG_WIN_DISABLED
                };

                wdgStart(&WDGD1, &wdgcfg);
            }
            break;

		default:
			break;
		}
	}
}

msg_t cenx4_app_setup_berry_enter_setup_mode(cenx4_app_setup_context_t * ctx, uint8_t node_id)
{
    msg_t ret;

    cenx4_can_handle_set_dispmode_t dispmode;

    // Put display 0 in boot mode
    dispmode.disp = 0;
    dispmode.dispmode = CENX4_UI_DISPMODE_BOOT;

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

    // Put display 1 in pots mode
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

    // Set pots
    ret = cenx4_app_setup_berry_update_ui(ctx, node_id);
    if (MSG_OK != ret)
    {
        return ret;
    }

    // Success
    return MSG_OK;
}

msg_t cenx4_app_setup_berry_update_ui(cenx4_app_setup_context_t * ctx, uint8_t node_id)
{
    cenx4_can_handle_set_dispmode_state_t state;
    msg_t ret;
    uint8_t idx = cenx4_app_setup_node_id_to_idx(node_id);
    cenx4_ui_t * ui;

    if (node_id == 0)
    {
    	const char * actions[] = {
    		"Exit",
			"Save",
			"Test",
			"BL",
    	};

    	ui = cenx4_ui_lock(0);

    	// See if configuration is valid or not
    	{
    		int i, j, cnt;

    		// Assume everything is okay
    		ctx->cfg_ok = true;

    		// This code checks if we have exactly one of each module number
    		for (i = 0; i < (cenx4_can.auto_alloc_num_devs + 1); ++i)
    		{
    			cnt = 0;
    			for (j = 0; j < (cenx4_can.auto_alloc_num_devs + 1); ++j)
    			{
    				if (ctx->node_id_to_mod_num[j] == i)
    				{
    					cnt++;
    				}
    			}

    			if (cnt == 0)
    			{
    				chsnprintf(ui->state.text.lines[2], CENX4_UI_MAX_LINE_TEXT_LEN-1, "Mod%d miss", i);
    				ctx->cfg_ok = false;
    				break;
    			}

    			if (cnt > 1)
    			{
    				chsnprintf(ui->state.text.lines[2], CENX4_UI_MAX_LINE_TEXT_LEN-1, "Mod%d dupe", i);
    				ctx->cfg_ok = false;
    				break;
    			}
    		}

    		if (ctx->cfg_ok)
    		{
    			chsnprintf(ui->state.text.lines[2], CENX4_UI_MAX_LINE_TEXT_LEN-1, "ModCfg OK");
    		}
    	}


    	// Action button
    	strcpy(ui->state.text.lines[3], actions[ctx->cur_action]);

    	cenx4_ui_unlock(ui);

    	ui = cenx4_ui_lock(1);
		ui->state.split_pot.pots[0].val = ctx->node_id_to_mod_num[0];
		ui->state.split_pot.pots[1].val = ctx->test_pot_val[0];
		cenx4_ui_unlock(ui);
    }
    else
    {
		memset(&state, 0, sizeof(state));
		state.disp = 1;

		state.state.split_pot.pots[0].flags =
			CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
			CENX4_UI_DISPMODE_POT_FLAGS_FILL |
			CENX4_UI_DISPMODE_POT_FLAGS_RENDER_VAL |
			CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
		chsnprintf(state.state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "ModNum");
		state.state.split_pot.pots[0].val = ctx->node_id_to_mod_num[idx];

		state.state.split_pot.pots[1].flags = state.state.split_pot.pots[0].flags;
		chsnprintf(state.state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Test");
		state.state.split_pot.pots[1].val = ctx->test_pot_val[idx];

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
    }

    return MSG_OK;
}
