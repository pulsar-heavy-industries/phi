#include "hyperion_io.h"

phi_rotenc_t rotencs[2];
phi_btn_t btns[8];


static SPIConfig spi_cfg = {
    NULL,
    GPIOD,
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
int32_t rotz[2] = {0, 0};

static void rotenc_callback(uint8_t rotenc, int8_t value)
{
    rotz[rotenc] += value;
}

static void btn_callback(uint8_t btn, phi_btn_event_t event, uint32_t param)
{
    uint32_t map[] = {
        1 << 9,
        1 << 1,
        1 << 9,
        1 << 1,
        1 << 11,
        1 << 3,
        1 << 13,
        1 << 5,
    };
    uint32_t map2[] = {
        1 << 10,
        1 << 2,
        1 << 10,
        1 << 2,
        1 << 12,
        1 << 4,
        1 << 8,
        1 << 6,
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

        phi_rotenc_update_state(&rotencs[0], (((data_in >> 8) & 1) << 1) | ((data_in >> 9) & 1));
        phi_rotenc_update_state(&rotencs[1], (((data_in >> 13) & 1) << 1) | ((data_in >> 14) & 1));


        phi_btn_update_state(&(btns[0]), now, (data_in >> 10) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[1]), now, (data_in >> 12) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[2]), now, (data_in >> 0) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[3]), now, (data_in >> 3) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[4]), now, (data_in >> 1) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[5]), now, (data_in >> 5) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[6]), now, (data_in >> 2) & 1 ? FALSE : TRUE);
        phi_btn_update_state(&(btns[7]), now, (data_in >> 4) & 1 ? FALSE : TRUE);

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
