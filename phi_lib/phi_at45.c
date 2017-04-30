#include "phi_lib/phi_at45.h"

const phi_at45_desc_t phi_at45_descs[] = {
	{0x2600, "AT45DB161D", 4096},
};

phi_at45_ret_t phi_at45_init(phi_at45_t * at45, const phi_at45_cfg_t * cfg)
{
	phi_at45_ret_t ret;
	uint8_t i;

	memset(at45, 0, sizeof(*at45));
	at45->cfg = cfg;

	spiAcquireBus(at45->cfg->spi);
	spiStart(at45->cfg->spi, &(at45->cfg->spi_cfg));

    /* Configure to 512 byte pages */
    uint8_t tx_cfg[] = {0x3D, 0x2A, 0x80, 0xA6};
	spiSelect(at45->cfg->spi);
	spiSend(at45->cfg->spi, sizeof(tx_cfg), tx_cfg);
	spiUnselect(at45->cfg->spi);

	ret = phi_at45_wait_for_rdy(at45, NULL);
	if (ret != PHI_AT45_RET_SUCCESS)
	{
		goto lbl_ret;
	}

	/* Verify we're in 512 byte mode and that density code is what we expect */
	uint8_t tx_status[] = {0xD7, };
	uint8_t rx_status[2];

	spiSelect(at45->cfg->spi);
	spiSend(at45->cfg->spi, sizeof(tx_status), tx_status);
	spiReceive(at45->cfg->spi, sizeof(rx_status), rx_status);
	spiUnselect(at45->cfg->spi);

	if (!rx_status[0] & 1)
	{
		ret = PHI_AT45_RET_NOT_512;
		goto lbl_ret;
	}

	if ((rx_status[0] & 0x3C) != 0x2C)
	{
		ret = PHI_AT45_RET_UNEXPECTED_DENSITY;
		goto lbl_ret;
	}

	/* Get device id */
	uint8_t tx_id[] = {0x9F, };
	uint8_t rx_id[5];

	spiSelect(at45->cfg->spi);
	spiSend(at45->cfg->spi, sizeof(tx_id), tx_id);
	spiReceive(at45->cfg->spi, sizeof(rx_id), rx_id);
	spiUnselect(at45->cfg->spi);

	uint16_t dev_id = rx_id[1] << 8 | rx_id[2];
	for (i = 0; i < PHI_ARRLEN(phi_at45_descs); ++i)
	{
		if (dev_id == phi_at45_descs[i].dev_id)
		{
			at45->desc = &(phi_at45_descs[i]);
			break;
		}
	}

	if (at45->desc == NULL)
	{
		ret = PHI_AT45_RET_UNKNOWN_DEVICE;
		goto lbl_ret;
	}

	/* Done */
	ret = PHI_AT45_RET_SUCCESS;

lbl_ret:
	spiReleaseBus(at45->cfg->spi);
	return ret;
}

phi_at45_ret_t phi_at45_erase_chip(phi_at45_t * at45)
{
	phi_at45_ret_t ret;
	uint16_t status;

	uint8_t tx_erase[] = {0xC7, 0x94, 0x80, 0x9A};

	spiAcquireBus(at45->cfg->spi);
	spiStart(at45->cfg->spi, &(at45->cfg->spi_cfg));

	/* Write */
	spiSelect(at45->cfg->spi);
	spiSend(at45->cfg->spi, sizeof(tx_erase), tx_erase);
	spiUnselect(at45->cfg->spi);

	/* Wait */
	ret = phi_at45_wait_for_rdy(at45, &status);
	if (ret != PHI_AT45_RET_SUCCESS)
	{
		goto lbl_ret;
	}

	if (status & (1 << 13))
	{
		ret = PHI_AT45_RET_EPE;
		goto lbl_ret;
	}

	/* Done */
	ret = PHI_AT45_RET_SUCCESS;

lbl_ret:
	spiReleaseBus(at45->cfg->spi);
	return ret;
}

phi_at45_ret_t phi_at45_page_write(phi_at45_t * at45, uint32_t addr, const void * buf, size_t len)
{
	phi_at45_ret_t ret;
	uint16_t status;

	uint8_t tx_write[] = {
		0x82, // Main Memory Page Program through Buffer 1 using the binary page size (512 bytes)
		(addr >> 16) & 0xFF,
		(addr >> 8) & 0xFF,
		addr & 0xFF,

	};
	chDbgCheck(len <= PHI_AT45_PAGE_SIZE);
	chDbgCheck((addr % PHI_AT45_PAGE_SIZE) == 0);
	chDbgCheck((addr + len) <= (at45->desc->num_pages * PHI_AT45_PAGE_SIZE));

	spiAcquireBus(at45->cfg->spi);
	spiStart(at45->cfg->spi, &(at45->cfg->spi_cfg));

	/* Write */
	spiSelect(at45->cfg->spi);
	spiSend(at45->cfg->spi, sizeof(tx_write), tx_write);
	spiSend(at45->cfg->spi, len, buf);
	spiUnselect(at45->cfg->spi);

	/* Wait */
	ret = phi_at45_wait_for_rdy(at45, &status);
	if (ret != PHI_AT45_RET_SUCCESS)
	{
		goto lbl_ret;
	}

	if (status & (1 << 13))
	{
		ret = PHI_AT45_RET_EPE;
		goto lbl_ret;
	}

	/* Done */
	ret = PHI_AT45_RET_SUCCESS;

lbl_ret:
	spiReleaseBus(at45->cfg->spi);
	return ret;
}

void phi_at45_read(phi_at45_t * at45, uint32_t addr, void * buf, size_t len)
{
	uint8_t tx_read[] = {
		0x1B, //  Continuous Array Read (High Frequency Mode: 1Bh Opcode)
		(addr >> 16) & 0xFF,
		(addr >> 8) & 0xFF,
		addr & 0xFF,
		0,
		0,
	};
	// TODO convert this to a regular check
	chDbgCheck((addr + len) <= (at45->desc->num_pages * PHI_AT45_PAGE_SIZE));

	spiAcquireBus(at45->cfg->spi);
	spiStart(at45->cfg->spi, &(at45->cfg->spi_cfg));

	/* Read */
	spiSelect(at45->cfg->spi);
	spiSend(at45->cfg->spi, sizeof(tx_read), tx_read);
	spiReceive(at45->cfg->spi, len, buf);
	spiUnselect(at45->cfg->spi);

	/* Done */
	spiReleaseBus(at45->cfg->spi);
}

/* Internal methods */
phi_at45_ret_t phi_at45_wait_for_rdy(phi_at45_t * at45, uint16_t * out_last_status)
{
	uint8_t tx[] = {0xD7, };
	uint8_t rx[2];
	systime_t start = chVTGetSystemTimeX();

	spiSelect(at45->cfg->spi);
	spiSend(at45->cfg->spi, sizeof(tx), tx);

	while ((chVTGetSystemTimeX() - start) < MS2ST(1000))
	{
		spiReceive(at45->cfg->spi, sizeof(rx), rx);
		if (rx[0] & (1 << 7))
		{
			spiUnselect(at45->cfg->spi);

			if (out_last_status != NULL)
			{
				*out_last_status = rx[0] | rx[1] << 8;
			}

			return PHI_AT45_RET_SUCCESS;
		}
	}

	spiUnselect(at45->cfg->spi);
	return PHI_AT45_RET_TIMEOUT;
}
