#ifndef NARVI_IO_H_
#define NARVI_IO_H_

#include "phi_lib/phi_rotenc.h"
#include "phi_lib/phi_btn.h"


extern phi_rotenc_t narvi_rotencs[2];
extern phi_btn_t narvi_btns[2];

void narvi_io_init(void);

#endif /* NARVI_IO_H_ */
