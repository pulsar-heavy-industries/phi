#include "phi_lib/phi_app_mgr.h"
#include "narvi_io.h"
#include "lcd.h"

phi_rotenc_t narvi_rotencs[2];
phi_btn_t narvi_btns[2];


static void encoder_event_cb(uint8_t pin, int8_t value)
{

}

extern uint32_t dac_buffer_pos;
extern uint32_t dac_buffer2_pos;
static void btn_event_cb(uint8_t btn, phi_btn_event_t event, uint32_t param)
{
	if (event != PHI_BTN_EVENT_PRESSED)
	{
		return;
	}

	switch (btn)
	{
	case 0:
		dac_buffer_pos++;
		dac_buffer2_pos++;
		break;

	case 1:
		dac_buffer_pos = dac_buffer2_pos = 0;
		break;

	default:
		chDbgCheck(FALSE);
	}
}

volatile int aa[100] = {0,}, cc[100] = {0,};
volatile int aa_x = 1, cc_x = 1;
static THD_WORKING_AREA(narvi_io_thread_wa, 512);
static THD_FUNCTION(narvi_io_thread, arg)
		{
    (void)arg;
    chRegSetThreadName("gpio");

    // Hack for initial LCD update
    encoder_event_cb(0, 0);

    while (true)
    {
        int pa = palReadPort(GPIOA);
        int pb = palReadPort(GPIOB);
        int pc = palReadPort(GPIOC);

        systime_t now = chVTGetSystemTimeX();

        phi_rotenc_update_state(&narvi_rotencs[0], (((pa >> GPIOA_CTRL_ROT1_A) & 1) << 1) | ((pc >> GPIOC_CTRL_ROT1_B) & 1));
        phi_rotenc_update_state(&narvi_rotencs[1], (((pb >> GPIOB_CTRL_ROT2_A) & 1) << 1) | ((pb >> GPIOB_CTRL_ROT2_B) & 1));

        phi_btn_update_state(&(narvi_btns[0]), now, (pc >> GPIOC_CTRL_ROT1_SW) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(narvi_btns[1]), now, (pb >> GPIOB_CTRL_ROT2_SW) & 1 ? FALSE : TRUE);

        phi_rotenc_process(narvi_rotencs, PHI_ARRLEN(narvi_rotencs), encoder_event_cb);
        phi_btn_process(
        	narvi_btns,
            PHI_ARRLEN(narvi_btns),
            btn_event_cb,
            MS2ST(20),
            MS2ST(1000),
            MS2ST(100)
        );

	    chThdSleepMicroseconds(1000);
    }
}

void narvi_io_init(void)
{
	uint32_t i;
	for (i = 0; i < PHI_ARRLEN(narvi_rotencs); ++i) {
	   phi_rotenc_init(&(narvi_rotencs[i]));
	   narvi_rotencs[i].speed = 1;
	}

	for (i = 0; i < PHI_ARRLEN(narvi_btns); ++i) {
		phi_btn_init(&(narvi_btns[i]));
	}

	chThdCreateStatic(narvi_io_thread_wa, sizeof(narvi_io_thread_wa), NORMALPRIO, narvi_io_thread, NULL);
}
