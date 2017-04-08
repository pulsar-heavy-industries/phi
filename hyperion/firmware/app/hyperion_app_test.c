#include "hyperion_app_test.h"
#include "hyperion_ui.h"


const phi_app_desc_t hyperion_app_test_desc = {
    .start = hyperion_app_test_start,
    .stop = hyperion_app_test_stop,
	.tick = hyperion_app_test_tick,
	.tick_interval = MS2ST(10),
};

void hyperion_app_test_start(void * _ctx)
{
    hyperion_app_test_context_t * ctx = (hyperion_app_test_context_t *) _ctx;
//    hyperion_ui_t * ui;
//    uint8_t i;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

}

void hyperion_app_test_stop(void * ctx)
{
    (void) ctx;
}

void hyperion_app_test_tick(void * ctx)
{

}
