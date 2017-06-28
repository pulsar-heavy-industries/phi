/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "phi_lib/phi_bl_common.h"

#include "usbcfg.h"




#include "lcd.h"

static uint8_t ii;

#define LINE_RS                     PAL_LINE(GPIOE, 14U) // MOSI
#define LINE_RW                     PAL_LINE(GPIOE, 13U) // MISO
#define LINE_E                      PAL_LINE(GPIOE, 12U) // CLK
#define LINE_A                      PAL_LINE(GPIOE, 11U) // D4 unused

/* Data PIN are connected from PC0 to PC7 */
#if !LCD_USE_4_BIT_MODE
#error sux
#endif
#define LINE_D4                     PAL_LINE(GPIOE, 7U)
#define LINE_D5                     PAL_LINE(GPIOE, 8U)
#define LINE_D6                     PAL_LINE(GPIOE, 9U)
#define LINE_D7                     PAL_LINE(GPIOE, 10U)

/*===========================================================================*/
/* LCD configuration                                                         */
/*===========================================================================*/

#if LCD_USE_DIMMABLE_BACKLIGHT
static const PWMConfig pwmcfg = {
  100000,                                   /* 100kHz PWM clock frequency.   */
  100,                                      /* PWM period is 1000 cycles.    */
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0
};
#endif

static const lcd_pins_t lcdpins = {
  LINE_RS,
  LINE_RW,
  LINE_E,
  LINE_A,
  {
#if !LCD_USE_4_BIT_MODE
   LINE_D0,
   LINE_D1,
   LINE_D2,
   LINE_D3,
#endif
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
#if LCD_USE_DIMMABLE_BACKLIGHT
  &PWMD1,                   /* PWM Driver for back-light */
  &pwmcfg,                  /* PWM driver configuration for back-light */
  0,                        /* PWM channel */
#endif
  100,                      /* Back-light */
};




/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("blinker2");
  while (true) {
    palClearPad(GPIOA, GPIOA_USER_LED);
    chThdSleepMilliseconds(250);
    palSetPad(GPIOA, GPIOA_USER_LED);
    chThdSleepMilliseconds(250);
  }
}
/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/

extern audio_state_t audio;


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
int main(void) {
	bool force_bl;

    halInit();

    force_bl = FALSE; // TODO
if (!force_bl) {
        boot_user();
    } else {
        strcpy(boot_user_status, "Forced");
    }

    chSysInit();



  lcdInit();

#if LCD_USE_DIMMABLE_BACKLIGHT
  /* Configuring Anode PIN as TIM1 CH1 alternate function. */
  palSetLineMode(LINE_A, PAL_MODE_ALTERNATE(1));
#else
  /* Configuring Anode PIN as TIM1 CH1 alternate function. */
  palSetLineMode(LINE_A, PAL_MODE_OUTPUT_PUSHPULL |
                 PAL_STM32_OSPEED_HIGHEST);
#endif


  /* Configuring RW, RS and E PIN as Output Push Pull. Note that Data PIN are
     managed Internally */
  palSetLineMode(LINE_RW, PAL_MODE_OUTPUT_PUSHPULL |
                PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_RS, PAL_MODE_OUTPUT_PUSHPULL |
                PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_E, PAL_MODE_OUTPUT_PUSHPULL |
                PAL_STM32_OSPEED_HIGHEST);


  lcdStart(&LCDD1, &lcdcfg);
  lcdWriteString(&LCDD1, "NARVI BL", 0);
 lcdWriteString(&LCDD1, boot_user_status, 40);


//  ab_main_midi_init();
  mduObjectInit(&MDU1);
  mduStart(&MDU1, &midiusbcfg);


  audioObjectInit(&audio);



  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(midiusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(midiusbcfg.usbp, &usbcfg);
  usbConnectBus(midiusbcfg.usbp);



  /*
   * Creating the blinker threads.

   */
//  codec_init();
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    NORMALPRIO + 10, Thread2, NULL);

//	 while (1) chThdSleepMilliseconds(10);



  /*
   * Normal main() thread activity, in this demo it just performs
   * a shell respawn upon its termination.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
