#ifndef CENX4_APP_SLAVE_H_
#define CENX4_APP_SLAVE_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"
#include "cenx4_can.h"
#include "cenx4_app_cfg.h"

extern const phi_app_desc_t cenx4_app_slave_desc;

typedef struct cenx4_app_slave_context_s
{
	uint8_t unused;

} cenx4_app_slave_context_t;

void cenx4_app_slave_start(void * ctx);
void cenx4_app_slave_stop(void * ctx);
void cenx4_app_slave_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_slave_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void cenx4_app_slave_can_cmd(void * ctx, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t xfer_id, const uint8_t * data, size_t len);

#endif /* CENX4_APP_SLAVE_H_ */
