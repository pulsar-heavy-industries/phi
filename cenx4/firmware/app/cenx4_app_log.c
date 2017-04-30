#include "cenx4_app_log.h"
#include "cenx4_app_setup.h"
#include "cenx4_app_registry.h"


char cenx4_app_log_buf[CENX4_APP_LOG_MAX_MSGS][CENX4_APP_LOG_MAX_MSG_LEN] = {{0, }, };
uint32_t cenx4_app_log_write_idx = 0;

const phi_app_desc_t cenx4_app_log_viewer_desc = {
    .start = cenx4_app_log_viewer_start,
    .stop = cenx4_app_log_viewer_stop,
    .encoder_event = cenx4_app_log_viewer_encoder_event,
	.btn_event = cenx4_app_log_viewer_btn_event,
};

void cenx4_app_log(const char * msg)
{
	uint32_t len = strlen(msg);
	memcpy(cenx4_app_log_buf[cenx4_app_log_write_idx % CENX4_APP_LOG_MAX_MSGS], msg, phi_min(len, CENX4_APP_LOG_MAX_MSG_LEN));
	cenx4_app_log_write_idx++;
}

void cenx4_app_log_viewer_start(void * _ctx)
{
    cenx4_app_log_viewer_context_t * ctx = (cenx4_app_log_viewer_context_t *) _ctx;
	cenx4_ui_t * ui;

    // Basic init
    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));
    ctx->read_idx = cenx4_app_log_write_idx;

    ui = cenx4_ui_lock(0);
    memset(&(ui->state), 0, sizeof(ui->state));
    ui->state.callback.func = cenx4_app_log_render;
    ui->state.callback.ctx = ctx;
    ui->dispmode = CENX4_UI_DISPMODE_CALLBACK;
    cenx4_ui_unlock(ui);

    ui = cenx4_ui_lock(1);
    memset(&(ui->state), 0, sizeof(ui->state));
    ui->dispmode = CENX4_UI_DISPMODE_BOOT;
    cenx4_ui_unlock(ui);
}

void cenx4_app_log_viewer_stop(void * ctx)
{
	(void) ctx;
}

void cenx4_app_log_viewer_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    cenx4_app_log_viewer_context_t * ctx = (cenx4_app_log_viewer_context_t *) _ctx;

	if ((node_id == 0) && (encoder_num == 0))
	{
		ctx->read_idx += (10 * val_change);
	}
}

void cenx4_app_log_viewer_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
	phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
}

void cenx4_app_log_render(cenx4_ui_t * ui, void * _ctx)
{
    cenx4_app_log_viewer_context_t * ctx = (cenx4_app_log_viewer_context_t *) _ctx;
    char buf[CENX4_APP_LOG_MAX_MSG_LEN + 1];
    coord_t y = 10;
    uint8_t idx = (ctx->read_idx - 10) % CENX4_APP_LOG_MAX_MSGS;

	buf[CENX4_APP_LOG_MAX_MSG_LEN] = 0;

    chsnprintf(buf, CENX4_APP_LOG_MAX_MSG_LEN, "Log %d-%d", idx, ctx->read_idx % CENX4_APP_LOG_MAX_MSGS);
	cenx4_ui_text(ui, 0, 0, ui->w, 0, justifyCenter, buf);

	for (int i = 0; i < 10; i++)
	{
		memcpy(buf, cenx4_app_log_buf[(idx + i) % CENX4_APP_LOG_MAX_MSGS], CENX4_APP_LOG_MAX_MSG_LEN);

		cenx4_ui_text(ui, 0, y, ui->w, 0, justifyLeft, buf);
		y += 10;
	}


}
