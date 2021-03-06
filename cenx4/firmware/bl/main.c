#include "phi_lib/phi_lib.h"
#include "gfx.h"

#include "cenx4_can.h"
#include "cenx4_ui.h"
#include "cenx4_midi.h"
#include "usbcfg.h"

bool cenx4_is_master;


/*===========================================================================*/
/* Random nunmber generation
 *===========================================================================*/
#define PHI_RND_ADC_BUF_DEPTH 32
#define PHI_RND_ADC_CH_NUM    2

void rnd_init(void)
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

#include "phi_lib/phi_bl_common.h"
char boot_user_status[CENX4_UI_MAX_LINE_TEXT_LEN];

void user_jump_to_app(uint32_t address) {
  typedef void (*pFunction)(void);

  pFunction Jump_To_Application;

  /* variable that will be loaded with the start address of the application */
  volatile uint32_t * JumpAddress;
  const volatile uint32_t * ApplicationAddress = (volatile uint32_t *) address;

  /* get jump address from application vector table */
  JumpAddress = (volatile uint32_t *) ApplicationAddress[1];

  /* load this address into function pointer */
  Jump_To_Application = (pFunction) JumpAddress;

  /* reset all interrupts to default */
  chSysDisable();

  /* Clear pending interrupts just to be on the safe side*/
  // TODO SCB_ICSR = ICSR_PENDSVCLR;

  /* Disable all interrupts */
  for (int i = WWDG_IRQn; i < 82; ++i)
  {
      nvicDisableVector(i);
  }

  /* set stack pointer as in application's vector table */
  __set_MSP((uint32_t) (ApplicationAddress[0]));
  Jump_To_Application();
}

void boot_user(void)
{
    const phi_bl_hdr_t * hdr = (phi_bl_hdr_t *) CENX4_BL_USER_ADDR;
    uint32_t flash_size = *((uint16_t *)FLASH_SIZE_DATA_REGISTER) * 1024;
    uint32_t flash_end = PHI_BL_FLASH_START + flash_size;

    int is_watchdog_reset = RCC->CSR;
    RCC->CSR |= RCC_CSR_RMVF;

    if (0 == palReadPad(GPIOB, GPIOB_ROT1_SW))
    {
        strcpy(boot_user_status, "Forced");
        return;
    }

    if (is_watchdog_reset & RCC_CSR_IWDGRSTF)
    {
        strcpy(boot_user_status, "Requested");
        return;
    }

    if (hdr->magic != PHI_BL_HDR_MAGIC)
    {
        strcpy(boot_user_status, "BadMagic");
        return;
    }

    if (hdr->hdr_data_crc32 != phi_crc32(&(hdr->hdr_data[0]), PHI_BL_HDR_DATA_SIZE))
    {
        strcpy(boot_user_status, "BadHdrCrc");
        return;
    }

    if ((hdr->fw_data_size == 0) ||
    	((hdr->start_addr + hdr->fw_data_size) >= flash_end))
	{
		strcpy(boot_user_status, "BadDataSize");
		return;
	}

    if (hdr->fw_data_crc32 != phi_crc32(&(hdr->fw_data[0]), hdr->fw_data_size))
    {
        strcpy(boot_user_status, "BadFwCrc");
        return;
    }

    strcpy(boot_user_status, "FW OK");

    user_jump_to_app(hdr->start_addr);
}


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

phi_at45_t at45;
#include "phi_lib/phi_bl_multiimg.h"
volatile phi_bl_multiimg_hdr_t hhh;

int main(void)

{
    cenx4_ui_t * ui;
    phi_at45_ret_t ret;

    halInit();
    boot_user();

    chSysInit();
	cenx4_is_master = palReadPad(GPIOC, GPIOC_MASTER_EN) ? FALSE : TRUE;

    rnd_init();
    gfxInit();
    cenx4_ui_init();

    if (cenx4_is_master)
    {
		cenx4_midi_init();

		/*
		 * Activates the USB driver and then the USB bus pull-up on D+.
		 * Note, a delay is inserted in order to not have to disconnect the cable
		 * after a reset.
		 */
		#define usb_lld_connect_bus(usbp) palClearPad(GPIOA, GPIOA_USB_CONN)
		#define usb_lld_disconnect_bus(usbp) palSetPad(GPIOA, GPIOA_USB_CONN)
		usbDisconnectBus(midiusbcfg.usbp);
		chThdSleepMilliseconds(1000);
		usbStart(midiusbcfg.usbp, &usbcfg);
		usbConnectBus(midiusbcfg.usbp);

		// TODO show error
		ret = phi_at45_init(&at45, &at45_cfg);
		chDbgCheck(ret == PHI_AT45_RET_SUCCESS);
    }
    else
    {
    	cenx4_can_init();
    }


    ui = cenx4_ui_lock(1);
    strcpy(ui->state.boot.misc_text[1], boot_user_status);
    cenx4_ui_unlock(ui);

    while (true) {
	   chThdSleepMilliseconds(100);
	   palTogglePad(GPIOA, GPIOA_LED);
    }

    return 0;
}
