#include "codec.h"
#include "usbcfg.h"

/* SPI configuration for PCM1792A */
const SPIConfig dac_main_spi_cfg = {
	NULL,
	GPIOD,
	GPIOD_DAC_CS,
	SPI_CR1_BR_1 | SPI_CR1_BR_2,// | SPI_CR1_CPOL |SPI_CR1_CPHA,
};

extern int32_t dac_buffer[DAC_AUDIO_BUFFER_SIZE + DAC_AUDIO_MAX_PACKET_SIZE];
extern int32_t dac_buffer2[DAC_AUDIO_BUFFER_SIZE + DAC_AUDIO_MAX_PACKET_SIZE];

const stm32_dma_stream_t * sai_dma1 = NULL, * sai_dma2 = NULL;

static void dma_sai_interrupt(void * dat, uint32_t flags)
{
	dmaStreamClearInterrupt(sai_dma1);
}


void codec_init(void)
{
	uint32_t sai_tx_dma_mode;

	if (sai_dma1 || sai_dma2)
	{
		return;
	}

	/* PLLSAI activation.*/
	RCC->PLLSAICFGR = (STM32_PLLSAIN_VALUE << 6) | (STM32_PLLSAIR_VALUE << 28) | (STM32_PLLSAIQ_VALUE << 24);
	RCC->DCKCFGR = 0;
	RCC->CR |= RCC_CR_PLLSAION;

	while (!(RCC->CR & RCC_CR_PLLSAIRDY));

	rccEnableAPB2(RCC_APB2ENR_SAI1EN, FALSE);

	/* GPIO configuration */

	// MEASURED BCK=3.0717MHz MCK=12.287MHz
	palSetPadMode(GPIOE, 2, PAL_MODE_ALTERNATE(6)); // A MCK
	palSetPadMode(GPIOE, 4, PAL_MODE_ALTERNATE(6)); // A FS
	palSetPadMode(GPIOE, 5, PAL_MODE_ALTERNATE(6)); // A SCK
	palSetPadMode(GPIOE, 6, PAL_MODE_ALTERNATE(6)); // A SD
	palSetPadMode(GPIOE, 3, PAL_MODE_ALTERNATE(6)); // B SD


	/* SAI Block A initialization */
	SAI1_Block_A->FRCR = 0;
	SAI1_Block_A->FRCR = 0
		| SAI_xFRCR_FSDEF // FS signal is a start of frame signal + channel side identification
		| SAI_xFRCR_FSOFF // FS is asserted one bit before the first bit of the slot 0.
		| SAI_xFRCR_FSPO
		| 63
		| (31 << 8)
		;

	SAI1_Block_A->CR1 = 0;
	SAI1_Block_A->CR1 |= 0
		| SAI_xCR1_DS_1 |  SAI_xCR1_DS_2 // 24 bit data size
//		| SAI_xCR1_DS_2 // 16 bit data size
//		| SAI_xCR1_DS_0 | SAI_xCR1_DS_1 | SAI_xCR1_DS_2  // 32 bit
//		| SAI_xCR1_CKSTR
//		| SAI_xCR1_LSBFIRST
		;
	SAI1_Block_A->CR1 |= SAI_xCR1_OUTDRIV | SAI_xCR1_MCKDIV_1;
	SAI1_Block_A->CR2 = SAI_xCR2_FTH_0;

    SAI1_Block_A->SLOTR = 0
//      | SAI_xSLOTR_SLOTSZ_0 // 16 bit slot data size
    	| SAI_xSLOTR_SLOTSZ_1 // 32 bit slot size
    	// NO SLOTSZ = 00: The slot size is equivalent to the data size (specified in DS[3:0] in the SAI_ACR1 register).
		| SAI_xSLOTR_NBSLOT_0 // 1 +1 slot (2 slots)
		| (3 << 16) // slot 0 + 1
//		| 8 // first bit offset
		;

    /* SAI Block B initialization */
    SAI1_Block_B->FRCR = 0;
    SAI1_Block_B->FRCR = 0
		| SAI_xFRCR_FSDEF // FS signal is a start of frame signal + channel side identification
		| SAI_xFRCR_FSOFF // FS is asserted one bit before the first bit of the slot 0.
		| SAI_xFRCR_FSPO
//  	| 31 // Frame length (32)
//  	| (15 << 8)
  		| 63
  		| (31 << 8)
  		;

    SAI1_Block_B->CR1 = 0;
    SAI1_Block_B->CR1 |= 0
    	| SAI_xCR1_SYNCEN_0 // 01: audio block is synchronous with the other internal audio block
		| SAI_xCR1_MODE_1 // 10: Slave transmitter
  		| SAI_xCR1_DS_1 |  SAI_xCR1_DS_2 // 24 bit data size
//  	| SAI_xCR1_DS_2 // 16 bit data size
  //	| SAI_xCR1_DS_0 | SAI_xCR1_DS_1 | SAI_xCR1_DS_2  // 32 bit
  //	| SAI_xCR1_CKSTR
  //	| SAI_xCR1_LSBFIRST
  		;
    SAI1_Block_B->CR1 |= SAI_xCR1_OUTDRIV | SAI_xCR1_MCKDIV_1;
    SAI1_Block_B->CR2 = SAI_xCR2_FTH_0;

    SAI1_Block_B->SLOTR = 0
//      | SAI_xSLOTR_SLOTSZ_0 // 16 bit slot data size
      	| SAI_xSLOTR_SLOTSZ_1 // 32 bit slot size
      	// NO SLOTSZ = 00: The slot size is equivalent to the data size (specified in DS[3:0] in the SAI_ACR1 register).
  		| SAI_xSLOTR_NBSLOT_0 // 1 +1 slot (2 slots)
  		| (3 << 16) // slot 0 + 1
  //	| 8 // first bit offset
  		;

    /* DMA Initialization */
	#define STM32_SAI1_A_DMA_CHN        0x00000000
	#define STM32_SAI1_B_DMA_CHN        0x00000000
	#define STM32_SAI_SAI1_DMA_PRIORITY 1
    #define SAI_A_DMA_CHANNEL 0
    #define SAI_B_DMA_CHANNEL 1

    chDbgCheck(DAC_AUDIO_BUFFER_SIZE < 0xFFFF);

    // DMA 1
    sai_dma1 = STM32_DMA_STREAM(STM32_SAI_SAI1_A_DMA_STREAM);

    sai_tx_dma_mode = STM32_DMA_CR_CHSEL(SAI_A_DMA_CHANNEL)
  		| STM32_DMA_CR_PL(STM32_SAI_SAI1_DMA_PRIORITY) | STM32_DMA_CR_DIR_M2P
//    	| STM32_DMA_CR_TEIE | STM32_DMA_CR_TCIE | STM32_DMA_CR_DBM  // double buffer mode
		| STM32_DMA_CR_CIRC
//    	| STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_WORD;
		| STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD;

    dmaStreamAllocate(sai_dma1, SAI_A_DMA_CHANNEL, (stm32_dmaisr_t) dma_sai_interrupt, (void *) SAI1_Block_A);
    dmaStreamSetPeripheral(sai_dma1, &(SAI1_Block_A->DR));
    dmaStreamSetMemory0(sai_dma1, dac_buffer);
    // dmaStreamSetMemory1(i2sdma, &(dac_buffer[DAC_AUDIO_BUFFER_SIZE / 2] ));
    dmaStreamSetTransactionSize(sai_dma1, DAC_AUDIO_BUFFER_SIZE);
    dmaStreamSetMode(sai_dma1, sai_tx_dma_mode | STM32_DMA_CR_MINC);
    dmaStreamClearInterrupt(sai_dma1);
    dmaStreamEnable(sai_dma1);

    // DMA 2
    sai_dma2 = STM32_DMA_STREAM(STM32_SAI_SAI1_B_DMA_STREAM);

    sai_tx_dma_mode = STM32_DMA_CR_CHSEL(SAI_B_DMA_CHANNEL)
    	| STM32_DMA_CR_PL(STM32_SAI_SAI1_DMA_PRIORITY) | STM32_DMA_CR_DIR_M2P
//    	| STM32_DMA_CR_TEIE | STM32_DMA_CR_TCIE | STM32_DMA_CR_DBM  // double buffer mode
		| STM32_DMA_CR_CIRC
//    	| STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_WORD;
		| STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD;

    dmaStreamAllocate(sai_dma2, SAI_B_DMA_CHANNEL, (stm32_dmaisr_t) dma_sai_interrupt, (void *) SAI1_Block_B);
    dmaStreamSetPeripheral(sai_dma2, &(SAI1_Block_B->DR));
    dmaStreamSetMemory0(sai_dma2, dac_buffer2);
    //dmaStreamSetMemory1(i2sdma, &(dac_buffer[DAC_AUDIO_BUFFER_SIZE / 2] ));
    dmaStreamSetTransactionSize(sai_dma2, DAC_AUDIO_BUFFER_SIZE);
    dmaStreamSetMode(sai_dma2, sai_tx_dma_mode | STM32_DMA_CR_MINC);
    dmaStreamClearInterrupt(sai_dma2);
    dmaStreamEnable(sai_dma2);

    /* Enable SAI1 */
	SAI1_Block_B->CR1 |= SAI_xCR1_DMAEN;
	SAI1_Block_B->CR1 |= SAI_xCR1_SAIEN;

	SAI1_Block_A->CR1 |= SAI_xCR1_DMAEN;
	SAI1_Block_A->CR1 |= SAI_xCR1_SAIEN;
}

