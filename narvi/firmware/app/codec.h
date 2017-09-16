#ifndef __CODEC_H
#define __CODEC_H
#include <stdint.h>

#define AB_BUFSIZE 128

// double buffers for DMA, interleaved stereo
extern uint16_t buf[AB_BUFSIZE * 2]; // *2 for stereo
extern uint16_t buf2[AB_BUFSIZE * 2];
extern uint16_t rbuf[AB_BUFSIZE * 2];
extern uint16_t rbuf2[AB_BUFSIZE * 2];

extern void codec_init(void);
extern void codecStop(void);

extern void computebufI(uint16_t *inp, uint16_t *outp);

void codec_clearbuffer(void);

////

#include "hal.h"

#define CODEC_I2S_ENABLE rccEnableSPI2(FALSE); rccEnableSPI3(FALSE);
#define CODEC_I2S_DISABLE rccDisableSPI2(FALSE); rccDisableSPI3(FALSE)
#define CODEC_I2S SPI2


void codec_CS43L22_hw_init(void);
void codec_CS43L22_hw_reset(void);
void codec_CS43L22_i2s_init_48k(void);
void codec_set_mute(bool mute);
void codec_set_vol(uint8_t vol);

#endif /* __CODEC_H */
