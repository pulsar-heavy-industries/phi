#include "hyperion_app_test.h"
#include "hyperion_ui.h"
#include "hyperion_io.h"
#include "hyperion_can.h"


const phi_app_desc_t hyperion_app_test_desc = {
    .start = hyperion_app_test_start,
    .stop = hyperion_app_test_stop,
	.tick = hyperion_app_test_tick,
	.tick_interval = MS2ST(10),
	.encoder_event = hyperion_app_test_encoder_event,
	.btn_event = hyperion_app_test_btn_event,
	.pot_event = hyperion_app_test_pot_event,
};

void hyperion_app_test_start(void * _ctx)
{
    hyperion_app_test_context_t * ctx = (hyperion_app_test_context_t *) _ctx;
    hyperion_ui_t * ui;
//    uint8_t i;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    ui = hyperion_ui_lock();
    ui->dispmode = HYPERION_UI_DISPMODE_TEXTS;
    memset(&(ui->state), 0, sizeof(ui->state));
    ui->state.text.flags[0] = HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL | HYPERION_UI_DISPMODE_TEXT_FLAGS_ALIGN_CENTER;
    ui->state.text.flags[1] = HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;
    ui->state.text.flags[2] = HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;
    ui->state.text.flags[3] = HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;
    ui->state.text.flags[4] = HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;
    strcpy(ui->state.text.lines[0], "Hyperion Test");
    hyperion_ui_unlock(ui);

    hyperion_io_force_pots_read();
}

void hyperion_app_test_stop(void * ctx)
{
    (void) ctx;
}

void hyperion_app_test_tick(void * _ctx)
{
    hyperion_app_test_context_t * ctx = (hyperion_app_test_context_t *) _ctx;
    hyperion_ui_t * ui;
    systime_t ping;
    systime_t now = chVTGetSystemTime();
    char buf[HYPERION_UI_MAX_LINE_TEXT_LEN];
    msg_t ret;

    if ((now - ctx->last_updated) > MS2ST(100))
    {
        ctx->cnt++;
        hyperion_io_set_bar_graph(ctx->cnt, ctx->fill);
        if (ctx->cnt == 10)
        {
            ctx->fill = ctx->fill ? 0 : 1;
            ctx->cnt = 0;
        }
        ctx->last_updated = now;
    }

    ping = chVTGetSystemTimeX();
	ret = phi_can_xfer(
		&hyperion_can,
		PHI_CAN_PRIO_LOWEST,
		PHI_CAN_MSG_ID_ECHO,
		PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
		(const uint8_t *) &ping,
		sizeof(ping),
		(uint8_t *) &ping,
		sizeof(ping),
		NULL,
		PHI_CAN_DEFAULT_TIMEOUT);
	switch (ret)
	{
	case MSG_RESET:
		strcpy(buf, "P: RST");
		break;

	case MSG_TIMEOUT:
		strcpy(buf, "P: Timeout");

		break;

	case MSG_OK:
		chsnprintf(buf, sizeof(buf) - 1, "P: %2.2fms", ((float)ST2US(chVTGetSystemTimeX() - ping)) / 1000.0f);
		break;

	default:
		chDbgAssert(FALSE, "WTF");
		break;
	}

    ui = hyperion_ui_lock();
    chsnprintf(ui->state.text.lines[1], HYPERION_UI_MAX_LINE_TEXT_LEN, "Rot %d %d", ctx->rotencs[0], ctx->rotencs[1]);
    chsnprintf(ui->state.text.lines[2], HYPERION_UI_MAX_LINE_TEXT_LEN, "%d %d %d %d", ctx->pots[0], ctx->pots[1], ctx->pots[2], ctx->pots[3]);
    chsnprintf(ui->state.text.lines[3], HYPERION_UI_MAX_LINE_TEXT_LEN, "%d %d %d %d", ctx->pots[4], ctx->pots[5], ctx->pots[6], ctx->pots[7]);
    strcpy(ui->state.text.lines[4], buf);
    hyperion_ui_unlock(ui);

}

void hyperion_app_test_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
	hyperion_app_test_context_t * ctx = (hyperion_app_test_context_t *) _ctx;

	ctx->rotencs[encoder_num] += val_change;

}
void hyperion_app_test_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
    (void) param;

    if (btn_num >= 8)
    {
        btn_num -= 8;

        switch (event)
        {
        case PHI_BTN_EVENT_PRESSED:
            hyperion_io_set_btn_leds(btn_num, HYPERION_IO_BTN_LED_ORANGE);
            break;

        case PHI_BTN_EVENT_HELD:
            hyperion_io_set_btn_leds(btn_num, HYPERION_IO_BTN_LED_GREEN);
            break;

        case PHI_BTN_EVENT_RELEASED:
            hyperion_io_set_btn_leds(btn_num, HYPERION_IO_BTN_LED_OFF);
            break;

        default:
            break;
        }
    }
    else
    {
        switch (event)
        {
        case PHI_BTN_EVENT_PRESSED:
            hyperion_io_set_btn_leds(btn_num, HYPERION_IO_BTN_LED_RED);
            break;

        case PHI_BTN_EVENT_HELD:
            hyperion_io_set_btn_leds(btn_num, HYPERION_IO_BTN_LED_GREEN);
            break;

        case PHI_BTN_EVENT_RELEASED:
            hyperion_io_set_btn_leds(btn_num, HYPERION_IO_BTN_LED_OFF);
            break;

        default:
            break;
        }
    }
}

void hyperion_app_test_pot_event(void * _ctx, uint8_t node_id, uint8_t pot_num, uint8_t val)
{
    hyperion_app_test_context_t * ctx = (hyperion_app_test_context_t *) _ctx;
    ctx->pots[pot_num] = val;
}