void codec_deinit(void)
{
	/* Disable SAI1 */
	SAI1_Block_B->CR1 &= ~SAI_xCR1_DMAEN;
	SAI1_Block_B->CR1 &= ~SAI_xCR1_SAIEN;

	SAI1_Block_A->CR1 &= ~SAI_xCR1_DMAEN;
	SAI1_Block_A->CR1 &= ~SAI_xCR1_SAIEN;

	/* Deinit DMA */
	if (sai_dma2) {
		dmaStreamDisable(sai_dma2);
		dmaStreamRelease(sai_dma2);
		sai_dma2 = NULL;
	}

	if (sai_dma1) {
		dmaStreamDisable(sai_dma1);
		dmaStreamRelease(sai_dma1);
		sai_dma1 = NULL;
	}

}

void codec_set_vol(uint8_t vol)
{
	uint8_t buf[2];

	spiAcquireBus(&SPID3);
	spiStart(&SPID3, &dac_main_spi_cfg);

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
	buf[1] = 0x50 | 0x80; // 0x50 default | 0x80 = Attenuation Load Control
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

	spiAcquireBus(&SPID3);
	spiStart(&SPID3, &dac_main_spi_cfg);

	buf[0] = 18;
	buf[1] = 0x50 | (mute ? 1 : 0); // 0x50 default | 1 = set mute
	spiSelect(&SPID3);
	spiSend(&SPID3, 2, buf);
	spiUnselect(&SPID3);

	spiReleaseBus(&SPID3);
}
