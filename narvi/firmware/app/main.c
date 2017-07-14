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
#include "test.h"

#include "chprintf.h"

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
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 1024);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker1");

//
//  {
//
//	  palSetPadMode(GPIOF, 7, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST); // SCK
//	  palSetPadMode(GPIOF, 8, PAL_MODE_ALTERNATE(5)); // MISO
//	  palSetPadMode(GPIOF, 9, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST); // MOSI
//
//	  palSetPadMode(GPIOC, 7, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST); // CS
//	  palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST); // CS
//
//	  palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST); // CS
//	  spiUnselect(&SPID5);
//
//	  set_vol(0xff);
//
//	  return;
//
//while (1) {
//	  //{
//	  static SPIConfig spi_cfg = {
//	  	NULL,
//	  	GPIOC,
//		8,
//		SPI_CR1_BR_1 | SPI_CR1_BR_2,// | SPI_CR1_CPOL |SPI_CR1_CPHA,
//	  };
//
//	  volatile uint8_t ch;
////	  volatile uint8_t ch2[2] = {0x80 | 23, 00};
//	  volatile uint8_t ch2[2] = {0x80 | 18, 0};
//	  volatile uint8_t ch3[2] = {2,2};
//
//	  spiAcquireBus(&SPID5);
//	  spiStart(&SPID5, &spi_cfg);
///*
//	  spiSelect(&SPID5);
//	  spiExchange(&SPID5, 2, ch2, ch3);
//	  spiUnselect(&SPID5);*/
///*
//	  ch2[0] = 16;
//	  ch2[1] = 0xff;
//	  ch3[0] = ch3[1] = 0;
//	  spiSelect(&SPID5);
//	  spiExchange(&SPID5, 2, ch2, ch3);
//	  spiUnselect(&SPID5);
//*/
//	  ch2[0] = 0x80 | 18;
//	  ch2[1] = 0;
//	  ch3[0] = ch3[1] = 0;
//	  spiSelect(&SPID5);
//	  spiExchange(&SPID5, 2, ch2, ch3);
//	  spiUnselect(&SPID5);
//
//	/*  ch2[0] = 18;
//	  ch2[1] = 0x50 | 0x80;
//	  ch3[0] = ch3[1] = 0;
//	  spiSelect(&SPID5);
//	  spiExchange(&SPID5, 2, ch2, ch3);
//	  spiUnselect(&SPID5);
//*/
//	/*  ch2[0] = 0x80 | 16;
//	  ch2[1] = 0;
//	  ch3[0] = ch3[1] = 0;
//	  spiSelect(&SPID5);
//	  spiExchange(&SPID5, 2, ch2, ch3);
//	  spiUnselect(&SPID5);*/
///*
//	  ch2[0] = 0x80 | 17;
//	  ch2[1] = 0;
//	  ch3[0] = ch3[1] = 0;
//	  spiSelect(&SPID5);
//	  spiExchange(&SPID5, 2, ch2, ch3);
//	  spiUnselect(&SPID5);
//*/
//
//	  spiReleaseBus(&SPID5);
//}
//
//
//
//
//  }

  uint8_t fVULevelL, fVULevelR;

  uint32_t cnt = 0;

  while (true) {
	  char buf[17];
//	  uint32_t vu = HAL_SAI_GetVULevels();
//	  uint16_t vu_l = (vu >> 16) & 0xffff;
//	  uint16_t vu_r = vu & 0xffff;
	  memset(buf, 0, sizeof(buf));


	  uint32_t vuLevel = HAL_SAI_GetVULevels();
	  uint16_t new_l = (vuLevel >> 16);// >> 12;
	  uint16_t new_r = (vuLevel & 0xFFFF);// >> 12;
		if (fVULevelL < new_l)
			fVULevelL = new_l;
		else
		{
			if (fVULevelL > 0)
				fVULevelL--;
			if (fVULevelL > 0)
						fVULevelL--;
			if (fVULevelL > 0)
						fVULevelL--;
		}

		if (fVULevelR < new_r)
			fVULevelR = new_r;
		else
		{
			if (fVULevelR > 0)
				fVULevelR--;
			if (fVULevelR > 0)
						fVULevelR--;
			if (fVULevelR > 0)
						fVULevelR--;
		}

//    palClearPad(GPIOG, GPIOG_LED4_RED);
//    chThdSleepMilliseconds(500);
//    palSetPad(GPIOG, GPIOG_LED4_RED);
//    chThdSleepMilliseconds(500);
		++cnt;

		const char vu[][9] = {
			"        ",
			"|       ",
			"#       ",
			"#|      ",
			"##      ",
			"##|     ",
			"###     ",
			"###|    ",
			"####    ",
			"####!   ",
			"#####   ",
			"#####!  ",
			"######  ",
			"######! ",
			"####### ",
			"#######!",
		};
#if 0
		if (cnt == 10) {
		  lcdReturnHome(&LCDD1);

		  chsnprintf(buf, sizeof(buf) - 1, "L: %s", vu[fVULevelL >> 3]);
		  lcdWriteString(&LCDD1, buf, 0);

		  chsnprintf(buf, sizeof(buf) - 1, "R: %s", vu[fVULevelR >> 3]);
		  lcdWriteString(&LCDD1, buf, 40);

		  cnt = 0;
		}
#endif
	  chThdSleepMicroseconds(100);
  }
}

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



