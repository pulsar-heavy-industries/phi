#include "codec.h"
#include "usbcfg.h"

extern int32_t dac_buffer[DAC_AUDIO_BUFFER_SIZE + DAC_AUDIO_MAX_PACKET_SIZE];
extern int32_t dac_buffer2[DAC_AUDIO_BUFFER_SIZE + DAC_AUDIO_MAX_PACKET_SIZE];


void codec_init(void) {
  codec_CS43L22_i2s_init_48k();
  codec_CS43L22_hw_init();

}

void codecStop(void) {
}

void codec_clearbuffer(void) {
  int i;
  for(i=0;i<AB_BUFSIZE*2;i++){
    buf[i]=0;
    buf2[i]=0;
  }
}


const stm32_dma_stream_t * sai_dma1, * sai_dma2;
static uint32_t i2stxdmamode = 0;
static uint32_t i2stxdmamode2 = 0;


void codec_set_vol(uint8_t vol)
{
  uint8_t buf[2];

  static SPIConfig spi_cfg = {
  	NULL,
  	GPIOD,
	GPIOD_DAC_CS,
	SPI_CR1_BR_1 | SPI_CR1_BR_2,// | SPI_CR1_CPOL |SPI_CR1_CPHA,
  };

  spiAcquireBus(&SPID3);
  spiStart(&SPID3, &spi_cfg);

  buf[0] = 16;
  buf[1] = vol;
  spiSelect(&SPID3);
  spiSend(&SPID3, 2, buf);
  spiUnselect(&SPID3);

  buf[0] = 17;
  buf[1] = vol;
  spiSelect(&SPID3);
  spiSend(&SPID3, 2, buf);
  spiUnselect(&SPID3);

  buf[0] = 18;
  buf[1] = 0x50 | 0x80; // 0x50 default | 0x80 = set mute
  spiSelect(&SPID3);
  spiSend(&SPID3, 2, buf);
  spiUnselect(&SPID3);

  buf[0] = 18;
  buf[1] = 0x50;
  spiSelect(&SPID3);
  spiSend(&SPID3, 2, buf);
  spiUnselect(&SPID3);

  spiReleaseBus(&SPID3);
}

void codec_set_mute(bool mute)
{
  uint8_t buf[2];

  static SPIConfig spi_cfg = {
  	NULL,
  	GPIOC,
	8,
	SPI_CR1_BR_1 | SPI_CR1_BR_2,// | SPI_CR1_CPOL |SPI_CR1_CPHA,
  };

  spiAcquireBus(&SPID3);
  spiStart(&SPID3, &spi_cfg);

  buf[0] = 18;
  buf[1] = 0x50 | (mute ? 1 : 0); // 0x50 default | 1 = set mute
  spiSelect(&SPID3);
  spiSend(&SPID3, 2, buf);
  spiUnselect(&SPID3);

  spiReleaseBus(&SPID3);
}


void codec_CS43L22_hw_init(void) {
//  palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
//  palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
//

//  i2cStop(&CODEC_I2C);

}


static void dma_sai_interrupt(void * dat, uint32_t flags)
{
	dmaStreamClearInterrupt(sai_dma1);
}


#define M_PI  3.14159265358979323846f
#define FREQUENCY 440.0f // that extra ".0" is important
#define SAMPLES  48000
#define PERIOD (1.0f / FREQUENCY)
#define TIME_STEP  (1.0f / (float)SAMPLES);




volatile SAI_Block_TypeDef * blk_a = SAI1_Block_A;
volatile SAI_Block_TypeDef * blk_b = SAI1_Block_B;

//#include "pcm.h"
//#include "pcm2.h"


