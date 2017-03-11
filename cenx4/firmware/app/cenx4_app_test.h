#ifndef CENX4_APP_TEST_H_
#define CENX4_APP_TEST_H_

#include "phi_lib/phi_app_mgr.h"
#include "cenx4_conf.h"


extern const phi_app_desc_t cenx4_app_test_desc;

typedef struct cenx4_app_test_context_s
{
    uint8_t num_pots;
    uint8_t pots[CENX4_MAX_POTS];
} cenx4_app_test_context_t;

void cenx4_app_test_start(void * ctx);
void cenx4_app_test_stop(void * ctx);
void cenx4_app_test_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void cenx4_app_test_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);

#endif /* CENX4_APP_TEST_H_ */
