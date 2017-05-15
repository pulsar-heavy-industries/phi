#include "cenx4_app_registry.h"
#include "cenx4_can.h"
#include "cenx4_midi.h"
#include "cenx4_ui.h"
#include "cenx4_io.h"
#include "usbcfg.h"

cenx4_app_contexts_t cenx4_app_contexts;
bool cenx4_is_master;

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

#include "phi_lib/phi_at45.h"

const phi_at45_cfg_t at45_cfg = {
	.spi = &SPID1,
	.spi_cfg = {
		NULL,
		GPIOA,
		GPIOA_FLASH_CS,
		SPI_CR1_BR_1,
		0,
	},
};

phi_at45_t at45;;

int main(void)
{
	cenx4_ui_t * ui;
	phi_at45_ret_t ret;

	halInit();
	chSysInit();

	cenx4_is_master = palReadPad(GPIOC, GPIOC_MASTER_EN) ? FALSE : TRUE;

	phi_rnd_init();

	cenx4_io_init();

	gfxInit();
	cenx4_ui_init();

	if (cenx4_is_master)
	{
		cenx4_app_cfg_load();

		// TODO show error
		ret = phi_at45_init(&at45, &at45_cfg);
		chDbgCheck(ret == PHI_AT45_RET_SUCCESS);

		cenx4_midi_init();

		ui = cenx4_ui_lock(1);
		strcpy(ui->state.boot.misc_text, "USB");
		cenx4_ui_unlock(ui);
		#define usb_lld_connect_bus(usbp) palClearPad(GPIOA, GPIOA_USB_CONN)
		#define usb_lld_disconnect_bus(usbp) palSetPad(GPIOA, GPIOA_USB_CONN)
		usbDisconnectBus(midiusbcfg.usbp);
		chThdSleepMilliseconds(100);
		usbStart(midiusbcfg.usbp, &usbcfg);
		usbConnectBus(midiusbcfg.usbp);
	}

	ui = cenx4_ui_lock(1);
	strcpy(ui->state.boot.misc_text, "CAN");
	cenx4_ui_unlock(ui);
	cenx4_can_init();

	if (cenx4_is_master)
	{
		// HACK make debugging easier by resetting potential slaves
		{
			for (int i = 0; i < PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS; ++i)
			{
				phi_can_xfer(
					&cenx4_can,
					PHI_CAN_PRIO_LOWEST,
					PHI_CAN_MSG_ID_RESET,
					PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + i,
					NULL,
					0,
					NULL,
					0,
					NULL,
					PHI_CAN_DEFAULT_TIMEOUT
				);
			}
		}

		// Give slaves time to start?
		chThdSleepMilliseconds(1500);
	}

	ui = cenx4_ui_lock(1);
	strcpy(ui->state.boot.misc_text, "START");
	cenx4_ui_unlock(ui);

	phi_app_mgr_init();

	//phi_app_mgr_switch_app(&cenx4_app_ableton_desc, &(cenx4_app_contexts.ableton));
	//phi_app_mgr_switch_app(&cenx4_app_test_desc, &(cenx4_app_contexts.test));
	if (cenx4_is_master)
	{
		phi_app_mgr_switch_app(&cenx4_app_init_desc, &(cenx4_app_contexts.init));
//		phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
//		phi_app_mgr_switch_app(&cenx4_app_test_desc, &(cenx4_app_contexts.test));
	}
	else
	{
		phi_app_mgr_switch_app(&cenx4_app_slave_desc, &(cenx4_app_contexts.slave));
	}


	while (true) {
	   chThdSleepMilliseconds(500);
	   palTogglePad(GPIOA, GPIOA_LED);
	   // midi_usb_MidiSend3(1, 0x90, 0x40, i++);
	}

   return 0;
}
