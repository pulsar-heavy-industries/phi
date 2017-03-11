#ifndef CENX4_IO_H_
#define CENX4_IO_H_

#include "phi_lib/phi_rotenc.h"
#include "phi_lib/phi_btn.h"

extern phi_rotenc_t cenx4_rotencs[4];
extern phi_btn_t cenx4_btns[4];

void cenx4_io_init(void);

#endif /* CENX4_IO_H_ */