/*
 * Application entry point.
 */
int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
//
//  sdStart(&SD6, NULL);
//
////u8 i = 0;
//while (TRUE) {
////	i = chIOGet(&SD1);
////	if(i!=0)
//	uint8_t buf[] = {0};
//	sdRead(&SD6, buf, sizeof(buf));
//	  chThdSleepMilliseconds(100);
//}


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

#if 0
  lcdStart(&LCDD1, &lcdcfg);
  lcdWriteString(&LCDD1, "Narvi :)", 0);
//  lcdWriteString(&LCDD1, "abcdefghijklmnopqrstuvwxyz",40);
#endif

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
  codec_init();
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 10, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    NORMALPRIO + 10, Thread2, NULL);

//	 while (1) chThdSleepMilliseconds(10);




  static event_listener_t listener;
    chEvtRegisterMask(&audio.audio_events, &listener, AUDIO_EVENT);

    for(;;) {
        /*
         * Wait for audio event.
         */
        chEvtWaitOne(AUDIO_EVENT);
        eventflags_t evt = chEvtGetAndClearFlags(&listener);

        /*
         * USB state cahanged, switch LED3.
         */
        if (evt & AUDIO_EVENT_USB_STATE) {
      /*    if (USBD1.state == USB_ACTIVE)
            palSetPad(GPIOD, GPIOD_LED3);
          else
            palClearPad(GPIOD, GPIOD_LED3);*/
        }

        /*
         * Audio playback started (stopped).
         * Enable (Disable) external DAC and I2S bus.
         * Enable (Disable) SOF capture.
         */
        if (evt & AUDIO_EVENT_PLAYBACK) {
          if (audio.playback) {
    //        palSetPad(GPIOD, GPIOD_LED6);
  //          i2sStart(&I2SD3, &i2scfg);
  //          i2sStartExchange(&I2SD3);
            start_sof_capture();
          } else {
            stop_sof_capture();
  //          i2sStopExchange(&I2SD3);
  //          i2sStop(&I2SD3);
  //          palClearPad(GPIOD, GPIOD_LED6);
          }
        }

        /*
         * Set mute request received.
         */
        if (evt & AUDIO_EVENT_MUTE) {
        	/*
          if (audio.mute[0])
            palSetPad(GPIOD, GPIOD_LED4);
          else
            palClearPad(GPIOD, GPIOD_LED4);
          if (audio.mute[1])
            palSetPad(GPIOD, GPIOD_LED5);
          else
            palClearPad(GPIOD, GPIOD_LED5);
*/
          //audio_dac_update_mute(&audio);
        	set_mute(audio.mute[0]);
        }

        /*
         * Set volume request received.
         */
        if (evt & AUDIO_EVENT_VOLUME) {
        	uint8_t vol = audio.volume[0] / 128;

        	volatile float vol_in_db = (float)audio.volume[0] * 0.0039;
        	if (vol_in_db > 0) {
        		set_vol(0xff); // max
        	}
        	if (vol_in_db < -120) {
        		set_vol(14); // mute
        		return;
        	}
        	vol = 255 + (vol_in_db * 2);
        	set_vol(vol);
          //audio_dac_update_volume(&audio);
        }
      }




  /*
   * Normal main() thread activity, in this demo it just performs
   * a shell respawn upon its termination.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
