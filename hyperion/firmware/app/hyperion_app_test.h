#ifndef HYPERION_APP_TEST_H_
#define HYPERION_APP_TEST_H_

#include "phi_lib/phi_app_mgr.h"


extern const phi_app_desc_t hyperion_app_test_desc;

typedef struct hyperion_app_test_context_s
{
    int32_t rotencs[2];
    uint8_t pots[8];
    uint32_t cnt;
    uint32_t fill;
    systime_t last_updated;
} hyperion_app_test_context_t;

void hyperion_app_test_start(void * ctx);
void hyperion_app_test_stop(void * ctx);
void hyperion_app_test_tick(void * ctx);
void hyperion_app_test_encoder_event(void * ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change);
void hyperion_app_test_btn_event(void * ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param);
void hyperion_app_test_pot_event(void * ctx, uint8_t node_id, uint8_t pot_num, uint8_t val);


#endif /* HYPERION_APP_TEST_H_ */
