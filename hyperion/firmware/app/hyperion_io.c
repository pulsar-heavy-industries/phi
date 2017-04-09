#include "hyperion_io.h"
#include "phi_lib/phi_app_mgr.h"

phi_rotenc_t rotencs[2];
phi_btn_t btns[10];

static bool force_pots_read = FALSE;

#define ADC_GRP2_NUM_CHANNELS   8
#define ADC_GRP2_BUF_DEPTH      16
adcsample_t adc_data[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
uint32_t adc_data_filtered[ADC_GRP2_NUM_CHANNELS] = {0, };
uint8_t adc_data_lowres[ADC_GRP2_NUM_CHANNELS] = {0, };
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
    int i, j;

    (void) adcp; (void) buffer; (void) n;

    for (i = 0; i < ADC_GRP2_NUM_CHANNELS; ++i)
    {
        adc_data_filtered[i] = 0;
        for (j = 0; j < ADC_GRP2_BUF_DEPTH; ++j)
        {
            int idx = (j * ADC_GRP2_NUM_CHANNELS) + i;
            adc_data_filtered[i] += adc_data[idx];
        }
        adc_data_filtered[i] /= ADC_GRP2_BUF_DEPTH;
    }
}


static const ADCConversionGroup adcgrpcfg2 = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  adccallback, // adccallback,
  NULL, // adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  0,                        /* SMPR2 */
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_480) | ADC_SMPR2_SMP_AN1(ADC_SAMPLE_480) |
  ADC_SMPR2_SMP_AN2(ADC_SAMPLE_480) | ADC_SMPR2_SMP_AN3(ADC_SAMPLE_480) |
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_480) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_480) |
  ADC_SMPR2_SMP_AN6(ADC_SAMPLE_480) | ADC_SMPR2_SMP_AN7(ADC_SAMPLE_480),
  ADC_SQR1_NUM_CH(ADC_GRP2_NUM_CHANNELS),
  ADC_SQR2_SQ8_N(ADC_CHANNEL_IN6) | ADC_SQR2_SQ7_N(ADC_CHANNEL_IN5),
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN4)   | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN7) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN2)   | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN1) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN3)   | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)
};




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

uint32_t data_out = 0;

static void rotenc_callback(uint8_t rotenc, int8_t value)
{
	phi_app_mgr_notify_encoder_event(0, rotenc, value);
}

static void btn_callback(uint8_t btn, phi_btn_event_t event, uint32_t param)
{
	phi_app_mgr_notify_btn_event(0, btn, event, param);
#if 0
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
#endif
}

static THD_WORKING_AREA(hyperion_io_thread_wa, 512);
static THD_FUNCTION(hyperion_io_thread, arg) {
    systime_t now;
    uint32_t data_in = 0;
    int i;

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

        for (i = 0; i < ADC_GRP2_NUM_CHANNELS; ++i)
        {
            uint8_t lowres = adc_data_filtered[i] >> 4;
            int delta = adc_data_lowres[i] - lowres;
            if ((delta > 2) || (delta < -2) || force_pots_read)
            {
                adc_data_lowres[i] = lowres;
                phi_app_mgr_notify_pot_event(0, i, lowres);
            }
        }

        force_pots_read = FALSE;

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

    adcStart(&ADCD1, NULL);
    adcStartConversion(&ADCD1, &adcgrpcfg2, adc_data, ADC_GRP2_BUF_DEPTH);
}

void hyperion_io_set_btn_leds(uint8_t btn, uint8_t leds)
{
    const uint32_t red[] = {
		1 << 8,		// BTN 1 BLUE
		1 << 9,		// BTN 2 BLUE
		1 << 12,	// BTN 3 RED
		1 << 10,	// BTN 4 RED
		1 << 2,		// BTN 5 RED
		1 << 1,		// BTN 6 RED
		1 << 6,		// BTN 7 RED
		1 << 5,		// BTN 8 RED
    };
    const uint32_t green[] = {
		1 << 8,		// BTN 1 BLUE
		1 << 9,		// BTN 2 BLUE
		1 << 13,	// BTN 3 GREEN
		1 << 11,	// BTN 4 GREEN
		1 << 3,		// BTN 5 GREEN
		1 << 4,		// BTN 6 GREEN
		1 << 7,		// BTN 7 GREEN
		1 << 0,		// BTN 8 GREEN
    };

    chDbgCheck(btn < 8);

    chSysLock();
    data_out &= ~(red[btn] | green[btn]);
    if (leds & HYPERION_IO_BTN_LED_RED)
    {
        data_out |= red[btn];
    }
    if (leds & HYPERION_IO_BTN_LED_GREEN)
    {
        data_out |= green[btn];
    }
    chSysUnlock();
}

void hyperion_io_set_bar_graph(uint8_t val, uint8_t fill)
{
    chDbgCheck(val < 11);
    chSysLock();
    data_out &= ~0xffc000;
    if (val)
    {
        val--;

        if (fill)
        {
            data_out |= ((1 << (val + 1)) - 1) << 14;
        }
        else
        {
            data_out |= (1 << val) << 14;
        }
    }
    chSysUnlock();
}

void hyperion_io_force_pots_read(void)
{
	force_pots_read = TRUE;
}
