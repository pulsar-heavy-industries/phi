#ifndef __CODEC_H
#define __CODEC_H

#include "phi_lib/phi_lib.h"

extern void codec_init(void);

void codec_CS43L22_hw_reset(void);
void codec_set_mute(bool mute);
void codec_set_vol(uint8_t vol);

#endif /* __CODEC_H */
