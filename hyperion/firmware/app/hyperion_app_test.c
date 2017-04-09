#include "hyperion_app_test.h"
#include "hyperion_ui.h"
#include "hyperion_io.h"


const phi_app_desc_t hyperion_app_test_desc = {
    .start = hyperion_app_test_start,
    .stop = hyperion_app_test_stop,
	.tick = hyperion_app_test_tick,
	.tick_interval = MS2ST(10),
	.encoder_event = hyperion_app_test_encoder_event,
	.btn_event = hyperion_app_test_btn_event,
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
    strcpy(ui->state.text.lines[0], "Hyperion Test");
    hyperion_ui_unlock(ui);

}

void hyperion_app_test_stop(void * ctx)
{
    (void) ctx;
}

void hyperion_app_test_tick(void * _ctx)
{
    hyperion_app_test_context_t * ctx = (hyperion_app_test_context_t *) _ctx;
    hyperion_ui_t * ui;
    extern uint8_t adc_data_lowres[];
    systime_t now = chVTGetSystemTime();

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

    ui = hyperion_ui_lock();
    chsnprintf(ui->state.text.lines[1], HYPERION_UI_MAX_LINE_TEXT_LEN, "Rot %d %d", ctx->rotencs[0], ctx->rotencs[1]);
    chsnprintf(ui->state.text.lines[2], HYPERION_UI_MAX_LINE_TEXT_LEN, "%d %d %d %d", adc_data_lowres[0], adc_data_lowres[1], adc_data_lowres[2], adc_data_lowres[3]);
    chsnprintf(ui->state.text.lines[3], HYPERION_UI_MAX_LINE_TEXT_LEN, "%d %d %d %d", adc_data_lowres[4], adc_data_lowres[5], adc_data_lowres[6], adc_data_lowres[7]);
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
