#ifndef CENX4_APP_LOG_H_
#define CENX4_APP_LOG_H_

#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_app_mgr.h"
#include "cenx4_ui.h"


#define CENX4_APP_LOG_MAX_MSG_LEN 10
#define CENX4_APP_LOG_MAX_MSGS    32

extern const phi_app_desc_t cenx4_app_log_viewer_desc;


typedef struct cenx4_app_log_viewer_context_s
{
	uint32_t read_idx;

} cenx4_app_log_viewer_context_t;

void cenx4_app_log(const char * msg);
#define cenx4_app_log_fmt(fmt, ...) do { \
	char __log_buf[CENX4_APP_LOG_MAX_MSG_LEN + 1]; \
	chsnprintf(__log_buf, CENX4_APP_LOG_MAX_MSG_LEN, fmt, __VA_ARGS__); \
	cenx4_app_log(__log_buf); \
} while(0)

void cenx4_app_log_viewer_start(void * ctx);
void cenx4_app_log_viewer_stop(void * ctx);
void cenx4_app_log_viewer_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_log_viewer_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);

void cenx4_app_log_render(cenx4_ui_t * ui, void * ctx);

#endif /* CENX4_APP_LOG_H_ */
