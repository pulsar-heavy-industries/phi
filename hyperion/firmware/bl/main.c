#include "phi_lib/phi_bl_common.h"
#include "hyperion_ui.h"
#include "hyperion_can.h"
#include "hyperion_io.h"

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
static ADCConfig adccfg = {};
#define ADC_BUF_DEPTH 2 // depth of buffer
#define ADC_CH_NUM 1    // number of used ADC channels
static volatile adcsample_t samples_buf[ADC_BUF_DEPTH * ADC_CH_NUM] = {5,}; // results array
static ADCConversionGroup adccg = {
   // this 3 fields are common for all MCUs
      // set to TRUE if need circular buffer, set FALSE otherwise
      TRUE,
      // number of channels
      (uint16_t)(ADC_CH_NUM),
      // callback function, set to NULL for begin
      NULL, NULL,
   // Resent fields are stm32 specific. They contain ADC control registers data.
   // Please, refer to ST manual RM0008.pdf to understand what we do.
      // CR1 register content, set to zero for begin
      0,
      // CR2 register content, set to zero for begin
	  ADC_CR2_SWSTART,
	    /* ADC SMPR1 */ 0,
	    /* ADC SMPR2 */ ADC_SMPR2_SMP_AN3(ADC_SAMPLE_480),
	    /* ADC SQR1 */ ADC_SQR1_NUM_CH(ADC_CH_NUM),
	    /* ADC SQR2 */ 0,
	    /* ADC SQR3 */ ADC_SQR3_SQ1_N(ADC_CHANNEL_IN3)
};

#define ADC_GRP2_NUM_CHANNELS   8
#define ADC_GRP2_BUF_DEPTH      16
adcsample_t adc_data[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];


static const ADCConversionGroup adcgrpcfg2 = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  NULL, //adccallback,
  NULL, // adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  0,                        /* SMPR2 */
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN1(ADC_SAMPLE_56) |
  ADC_SMPR2_SMP_AN2(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN3(ADC_SAMPLE_56) |
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_56) |
  ADC_SMPR2_SMP_AN6(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN7(ADC_SAMPLE_56),
  ADC_SQR1_NUM_CH(ADC_GRP2_NUM_CHANNELS),
  ADC_SQR2_SQ8_N(ADC_CHANNEL_IN7) | ADC_SQR2_SQ7_N(ADC_CHANNEL_IN6),
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN5)   | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN4) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN3)   | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1)   | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)
};


char boot_user_status[16];

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
    const phi_bl_hdr_t * hdr = (phi_bl_hdr_t *) PHI_BL_USER_ADDR;
    uint32_t flash_size = *((uint16_t *) 0x1FFF7A22) * 1024;
    uint32_t flash_end = PHI_BL_FLASH_START + flash_size;

    int is_watchdog_reset = RCC->CSR;
    RCC->CSR |= RCC_CSR_RMVF;

    if (is_watchdog_reset & RCC_CSR_WDGRSTF)
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
        strcpy(boot_user_status, "BadCrc");
        return;
    }

    strcpy(boot_user_status, "FW OK");

    user_jump_to_app(hdr->start_addr);
}



/*
 * Application entry point.
 */
int main(void)
{
	bool force_bl;
	hyperion_ui_t * ui;

    halInit();

    force_bl = hyperion_io_bl_force();
    if (!force_bl) {
        boot_user();
    } else {
        strcpy(boot_user_status, "Forced");
    }

    chSysInit();

    gfxInit();

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    hyperion_io_init();
    hyperion_ui_init();

    ui = hyperion_ui_lock();
    strcpy(ui->state.boot.misc_text, boot_user_status);
    hyperion_ui_unlock(ui);

    hyperion_can_init();


//    adcStart(&ADCD1, NULL);
//    adcStartConversion(&ADCD1, &adcgrpcfg2, adc_data, ADC_GRP2_BUF_DEPTH);


    while (true) {
    	chThdSleepMilliseconds(600);
    }

    return 0;
}
