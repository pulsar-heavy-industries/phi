#include "hyperion_io.h"

phi_rotenc_t rotencs[2];
phi_btn_t btns[10];


static SPIConfig spi_cfg = {
    NULL,
    GPIOB,
    12, // TODO why this one?
    SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2
};

#define RC 3
#define CLK 13
#define DATA 15
#define DATA_IN 14
#define RC2 2
static uint32_t shift_in_out(uint32_t data) {
    uint32_t out = 0;

    palClearPad(GPIOC, RC2);
    palClearPad(GPIOC, RC);

    chThdSleepMicroseconds(250);
    palSetPad(GPIOC, RC2);

    chThdSleepMicroseconds(250);

    spiExchange(&SPID2, 3, &data, &out);

    palSetPad(GPIOC, RC);


    return out;
}

#define __delay() for (volatile int delayz = 0; delayz < 100000; ++delayz) asm("nop")
bool hyperion_io_bl_force(void)
{
	volatile uint32_t data = 0;
	volatile int i;

	palSetPadMode(GPIOB, GPIOB_SPI2_SCK, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOB, GPIOB_SPI2_MOSI, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOB, GPIOB_SPI2_MISO, PAL_MODE_INPUT);

	palClearPad(GPIOC, RC2);
	palClearPad(GPIOC, RC);

	__delay();

	palSetPad(GPIOC, RC2);

	palClearPad(GPIOB, GPIOB_SPI2_MOSI);
	palClearPad(GPIOB, GPIOB_SPI2_SCK);

	for (i = 0; i < 24; ++i)
	{
		palSetPad(GPIOB, GPIOB_SPI2_SCK);
		__delay();

		data <<= 1;
		data |= palReadPad(GPIOB, GPIOB_SPI2_MISO) ? 1 : 0;

		palClearPad(GPIOB, GPIOB_SPI2_SCK);
		__delay();
	}

	palSetPad(GPIOC, RC);

	// Back to SPI mode
	palSetPadMode(GPIOB, GPIOB_SPI2_SCK, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(GPIOB, GPIOB_SPI2_MOSI, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(GPIOB, GPIOB_SPI2_MISO, PAL_MODE_ALTERNATE(5));

	// ENC1 button
	return data & (1 << 19) ? FALSE: TRUE;
}

uint32_t data_out = 0;
volatile int32_t rotz[2] = {0, 0};

static void rotenc_callback(uint8_t rotenc, int8_t value)
{
    rotz[rotenc] += value;
}

static void btn_callback(uint8_t btn, phi_btn_event_t event, uint32_t param)
{
// bar graph leds
//	1 << 22,
//	1 << 23,
//	1 << 14,
//	1 << 15,
//	1 << 16,
//	1 << 17,
//	1 << 18,
//	1 << 19,
//	1 << 20,
//	1 << 21,

    uint32_t map[] = {
    	1 << 8,		// BTN 1 BLUE
        1 << 9,		// BTN 2 BLUE
		1 << 12,	// BTN 3 RED
		1 << 10,	// BTN 4 RED
		1 << 2,		// BTN 5 RED
		1 << 1,		// BTN 6 RED
		1 << 6,		// BTN 7 RED
		1 << 5,		// BTN 8 RED
		1 << 22,    // ENC 1 BTN --> BAR GRAPH 1
		1 << 14,    // ENC 2 BTN --> BAR GRAPH 3
    };
    uint32_t map2[] = {
		1 << 8,		// BTN 1 BLUE
		1 << 9,		// BTN 2 BLUE
		1 << 13,	// BTN 3 GREEN
		1 << 11,	// BTN 4 GREEN
		1 << 3,		// BTN 5 GREEN
		1 << 4,		// BTN 6 GREEN
		1 << 7,		// BTN 7 GREEN
		1 << 0,		// BTN 8 GREEN
		1 << 23,    // ENC 1 BTN --> BAR GRAPH 2
		1 << 15,    // ENC 2 BTN --> BAR GRAPH 4
    };

    switch (event)
    {
    case PHI_BTN_EVENT_PRESSED:
        data_out |= map[btn];
        break;

    case PHI_BTN_EVENT_RELEASED:
        data_out &= ~(map[btn]);
        data_out &= ~(map2[btn]);
        break;

    case PHI_BTN_EVENT_HELD:
        data_out &= ~(map[btn]);
        data_out |= map2[btn];
        break;

    default:
    	break;
    }
}

static THD_WORKING_AREA(hyperion_io_thread_wa, 512);
static THD_FUNCTION(hyperion_io_thread, arg) {
    systime_t now;
    uint32_t data_in = 0;

    (void)arg;
    chRegSetThreadName("gpio");

    spiStart(&SPID2, &spi_cfg);
    shift_in_out(0);
    shift_in_out(0);
    shift_in_out(0);

    while (true)
    {
        now = chVTGetSystemTimeX();
        data_in = shift_in_out(data_out);

        phi_rotenc_update_state(&rotencs[0], (((data_in >> 0) & 1) << 1) | ((data_in >> 1) & 1));
        phi_rotenc_update_state(&rotencs[1], (((data_in >> 5) & 1) << 1) | ((data_in >> 6) & 1));

        phi_btn_update_state(&(btns[0]), now, (data_in >> 15) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[1]), now, (data_in >> 14) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[2]), now, (data_in >> 8) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[3]), now, (data_in >> 9) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[4]), now, (data_in >> 10) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[5]), now, (data_in >> 11) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[6]), now, (data_in >> 13) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[7]), now, (data_in >> 12) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[8]), now, (data_in >> 2) & 1 ? FALSE : TRUE);
		phi_btn_update_state(&(btns[9]), now, (data_in >> 4) & 1 ? FALSE : TRUE);

        phi_rotenc_process(rotencs, PHI_ARRLEN(rotencs), rotenc_callback);
        phi_btn_process(
            btns,
            PHI_ARRLEN(btns),
            btn_callback,
            MS2ST(20),
            MS2ST(1000),
            MS2ST(100)
        );

//        chThdSleepMicroseconds(500);
    }
}

void hyperion_io_init(void)
{
    uint32_t i;

    for (i = 0; i < PHI_ARRLEN(rotencs); ++i) {
        phi_rotenc_init(&(rotencs[i]));
    }

    for (i = 0; i < PHI_ARRLEN(btns); ++i) {
       phi_btn_init(&(btns[i]));
    }

    chThdCreateStatic(hyperion_io_thread_wa, sizeof(hyperion_io_thread_wa), NORMALPRIO, hyperion_io_thread, NULL);
}
