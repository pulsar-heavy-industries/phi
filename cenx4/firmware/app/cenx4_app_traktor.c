#include "cenx4_app_traktor.h"


const phi_app_desc_t cenx4_app_traktor_desc = {
    .start = cenx4_app_traktor_start,
    .stop = cenx4_app_traktor_stop,
	.tick = cenx4_app_traktor_tick,
	.tick_interval = MS2ST(10),
    .encoder_event = cenx4_app_traktor_encoder_event,
	.btn_event = cenx4_app_traktor_btn_event,
	.midi_cc = cenx4_app_traktor_midi_cc,
};


void cenx4_app_traktor_start(void * _ctx)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
	cenx4_ui_t * ui;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

	/* Move display 0 into dual pot mode for master/headphone volumes */
	ui = cenx4_ui_lock(0);

	ui->dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

	memset(&(ui->state), 0, sizeof(ui->state));

	ui->state.split_pot.pots[0].flags =
			CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
			CENX4_UI_DISPMODE_POT_FLAGS_FILL |
			CENX4_UI_DISPMODE_POT_FLAGS_RENDER_VAL |
			CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
	chsnprintf(ui->state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Master");
	ui->state.split_pot.pots[0].val = 20;

	ui->state.split_pot.pots[1].flags = ui->state.split_pot.pots[0].flags;
	chsnprintf(ui->state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Headphones");
	ui->state.split_pot.pots[1].val = 20;

	cenx4_ui_unlock(ui);

	/* Move display 1 into custom mode for displaying master VU meters */
    ui = cenx4_ui_lock(1);
    memset(&(ui->state), 0, sizeof(ui->state));
    ui->state.callback.func = cenx4_app_traktor_render_custom;
    ui->state.callback.ctx = ctx;
    ui->dispmode = CENX4_UI_DISPMODE_CALLBACK;
    cenx4_ui_unlock(ui);
}

void cenx4_app_traktor_stop(void * ctx)
{

}

void cenx4_app_traktor_tick(void * _ctx)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;
	cenx4_ui_t * ui;

    ui = cenx4_ui_lock(0);
    ui->state.split_pot.pots[0].val = ctx->master_level;
    ui->state.split_pot.pots[1].val = ctx->monitor_level;
    cenx4_ui_unlock(ui);
}

void cenx4_app_traktor_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    phi_midi_pkt_t pkt;

	if (node_id == 0)
	{
		switch (encoder_num)
		{
		// Master
		case 0:
		    pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER;
		    pkt.event = 0xB; // Control Change
		    pkt.val1 = CENX4_APP_TRAKTOR_MIDI_CC_MASTER_LEVEL;
		    pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset
		    phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
			break;

		// Monitor
		case 1:
		    pkt.chn = CENX4_APP_TRAKTOR_MIDI_CH_MASTER;
		    pkt.event = 0xB; // Control Change
		    pkt.val1 = CENX4_APP_TRAKTOR_MIDI_CC_MONITOR_LEVEL;
		    pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset
		    phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
			break;
		}
	}
}

void cenx4_app_traktor_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{

}

void cenx4_app_traktor_midi_cc(void * _ctx, phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;

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
    	}
    	break;
    }
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

void cenx4_app_traktor_render_custom(cenx4_ui_t * ui, void * _ctx)
{
    cenx4_app_traktor_context_t * ctx = (cenx4_app_traktor_context_t *) _ctx;

    gdispGClear(ui->g, Black);
    vertical_vu_helper(ui, 0, ctx->master_l);
    vertical_vu_helper(ui, 1, ctx->master_r);
}
