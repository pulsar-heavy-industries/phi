#include "cenx4_app_registry.h"
#include "cenx4_can.h"
#include "cenx4_midi.h"
#include "cenx4_ui.h"
#include "usbcfg.h"


cenx4_app_contexts_t cenx4_app_contexts;

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */

volatile int32_t cnt = 0;

#include "phi_lib/phi_rotenc.h"
#include "phi_lib/phi_btn.h"

phi_rotenc_t phi_rotencs[4];
phi_btn_t btns[4];

#include "cenx4_ui.h"
void encoder_event_cb(uint8_t pin, int8_t value)
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

void btn_callback(uint8_t btn, phi_btn_event_t event, uint32_t param)
{
    phi_app_mgr_notify_btn_event(0, btn, event, param);


#if CENX4_IS_SLAVE
    send me over CAN
#endif
}


/////////////////////////////////


static THD_WORKING_AREA(waThread1, 512);
static THD_FUNCTION(Thread1, arg){
    (void)arg;
    chRegSetThreadName("gpio");

    while (true)
    {
        int p = palReadPort(GPIOB);
        systime_t now = chVTGetSystemTimeX();

        phi_rotenc_update_state(&phi_rotencs[0], (((p >> GPIOB_ROT1_A) & 1) << 1) | ((p >> GPIOB_ROT1_B) & 1));
        phi_rotenc_update_state(&phi_rotencs[1], (((p >> GPIOB_ROT2_A) & 1) << 1) | ((p >> GPIOB_ROT2_B) & 1));
        phi_rotenc_update_state(&phi_rotencs[2], (((p >> GPIOB_ROT3_A) & 1) << 1) | ((p >> GPIOB_ROT3_B) & 1));
        phi_rotenc_update_state(&phi_rotencs[3], (((p >> GPIOB_ROT4_A) & 1) << 1) | ((p >> GPIOB_ROT4_B) & 1));

        phi_btn_update_state(&(btns[0]), now, (p >> GPIOB_ROT1_SW) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[1]), now, (p >> GPIOB_ROT2_SW) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[2]), now, (p >> GPIOB_ROT3_SW) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[3]), now, (p >> GPIOB_ROT4_SW) & 1 ? FALSE : TRUE);

        phi_rotenc_process(phi_rotencs, PHI_ARRLEN(phi_rotencs), encoder_event_cb);
        phi_btn_process(
            btns,
            PHI_ARRLEN(btns),
            btn_callback,
            MS2ST(20),
            MS2ST(1000),
            MS2ST(100)
        );

	    chThdSleepMicroseconds(1000);
    }
}



/*
 * Application entry point.
 */

#define PHI_RND_ADC_BUF_DEPTH 32
#define PHI_RND_ADC_CH_NUM    2

void phi_rnd_init(void)
{
    ADCConversionGroup adccg = {
        FALSE,
        PHI_RND_ADC_CH_NUM,
        NULL, // done callback
        NULL, // err callback
        ADC_CFGR_CONT,            /* CFGR    */
        ADC_TR(0, 4095),          /* TR1     */
        {                         /* SMPR[2] */
          0,
          0
        },
        {                         /* SQR[4]  */
          ADC_SQR1_SQ1_N(ADC_CHANNEL_IN16) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN17),
          0,
          0,
          0
        }
    };
    volatile adcsample_t samples_buf[PHI_RND_ADC_BUF_DEPTH * PHI_RND_ADC_CH_NUM];

    uint32_t seed = 0;
    uint32_t i, j;

    // First, seed with our device ID/etc
    seed ^= CENX4_HW_SW_VER;
    for (i = 0; i < 12; i += 4)
    {
        seed ^= *(uint32_t *)(STM32_REG_UNIQUE_ID + i);
    }

    // Now use the adc data
    adcStart(&ADCD1, NULL);
    adcSTM32EnableTS(&ADCD1);
    adcSTM32EnableVREF(&ADCD1);
    adcSTM32EnableVBAT(&ADCD1);



    for (j = 0; j < 10; ++j)
    {
        adcConvert(&ADCD1, &adccg, (adcsample_t *) samples_buf, PHI_RND_ADC_BUF_DEPTH);

        for (i = 0; i < sizeof(samples_buf); i += 2)
        {
            seed ^= *(uint32_t *)(&(samples_buf[i]));
        }
    }
    adcStop(&ADCD1);

    srand(seed);

}

/*===========================================================================
 * Main
 *===========================================================================*/



int main(void)
{
	cenx4_ui_t * ui;
   halInit();
   chSysInit();

   uint32_t i;
   for(i = 0; i < PHI_ARRLEN(phi_rotencs); ++i) {
       phi_rotenc_init(&(phi_rotencs[i]));
       phi_rotencs[i].speed = 1;
   }
   for(i = 0; i < PHI_ARRLEN(btns); ++i) {
      phi_btn_init(&(btns[i]));
  }

   phi_rnd_init();
   chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

   gfxInit();
   cenx4_ui_init();

   chThdSleepMilliseconds(1000);

   cenx4_app_cfg_load();

   bduObjectInit(&BDU1);
   bduStart(&BDU1, &bulkusbcfg);

   cenx4_midi_init();


   ui = cenx4_ui_lock(1);
   strcpy(ui->state.boot.misc_text, "USB");
   cenx4_ui_unlock(ui);
   #define usb_lld_connect_bus(usbp)
   #define usb_lld_disconnect_bus(usbp)
   usbDisconnectBus(midiusbcfg.usbp);
   chThdSleepMilliseconds(1500);
   usbStart(midiusbcfg.usbp, &usbcfg);
   usbConnectBus(midiusbcfg.usbp);

   ui = cenx4_ui_lock(1);
  strcpy(ui->state.boot.misc_text, "CAN");
  cenx4_ui_unlock(ui);
  cenx4_can_init();

   chThdSleepMilliseconds(1000);

   ui = cenx4_ui_lock(1);
  strcpy(ui->state.boot.misc_text, "START");
  cenx4_ui_unlock(ui);

   phi_app_mgr_init();
   phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
   //phi_app_mgr_switch_app(&cenx4_app_ableton_desc, &(cenx4_app_contexts.ableton));
   //phi_app_mgr_switch_app(&cenx4_app_test_desc, &(cenx4_app_contexts.test));


  /*
   * Serves timer events.
   */
   while (true) {
	   chThdSleepMilliseconds(500);
	   palTogglePad(GPIOA, GPIOA_LED);
	   // midi_usb_MidiSend3(1, 0x90, 0x40, i++);
   }

   return 0;
}
