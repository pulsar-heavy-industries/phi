#include "phi_lib/phi_lib.h"
#include "usbcfg.h"
#include "lcd.h"
#include "codec.h"
#include "narvi_midi.h"

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

static THD_WORKING_AREA(lcd_thread_wa, 1024);
static THD_FUNCTION(lcd_thread, arg) {
	(void)arg;
	chRegSetThreadName("blinker1");
	const char vu_map[][9] = {
		"\x01       ",
		"\x02       ",
		"\x03       ",
		"\x04       ",
		"\x05       ",
		"\x05\x01      ",
		"\x05\x02      ",
		"\x05\x03      ",
		"\x05\x04      ",
		"\x05\x05      ",
		"\x05\x05\x01     ",
		"\x05\x05\x02     ",
		"\x05\x05\x03     ",
		"\x05\x05\x04     ",
		"\x05\x05\x05     ",
		"\x05\x05\x05\x01    ",
		"\x05\x05\x05\x02    ",
		"\x05\x05\x05\x03    ",
		"\x05\x05\x05\x04    ",
		"\x05\x05\x05\x05    ",
		"\x05\x05\x05\x05\x01   ",
		"\x05\x05\x05\x05\x02   ",
		"\x05\x05\x05\x05\x03   ",
		"\x05\x05\x05\x05\x04   ",
		"\x05\x05\x05\x05\x05   ",
		"\x05\x05\x05\x05\x05\x01  ",
		"\x05\x05\x05\x05\x05\x02  ",
		"\x05\x05\x05\x05\x05\x03  ",
		"\x05\x05\x05\x05\x05\x04  ",
		"\x05\x05\x05\x05\x05\x05  ",
		"\x05\x05\x05\x05\x05\x05\x01 ",
		"\x05\x05\x05\x05\x05\x05\x02 ",
		"\x05\x05\x05\x05\x05\x05\x03 ",
		"\x05\x05\x05\x05\x05\x05\x04 ",
		"\x05\x05\x05\x05\x05\x05\x05 ",
		"\x05\x05\x05\x05\x05\x05\x05\x01",
		"\x05\x05\x05\x05\x05\x05\x05\x02",
		"\x05\x05\x05\x05\x05\x05\x05\x03",
		"\x05\x05\x05\x05\x05\x05\x05\x04",
		"\x05\x05\x05\x05\x05\x05\x05\x05",
	};

	uint8_t fVULevelL = 0, fVULevelR = 0;
	char buf[20];
	uint32_t cnt = 0;

	/* TODO cleanup this mess */
	while (true) {
	  memset(buf, 0, sizeof(buf));


	  uint32_t vuLevel = HAL_SAI_GetVULevels();
	  uint16_t new_l = (vuLevel >> 16);// >> 12;
	  uint16_t new_r = (vuLevel & 0xFFFF);// >> 12;
		if (fVULevelL < new_l)
		{
			fVULevelL = new_l;
		}
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
		{
			fVULevelR = new_r;
		}
		else
		{
			if (fVULevelR > 0)
				fVULevelR--;
			if (fVULevelR > 0)
						fVULevelR--;
			if (fVULevelR > 0)
						fVULevelR--;
		}

		++cnt;

		if (cnt == 10) {
		  lcdReturnHome(&LCDD1);

		  if (USBD2.state == USB_ACTIVE)
		  {
			  chsnprintf(buf, sizeof(buf) - 1, "L [%s]    ", vu_map[phi_lib_map(fVULevelL * 2, 0, 0xFF, 0, PHI_ARRLEN(vu_map))]);
			  lcdWriteString(&LCDD1, buf, 0);

			  chsnprintf(buf, sizeof(buf) - 1, "R [%s]    ", vu_map[phi_lib_map(fVULevelR * 2, 0, 0xFF, 0, PHI_ARRLEN(vu_map))]);
			  lcdWriteString(&LCDD1, buf, 40);
		  }
		  else
		  {
			  lcdWriteString(&LCDD1, "PHI Narvi       ", 0);
			  lcdWriteString(&LCDD1, "USB Disconnected", 40);
		  }

		  cnt = 0;
		}

	  chThdSleepMicroseconds(100);
  }
}

/* Create custom characters for nicer VU muter */
void lcd_create_custom_chars(void)
{
	uint8_t p1[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, };
	uint8_t p2[8] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, };
	uint8_t p3[8] = {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, };
	uint8_t p4[8] = {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, };
	uint8_t p5[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, };

	lcdCreateChar(&LCDD1, 1, p1);
	lcdCreateChar(&LCDD1, 2, p2);
	lcdCreateChar(&LCDD1, 3, p3);
	lcdCreateChar(&LCDD1, 4, p4);
	lcdCreateChar(&LCDD1, 5, p5);
}


/******************************************************************************
 * blinker
 *****************************************************************************/

static THD_WORKING_AREA(blinker_thread_wa, 128);
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

extern audio_state_t audio;

int main(void)
{
	char buf[21];
    static event_listener_t listener;

	halInit();
	chSysInit();

    lcdInit();
    palSetLineMode(LINE_RS, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    palSetLineMode(LINE_E, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

    memset(buf, 0, sizeof(buf));
    chsnprintf(buf, sizeof(buf) - 1, "Hw:%x Sw:%x", PHI_BL_HW_VER, PHI_BL_SW_VER);
    lcdStart(&LCDD1, &lcdcfg);
    lcd_create_custom_chars();
    lcdWriteString(&LCDD1, "PHI Narvi", 0);
    lcdWriteString(&LCDD1, buf, 40);

    // MIDI is on SD6, baudrate is configured in hal.conf (SERIAL_DEFAULT_BITRATE)
    sdStart(&SD6, NULL);

    narvi_midi_init();
    mduObjectInit(&MDU1);
    mduStart(&MDU1, &midiusbcfg);

    audioObjectInit(&audio);

    usbDisconnectBus(midiusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(midiusbcfg.usbp, &usbcfg);
    usbConnectBus(midiusbcfg.usbp);

    codec_init();
    chThdCreateStatic(lcd_thread_wa, sizeof(lcd_thread_wa), NORMALPRIO + 10, lcd_thread, NULL);
    chThdCreateStatic(blinker_thread_wa, sizeof(blinker_thread_wa), NORMALPRIO + 10, blinker_thread, NULL);

    chEvtRegisterMask(&audio.audio_events, &listener, AUDIO_EVENT);

    for (;;)
    {
        /* Wait for audio event */
        chEvtWaitOne(AUDIO_EVENT);
        eventflags_t evt = chEvtGetAndClearFlags(&listener);

        /* USB state changed */
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
        		start_sof_capture();
        	} else {
        		stop_sof_capture();
        		usb_reset_audio_bufs();
        	}
        }

        /* Set mute request received. */
        if (evt & AUDIO_EVENT_MUTE) {
        	codec_set_mute(audio.mute[0]);
        }

        /* Set volume request received. */
        if (evt & AUDIO_EVENT_VOLUME) {
        	uint8_t vol = audio.volume[0] / 128;

        	volatile float vol_in_db = (float)audio.volume[0] * 0.0039;
        	if (vol_in_db > 0) {
        		codec_set_vol(0xff); // max
        	}
        	if (vol_in_db < -120) {
        		codec_set_vol(14); // mute
        	} else {
        		vol = 255 + (vol_in_db * 2);
        		codec_set_vol(vol);
        	}
        }
    }
}
