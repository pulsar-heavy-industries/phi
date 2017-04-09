#ifndef HYPERION_APP_SLAVE_H_
#define HYPERION_APP_SLAVE_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"
#include "hyperion_can.h"

extern const phi_app_desc_t hyperion_app_slave_desc;

typedef struct hyperion_app_slave_context_s
{
	uint8_t unused;

} hyperion_app_slave_context_t;

void hyperion_app_slave_start(void * ctx);
void hyperion_app_slave_stop(void * ctx);
void hyperion_app_slave_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void hyperion_app_slave_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void hyperion_app_slave_pot_event(void * ctx, uint8_t node_id, uint8_t pot_num, uint8_t val);
void hyperion_app_slave_can_cmd(void * ctx, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t xfer_id, const uint8_t * data, size_t len);

#endif /* HYPERION_APP_SLAVE_H_ */
