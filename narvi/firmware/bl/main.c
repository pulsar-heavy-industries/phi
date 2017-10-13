#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_bl_common.h"
#include "phi_lib/phi_at45.h"
#include "usbcfg.h"
#include "lcd.h"
#include "narvi_midi.h"
#include "narvi_io.h"


/******************************************************************************
 * LCD
 *****************************************************************************/
#define LINE_RS                     PAL_LINE(GPIOE, 13U) // MISO
#define LINE_E                      PAL_LINE(GPIOE, 14U) // MOSI
#define LINE_A                      PAL_LINE(GPIOE, 14U) // UNUSED
#if !LCD_USE_4_BIT_MODE
#error sux
#endif
#define LINE_D4                     PAL_LINE(GPIOE, 7U)
#define LINE_D5                     PAL_LINE(GPIOE, 8U)
#define LINE_D6                     PAL_LINE(GPIOE, 9U)
#define LINE_D7                     PAL_LINE(GPIOE, 10U)

static const lcd_pins_t lcdpins = {
  LINE_RS,
  LINE_E,
  LINE_A,
  {
   LINE_D4,
   LINE_D5,
   LINE_D6,
   LINE_D7
  }
};

static const LCDConfig lcdcfg = {
  LCD_CURSOR_OFF,           /* Cursor disabled */
  LCD_BLINKING_OFF,         /* Blinking disabled */
  LCD_SET_FONT_5X10,        /* Font 5x10 */
  LCD_SET_2LINES,           /* 2 lines */
  &lcdpins,                 /* pin map */
  100,                      /* Back-light */
};


/******************************************************************************
 * Serial Flash
 *****************************************************************************/
const phi_at45_cfg_t at45_cfg = {
	.spi = &SPID3,
	.spi_cfg = {
		NULL,
		GPIOD,
		GPIOD_FLASH_CS,
		SPI_CR1_BR_1,
	},
};

phi_at45_t at45;


/******************************************************************************
 * LED Blinker
 *****************************************************************************/

static THD_WORKING_AREA(wa_blinker_thread, 128);
static THD_FUNCTION(blinker_thread, arg)
{
	(void)arg;
	chRegSetThreadName("blinker");

	while (true) {
		palClearPad(GPIOA, GPIOA_USER_LED);
		chThdSleepMilliseconds(250);
		palSetPad(GPIOA, GPIOA_USER_LED);
		chThdSleepMilliseconds(250);
	}
}


/******************************************************************************
 * Main
 *****************************************************************************/

char boot_user_status[16];

void user_jump_to_app(uint32_t address)
{
	typedef void (*pFunction)(void);
	pFunction Jump_To_Application;
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
	for (int i = WWDG_IRQn; i < 90; ++i)
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

    bool force_bl = palReadPad(GPIOC, GPIOC_CTRL_ROT1_SW) ? FALSE : TRUE;
//    force_bl = TRUE;
    if (force_bl)
    {
        strcpy(boot_user_status, "Forced");
        return;
    }

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
    halInit();
    boot_user();
    chSysInit();

    lcdInit();
    palSetLineMode(LINE_RS, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    palSetLineMode(LINE_E, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    lcdStart(&LCDD1, &lcdcfg);
    lcdWriteString(&LCDD1, "PHI Narvi - BL", 0);

    /* Try to init serial flash */
	char buf[20];
    phi_at45_ret_t ret = phi_at45_init(&at45, &at45_cfg);
    if (ret == PHI_AT45_RET_SUCCESS)
    {
    	chsnprintf(buf, sizeof(buf) - 1, "AT45: %dMB", (at45.desc->num_pages * 512) / (1024 * 1024));
    }
    else
    {
    	chsnprintf(buf, sizeof(buf) - 1, "AT45 err: %d", ret);
    }
    lcdWriteString(&LCDD1, buf, 40);
    chThdSleepSeconds(1);

    mduObjectInit(&MDU1);
    mduStart(&MDU1, &midiusbcfg);

    narvi_io_init();
    narvi_midi_init();

    /*
     * Activates the USB driver and then the USB bus pull-up on D+.
     * Note, a delay is inserted in order to not have to disconnect the cable
     * after a reset.
     */
    usbDisconnectBus(midiusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(midiusbcfg.usbp, &usbcfg);
    usbConnectBus(midiusbcfg.usbp);

    chThdCreateStatic(wa_blinker_thread, sizeof(wa_blinker_thread), NORMALPRIO + 10, blinker_thread, NULL);

    while (true) {
    	chThdSleepMilliseconds(500);
    }
}
