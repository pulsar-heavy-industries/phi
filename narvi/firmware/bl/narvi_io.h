#ifndef NARVI_IO_H_
#define NARVI_IO_H_

#include "phi_lib/phi_rotenc.h"
#include "phi_lib/phi_btn.h"

typedef enum {
	NARVI_IO_MENU_BL_STATUS = 0,
	NARVI_IO_MENU_VERSION,
	NARVI_IO_MENU_EXIT,
	NARVI_IO_MENU_LEN,
} narvi_io_menu_t;

extern phi_rotenc_t narvi_rotencs[2];
extern phi_btn_t narvi_btns[2];

void narvi_io_init(void);

#endif /* NARVI_IO_H_ */
