#ifndef CENX4_APP_INIT_H_
#define CENX4_APP_INIT_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_app_mgr.h"
#include "phi_lib/phi_bl_multiimg.h"

extern const phi_app_desc_t cenx4_app_init_desc;


typedef struct cenx4_app_init_context_s
{

} cenx4_app_init_context_t;

void cenx4_app_init_start(void * ctx);
void cenx4_app_init_stop(void * ctx);

void cenx4_app_init_enter_init_mode(cenx4_app_init_context_t * ctx);
void cenx4_app_init_update_slaves(cenx4_app_init_context_t * ctx);
bool cenx4_app_init_bootload_slave(cenx4_app_init_context_t * ctx, uint8_t node_id);

#endif /* CENX4_APP_INIT_H_ */
