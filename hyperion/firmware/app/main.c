#include "hyperion_ui.h"
#include "hyperion_can.h"
#include "hyperion_io.h"
#include "hyperion_app_registry.h"


hyperion_app_contexts_t hyperion_app_contexts;


static THD_WORKING_AREA(waThread1, 512); // TODO
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);       /* Orange.  */
    chThdSleepMilliseconds(500);
  }
}




/*
 * Application entry point.
 */


int main(void)
{
    halInit();
    chSysInit();
    gfxInit();


    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    hyperion_io_init();
    hyperion_ui_init();
    hyperion_can_init();

    chThdSleepMilliseconds(500);

    phi_app_mgr_init();
    phi_app_mgr_switch_app(&hyperion_app_test_desc, &(hyperion_app_contexts.test));


    while (true) {
    	chThdSleepMilliseconds(600);
    }

    return 0;
}
