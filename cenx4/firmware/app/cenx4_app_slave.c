#include "cenx4_app_slave.h"


const phi_app_desc_t cenx4_app_slave_desc = {
    .start = cenx4_app_slave_start,
    .stop = cenx4_app_slave_stop,
    .encoder_event = cenx4_app_slave_encoder_event,
	.btn_event = cenx4_app_slave_btn_event,
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
}

void cenx4_app_slave_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
}
