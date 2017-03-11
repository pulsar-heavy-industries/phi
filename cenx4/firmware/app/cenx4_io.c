#include "phi_lib/phi_app_mgr.h"
#include "cenx4_io.h"

phi_rotenc_t cenx4_rotencs[4];
phi_btn_t cenx4_btns[4];


static void encoder_event_cb(uint8_t pin, int8_t value)
{
    phi_app_mgr_notify_encoder_event(0, pin, value);

#if CENX4_IS_SLAVE
    phi_main_can_handle_encoder_event_t msg = {
        .encoder_num = pin,
        .val_change = value,
    };

    phi_can_xfer(
        &cenx4_can,
        PHI_CAN_PRIO_LOWEST + 1,
        PHI_CAN_MSG_ID_MAIN_ENCODER_EVENT,
        PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
        (const uint8_t *) &msg,
        sizeof(msg),
        NULL,
        0,
        NULL,
        PHI_CAN_DEFAULT_TIMEOUT
    );
#endif
}

static void btn_event_cb(uint8_t btn, phi_btn_event_t event, uint32_t param)
{
    phi_app_mgr_notify_btn_event(0, btn, event, param);


#if CENX4_IS_SLAVE
    send me over CAN
#endif
}

static THD_WORKING_AREA(cenx4_io_thread_wa, 512);
static THD_FUNCTION(cenx4_io_thread, arg){
    (void)arg;
    chRegSetThreadName("gpio");

    while (true)
    {
        int p = palReadPort(GPIOB);
        systime_t now = chVTGetSystemTimeX();

        phi_rotenc_update_state(&cenx4_rotencs[0], (((p >> GPIOB_ROT1_A) & 1) << 1) | ((p >> GPIOB_ROT1_B) & 1));
        phi_rotenc_update_state(&cenx4_rotencs[1], (((p >> GPIOB_ROT2_A) & 1) << 1) | ((p >> GPIOB_ROT2_B) & 1));
        phi_rotenc_update_state(&cenx4_rotencs[2], (((p >> GPIOB_ROT3_A) & 1) << 1) | ((p >> GPIOB_ROT3_B) & 1));
        phi_rotenc_update_state(&cenx4_rotencs[3], (((p >> GPIOB_ROT4_A) & 1) << 1) | ((p >> GPIOB_ROT4_B) & 1));

        phi_btn_update_state(&(cenx4_btns[0]), now, (p >> GPIOB_ROT1_SW) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(cenx4_btns[1]), now, (p >> GPIOB_ROT2_SW) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(cenx4_btns[2]), now, (p >> GPIOB_ROT3_SW) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(cenx4_btns[3]), now, (p >> GPIOB_ROT4_SW) & 1 ? FALSE : TRUE);

        phi_rotenc_process(cenx4_rotencs, PHI_ARRLEN(cenx4_rotencs), encoder_event_cb);
        phi_btn_process(
        	cenx4_btns,
            PHI_ARRLEN(cenx4_btns),
            btn_event_cb,
            MS2ST(20),
            MS2ST(1000),
            MS2ST(100)
        );

	    chThdSleepMicroseconds(1000);
    }
}

void cenx4_io_init(void)
{
	uint32_t i;
	for (i = 0; i < PHI_ARRLEN(cenx4_rotencs); ++i) {
	   phi_rotenc_init(&(cenx4_rotencs[i]));
	   cenx4_rotencs[i].speed = 1;
	}

	for (i = 0; i < PHI_ARRLEN(cenx4_btns); ++i) {
		phi_btn_init(&(cenx4_btns[i]));
	}

	chThdCreateStatic(cenx4_io_thread_wa, sizeof(cenx4_io_thread_wa), NORMALPRIO, cenx4_io_thread, NULL);
}
