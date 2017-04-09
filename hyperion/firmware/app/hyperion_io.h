#ifndef HYPERION_IO_H_
#define HYPERION_IO_H_

#include "phi_lib/phi_btn.h"
#include "phi_lib/phi_rotenc.h"

#define HYPERION_IO_BTN_LED_OFF    0
#define HYPERION_IO_BTN_LED_RED    1
#define HYPERION_IO_BTN_LED_BLUE   1 // For buttons 1 and 2
#define HYPERION_IO_BTN_LED_GREEN  2
#define HYPERION_IO_BTN_LED_ORANGE HYPERION_IO_BTN_LED_RED | HYPERION_IO_BTN_LED_GREEN


void hyperion_io_init(void);
void hyperion_io_set_btn_leds(uint8_t btn, uint8_t leds);
void hyperion_io_set_bar_graph(uint8_t val, uint8_t fill);
void hyperion_io_force_pots_read(void);

#endif /* HYPERION_IO_H_ */