void codec_CS43L22_i2s_init_48k(void) {
  ///////////////////////////////// SAI1

#define STM32_SAISRC_NOCLOCK    (0 << 23)   /**< No clock.                  */
#define STM32_SAISRC_PLL        (1 << 23)   /**< SAI_CKIN is PLL.           */
#define STM32_SAIR_DIV2         (0 << 16)   /**< R divided by 2.            */
#define STM32_SAIR_DIV4         (1 << 16)   /**< R divided by 4.            */
#define STM32_SAIR_DIV8         (2 << 16)   /**< R divided by 8.            */
#define STM32_SAIR_DIV16        (3 << 16)   /**< R divided by 16.           */

#define STM32_PLLSAIR_VALUE                 4
//#define STM32_PLLSAIR_POST       STM32_SAIR_DIV4



/* PLLSAI activation.*/
RCC->PLLSAICFGR = (STM32_PLLSAIN_VALUE << 6) | (STM32_PLLSAIR_VALUE << 28) | (STM32_PLLSAIQ_VALUE << 24);
//RCC->DCKCFGR = (RCC->DCKCFGR & ~RCC_DCKCFGR_PLLSAIDIVR) | STM32_PLLSAIR_POST;
RCC->DCKCFGR = 0;
RCC->CR |= RCC_CR_PLLSAION;

while (!(RCC->CR & RCC_CR_PLLSAIRDY));

rccEnableAPB2(RCC_APB2ENR_SAI1EN, FALSE);


       // MEASURED BCK=3.0717MHz MCK=12.287MHz
  palSetPadMode(GPIOE, 2, PAL_MODE_ALTERNATE(6)); // A MCK
  palSetPadMode(GPIOE, 4, PAL_MODE_ALTERNATE(6)); // A FS
  palSetPadMode(GPIOE, 5, PAL_MODE_ALTERNATE(6)); // A SCK
  palSetPadMode(GPIOE, 6, PAL_MODE_ALTERNATE(6)); // A SD
  palSetPadMode(GPIOE, 3, PAL_MODE_ALTERNATE(6)); // B SD


  volatile SAI_TypeDef * sai = SAI1;

  blk_a->FRCR = 0;
  blk_a->FRCR = 0
		  | SAI_xFRCR_FSDEF // FS signal is a start of frame signal + channel side identification
		  | SAI_xFRCR_FSOFF // FS is asserted one bit before the first bit of the slot 0.
		  | SAI_xFRCR_FSPO
//		  | 31 // Frame length (32)
//		  | (15 << 8)
		  | 63
		  | (31 << 8)
		  ;
  blk_a->CR1 = 0;
  blk_a->CR1 |= 0
		  | SAI_xCR1_DS_1 |  SAI_xCR1_DS_2 // 24 bit data size
//		  | SAI_xCR1_DS_2 // 16 bit data size
//		  | SAI_xCR1_DS_0 | SAI_xCR1_DS_1 | SAI_xCR1_DS_2  // 32 bit
//		  | SAI_xCR1_CKSTR
//		  | SAI_xCR1_LSBFIRST
		  ;
  blk_a->CR1 |= SAI_xCR1_OUTDRIV | SAI_xCR1_MCKDIV_1;
  blk_a->CR2 = SAI_xCR2_FTH_0;

    blk_a->SLOTR = 0
//    		| SAI_xSLOTR_SLOTSZ_0 // 16 bit slot data size
    		| SAI_xSLOTR_SLOTSZ_1 // 32 bit slot size
    		// NO SLOTSZ = 00: The slot size is equivalent to the data size (specified in DS[3:0] in the SAI_ACR1 register).
			| SAI_xSLOTR_NBSLOT_0 // 1 +1 slot (2 slots)

			| (3 << 16) // slot 0 + 1
//			| 8 // first bit offset
			;

    /////////

    blk_b->FRCR = 0;
    blk_b->FRCR = 0
  		  | SAI_xFRCR_FSDEF // FS signal is a start of frame signal + channel side identification
  		  | SAI_xFRCR_FSOFF // FS is asserted one bit before the first bit of the slot 0.
  		  | SAI_xFRCR_FSPO
//  		  | 31 // Frame length (32)
//  		  | (15 << 8)
  		  | 63
  		  | (31 << 8)
  		  ;
    blk_b->CR1 = 0;
    blk_b->CR1 |= 0
    		| SAI_xCR1_SYNCEN_0 // 01: audio block is synchronous with the other internal audio block
			| SAI_xCR1_MODE_1 // 10: Slave transmitter
  		  | SAI_xCR1_DS_1 |  SAI_xCR1_DS_2 // 24 bit data size
//  		  | SAI_xCR1_DS_2 // 16 bit data size
  //		  | SAI_xCR1_DS_0 | SAI_xCR1_DS_1 | SAI_xCR1_DS_2  // 32 bit
  //		  | SAI_xCR1_CKSTR
  //		  | SAI_xCR1_LSBFIRST
  		  ;
    blk_b->CR1 |= SAI_xCR1_OUTDRIV | SAI_xCR1_MCKDIV_1;
    blk_b->CR2 = SAI_xCR2_FTH_0;

      blk_b->SLOTR = 0
//      		| SAI_xSLOTR_SLOTSZ_0 // 16 bit slot data size
      		| SAI_xSLOTR_SLOTSZ_1 // 32 bit slot size
      		// NO SLOTSZ = 00: The slot size is equivalent to the data size (specified in DS[3:0] in the SAI_ACR1 register).
  			| SAI_xSLOTR_NBSLOT_0 // 1 +1 slot (2 slots)

  			| (3 << 16) // slot 0 + 1
  //			| 8 // first bit offset
  			;

#if 1
    /////// DMA
#define STM32_SAI1_A_DMA_CHN                0x00000000
#define STM32_SAI1_B_DMA_CHN                0x00000000

#define SAI_A_DMA_CHANNEL 0

      #define SAI_B_DMA_CHANNEL 1



#define STM32_SAI_SAI1_DMA_PRIORITY 1
// DMA 1
sai_dma1 = STM32_DMA_STREAM(STM32_SAI_SAI1_A_DMA_STREAM);

i2stxdmamode2 = STM32_DMA_CR_CHSEL(SAI_A_DMA_CHANNEL)
    | STM32_DMA_CR_PL(STM32_SAI_SAI1_DMA_PRIORITY) | STM32_DMA_CR_DIR_M2P
//    | STM32_DMA_CR_TEIE | STM32_DMA_CR_TCIE | STM32_DMA_CR_DBM  // double buffer mode
    | STM32_DMA_CR_CIRC
//    | STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_WORD;
    | STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD;

dmaStreamAllocate(sai_dma1, SAI_A_DMA_CHANNEL,
                  (stm32_dmaisr_t)dma_sai_interrupt, (void *)blk_a);

dmaStreamSetPeripheral(sai_dma1, &(blk_a->DR));
dmaStreamSetMemory0(sai_dma1, dac_buffer);
//dmaStreamSetMemory1(i2sdma, &(dac_buffer[DAC_AUDIO_BUFFER_SIZE / 2] ));
chDbgCheck(DAC_AUDIO_BUFFER_SIZE < 0xFFFF);
dmaStreamSetTransactionSize(sai_dma1, DAC_AUDIO_BUFFER_SIZE);// * 4);

//  dmaStreamSetMemory0(i2sdma, buf);
//  dmaStreamSetMemory1(i2sdma, buf2);
//  dmaStreamSetTransactionSize(i2sdma, sizeof(buf) / 2);
dmaStreamSetMode(sai_dma1, i2stxdmamode2 | STM32_DMA_CR_MINC);
dmaStreamClearInterrupt(sai_dma1);
dmaStreamEnable(sai_dma1);

// DMA 2
sai_dma2 = STM32_DMA_STREAM(STM32_SAI_SAI1_B_DMA_STREAM);

i2stxdmamode2 = STM32_DMA_CR_CHSEL(SAI_B_DMA_CHANNEL)
    | STM32_DMA_CR_PL(STM32_SAI_SAI1_DMA_PRIORITY) | STM32_DMA_CR_DIR_M2P
//    | STM32_DMA_CR_TEIE | STM32_DMA_CR_TCIE | STM32_DMA_CR_DBM  // double buffer mode
    | STM32_DMA_CR_CIRC
//    | STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_WORD;
    | STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD;

dmaStreamAllocate(sai_dma2, SAI_B_DMA_CHANNEL,
                  (stm32_dmaisr_t)dma_sai_interrupt, (void *)blk_b);

dmaStreamSetPeripheral(sai_dma2, &(blk_b->DR));
dmaStreamSetMemory0(sai_dma2, dac_buffer2);
//dmaStreamSetMemory1(i2sdma, &(dac_buffer[DAC_AUDIO_BUFFER_SIZE / 2] ));
chDbgCheck(DAC_AUDIO_BUFFER_SIZE < 0xFFFF);
dmaStreamSetTransactionSize(sai_dma2, DAC_AUDIO_BUFFER_SIZE);// * 4);

//  dmaStreamSetMemory0(i2sdma, buf);
//  dmaStreamSetMemory1(i2sdma, buf2);
//  dmaStreamSetTransactionSize(i2sdma, sizeof(buf) / 2);
dmaStreamSetMode(sai_dma2, i2stxdmamode2 | STM32_DMA_CR_MINC);
dmaStreamClearInterrupt(sai_dma2);
dmaStreamEnable(sai_dma2);

//

blk_b->CR1 |= SAI_xCR1_DMAEN; // enable;
blk_b->CR1 |= SAI_xCR1_SAIEN ; // enable;


blk_a->CR1 |= SAI_xCR1_DMAEN; // enable;
blk_a->CR1 |= SAI_xCR1_SAIEN ; // enable;
#else


  volatile int x = 0;

  while (1)
  {


	  // ~/tmp/stlink/st-flash write bdd2.raw 0x8040000
	  uint16_t * data = 0x8040000;
	  volatile int i = 0;

	  // Fill FIFO
	  while (!blk_a->SR & (SAI_xSR_FLVL_0 | SAI_xSR_FLVL_2))
	  {
		  blk_a->DR = *data; // left
		  blk_a->DR = *data; // right

		  blk_b->DR = *data; // left
		  blk_b->DR = *data; // right

		  ++data;
		  i++;
	  }


	  blk_b->CR1 |= SAI_xCR1_SAIEN; // enable;

	  blk_a->CR1 |= SAI_xCR1_SAIEN; // enable;


	  for (; i < 1334798 / 2; ++i)
	  {
		  while (blk_a->SR & (SAI_xSR_FLVL_0 | SAI_xSR_FLVL_2));
		  blk_a->DR = *data; // left

		  while (blk_b->SR & (SAI_xSR_FLVL_0 | SAI_xSR_FLVL_2));
		  blk_b->DR = *data; // left

		  while (blk_a->SR & (SAI_xSR_FLVL_0 | SAI_xSR_FLVL_2));
		  blk_a->DR = *data; // right

		  while (blk_b->SR & (SAI_xSR_FLVL_0 | SAI_xSR_FLVL_2));
		  blk_b->DR = *data; // right


		  ++data;
	  }

	  // Disable
	  blk_b->CR1 &= ~SAI_xCR1_SAIEN; // enable;
	  while (blk_b->CR1 & SAI_xCR1_SAIEN);
	  blk_b->CR2 |= SAI_xCR2_FFLUSH;


	  blk_a->CR1 &= ~SAI_xCR1_SAIEN; // enable;
	  while (blk_a->CR1 & SAI_xCR1_SAIEN);
	  blk_a->CR2 |= SAI_xCR2_FFLUSH;

	  chThdSleepMilliseconds(500);
  }
#endif
}
