#ifndef HYPERION_APP_TEST_H_
#define HYPERION_APP_TEST_H_

#include "phi_lib/phi_app_mgr.h"


extern const phi_app_desc_t hyperion_app_test_desc;

typedef struct hyperion_app_test_context_s
{
    uint8_t num_pots;
//    uint8_t pots[8];

} hyperion_app_test_context_t;

void hyperion_app_test_start(void * ctx);
void hyperion_app_test_stop(void * ctx);
void hyperion_app_test_tick(void * ctx);


#endif /* HYPERION_APP_TEST_H_ */
