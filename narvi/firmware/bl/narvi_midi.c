#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_midi.h"
#include "phi_lib/phi_bl_common.h"

#include "usbcfg.h"
#include "narvi_midi.h"
#include "lcd.h"

const phi_midi_cfg_t narvi_midi_cfg = {
    .in_handler            = narvi_midi_in_handler,
    .in_sysex              = narvi_midi_in_sysex,
    .get_dev_info          = narvi_midi_get_dev_info,
	.tx_pkt                = narvi_midi_tx_pkt,
	.tx_sysex              = narvi_midi_tx_sysex,
	.builtin_cmd_port_mask = PHI_MIDI_PORT_ALL,
};

static THD_WORKING_AREA(midi_thread_wa, 1024 + sizeof(phi_bl_multiimg_hdr_t) + PHI_MIDI_SYSEX_MAX_LEN);
/*__attribute__((noreturn)) */static THD_FUNCTION(midi_thread, arg)
{
	(void)arg;

	chRegSetThreadName("midi");

	uint8_t r[4];
	while (1)
	{
		if (!chnReadTimeout(&MDU1, &r[0], 4, TIME_INFINITE)) {
			// USB not active, yuck
			chThdSleepMilliseconds(10);
			continue;
		}

		phi_midi_rx_pkt(PHI_MIDI_PORT_USB1, (const phi_midi_pkt_t *) r);
	}
}

void narvi_midi_init(void)
{
    phi_midi_init(&narvi_midi_cfg);

    mduObjectInit(&MDU1);
    mduStart(&MDU1, &midiusbcfg);

    chThdCreateStatic(midi_thread_wa, sizeof(midi_thread_wa), NORMALPRIO + 1, midi_thread, NULL);
}

void narvi_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	(void) port; (void) pkt;
}

void narvi_midi_in_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    (void) port;
    (void) cmd;
    (void) data;
    (void) data_len;

    switch (cmd)
    {
    /* Internal flash */
    case NARVI_BL_MIDI_SYSEX_CMD_BL_START:
        narvi_midi_bl_start(port, data, data_len);
        break;

    case NARVI_BL_MIDI_SYSEX_CMD_BL_DATA:
        narvi_midi_bl_data(port, data, data_len);
        break;

    case NARVI_BL_MIDI_SYSEX_CMD_BL_DONE:
        narvi_midi_bl_done(port, data, data_len);
        break;

	/* Serial flash */
	case NARVI_BL_MIDI_SYSEX_CMD_BL_SERFLASH_START:
		narvi_midi_bl_serflash_start(port, data, data_len);
		break;

	case NARVI_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DATA:
		narvi_midi_bl_serflash_data(port, data, data_len);
		break;

	case NARVI_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DONE:
		narvi_midi_bl_serflash_done(port, data, data_len);
		break;

	case NARVI_BL_MIDI_SYSEX_CMD_BL_UPDATE_SELF_FROM_SERFLASH:
		narvi_midi_bl_update_self_from_serflash(port, data, data_len);
		break;
    }
}

void narvi_midi_get_dev_info(phi_midi_sysex_dev_info_t * dev_info)
{
    dev_info->dev_id = NARVI_DEV_ID;
    dev_info->hw_sw_ver = NARVI_HW_SW_VER;
    memcpy(&(dev_info->uid[0]), &(dev_info->dev_id), sizeof(uint32_t));
    memcpy(&(dev_info->uid[4]), (void *) STM32_REG_UNIQUE_ID, 12);
}

void narvi_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	if  (port & PHI_MIDI_PORT_USB1)
	{
		phi_usb_midi_send3(&MDU1, 1, pkt->chn_event, pkt->val1, pkt->val2);
	}
}

void narvi_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len)
{
	if  (port & PHI_MIDI_PORT_USB1)
	{
        phi_usb_midi_send_sysex(&MDU1, 1, data, len);
    }
}

/******************************************************************************
 * Bootloader - internal flash
 ******************************************************************************/

#include "phi_lib/st/stm32f4xx_flash.h"


static phi_bl_state_t phi_midi_bl = {
    .img_size = 0,
    .crc32 = 0,
    .cur_offset = 0,
};


void narvi_midi_bl_start(phi_midi_port_t port, const void * data, size_t data_len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_start_t * msg = (phi_bl_msg_start_t *) data;

    if (sizeof(*msg) != data_len)
    {
        ret = PHI_BL_RET_SIZE_MISMATCH;
        goto lbl_ret;
    }

    if (msg->bl_hdr.write_addr != NARVI_BL_USER_ADDR)
    {
        ret = PHI_BL_RET_START_ADDR_MISMATCH;
        goto lbl_ret;
    }

    if (!phi_is_aligned(msg->bl_hdr.write_addr, NARVI_BL_FLASH_PAGE_SIZE))
    {
        ret = PHI_BL_RET_START_ADDR_MISALIGNED;
        goto lbl_ret;
    }

    if (!phi_is_aligned(msg->img_size, PHI_BL_DATA_PACKET_SIZE))
    {
        ret = PHI_BL_RET_IMG_SIZE_MISALIGNED;
        goto lbl_ret;
    }

    if (msg->bl_hdr.dev_id != NARVI_DEV_ID)
    {
        ret = PHI_BL_RET_DEV_ID_MISMATCH;
        goto lbl_ret;
    }

    memset(&phi_midi_bl, 0, sizeof(phi_midi_bl));
    phi_midi_bl.img_size = msg->img_size;
    phi_midi_bl.crc32 = msg->img_crc32;

    FLASH_Unlock();

    lcdReturnHome(&LCDD1);
    lcdWriteString(&LCDD1, "START       ", 40);

    ret = PHI_BL_RET_OK;

lbl_ret:
    phi_midi_tx_sysex(port, NARVI_BL_MIDI_SYSEX_CMD_BL_START, &ret, 1);
}

void narvi_midi_bl_data(phi_midi_port_t port, const void * data, size_t data_len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_data_t * msg = (phi_bl_msg_data_t *) data;
    uint32_t addr, i;

    if (sizeof(*msg) != data_len)
    {
        ret = PHI_BL_RET_SIZE_MISMATCH;
        goto lbl_ret;
    }

    if (0 == phi_midi_bl.img_size)
    {
        ret = PHI_BL_RET_NOT_READY;
        goto lbl_ret;
    }

    if (phi_midi_bl.cur_offset != msg->offset)
    {
        ret = PHI_BL_RET_OFFSET_MISMATCH;
        goto lbl_ret;
    }

    for (i = 0; i < sizeof(msg->buf); i += 4)
    {
        addr = NARVI_BL_USER_ADDR + phi_midi_bl.cur_offset + i;

        if ((phi_midi_bl.cur_offset + i) >= phi_midi_bl.img_size)
        {
            ret = PHI_BL_RET_OFFSET_OVERFLOW;
        }

        // If current offset is beginning of a page, erase page
        if (phi_is_aligned(addr, NARVI_BL_FLASH_PAGE_SIZE))
        {
        	/* This is ugly. EraseSector needs a sector number. Based on the stuff inside stm32f4xx_flash.h, we can learn:
			 * 1) FLASH_Sector_5 is the sector where NARVI_BL_USER_ADDR starts
			 * 2) The delta between each consecutive sectors is 8.
			 * The math below is a ghetto way of calculating the sector number we need to erase
			 */
			if (FLASH_COMPLETE != FLASH_EraseSector(FLASH_Sector_5 + (8 * (phi_midi_bl.cur_offset / NARVI_BL_FLASH_PAGE_SIZE)), VoltageRange_3))
			{
				ret = PHI_BL_RET_FLASH_ERASE_ERR;
				goto lbl_ret;
			}

        }

        if (FLASH_COMPLETE != FLASH_ProgramWord(addr, ((uint32_t *)msg->buf)[i / 4]))
        {
            ret = PHI_BL_RET_FLASH_WRITE_ERR;
            goto lbl_ret;
        }
    }

    phi_midi_bl.cur_offset += sizeof(msg->buf);
    ret = PHI_BL_RET_OK;

    char buf[20];
	chsnprintf(buf, sizeof(buf) - 1, "%d%%     ", phi_midi_bl.cur_offset * 100 / phi_midi_bl.img_size);
	lcdReturnHome(&LCDD1);
	lcdWriteString(&LCDD1, buf, 40);

lbl_ret:
    if (PHI_BL_RET_OK != ret)
    {
        // we're fucked
        memset(&phi_midi_bl, 0, sizeof(phi_midi_bl));
        FLASH_Lock();
    }

    phi_midi_tx_sysex(port, NARVI_BL_MIDI_SYSEX_CMD_BL_DATA, &ret, 1);
}

void narvi_midi_bl_done(phi_midi_port_t port, const void * data, size_t data_len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    char buf[20];

    (void) data;
    (void) data_len;

    if (0 == phi_midi_bl.img_size)
    {
        ret = PHI_BL_RET_NOT_READY;
        goto lbl_ret;
    }

    if (phi_midi_bl.cur_offset != phi_midi_bl.img_size)
    {
        ret = PHI_BL_RET_MISSING_DATA;
        goto lbl_ret;
    }

    if (phi_midi_bl.crc32 != phi_crc32((void *) NARVI_BL_USER_ADDR, phi_midi_bl.img_size))
    {
        ret = PHI_BL_RET_BAD_CRC;
        goto lbl_ret;
    }

    ret = PHI_BL_RET_OK;

lbl_ret:
    chsnprintf(buf, sizeof(buf) - 1, "DONE %d", ret);
    lcdReturnHome(&LCDD1);
	lcdWriteString(&LCDD1, buf, 40);

    memset(&phi_midi_bl, 0, sizeof(phi_midi_bl));
    FLASH_Lock();

    phi_midi_tx_sysex(port, NARVI_BL_MIDI_SYSEX_CMD_BL_DONE, &ret, 1);

    if (ret == PHI_BL_RET_OK)
    {
        chThdSleepMilliseconds(100);
        NVIC_SystemReset();
    }
}


/******************************************************************************
 * Bootloader - serial flash
 ******************************************************************************/

#include "phi_lib/phi_at45.h"

static phi_bl_state_t phi_midi_bl_serflash = {
    .img_size = 0,
    .crc32 = 0,
    .cur_offset = 0,
};

extern phi_at45_t at45;
uint8_t phi_midi_bl_serflash_buf[PHI_AT45_PAGE_SIZE];
uint32_t phi_midi_bl_serflash_buf_used = 0;
uint32_t phi_midi_bl_serflash_pages_written = 0;

void narvi_midi_bl_serflash_start(phi_midi_port_t port, const void * data, size_t data_len)
{
#if 0 // TODO Flash
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_multiimg_msg_start_t * msg = (phi_bl_multiimg_msg_start_t *) data;

    if (sizeof(*msg) != data_len)
    {
        ret = PHI_BL_RET_SIZE_MISMATCH;
        goto lbl_ret;
    }

    // We currently only support writing from beginning of flash
    if (!phi_is_aligned(msg->img_size, PHI_BL_DATA_PACKET_SIZE))
    {
        ret = PHI_BL_RET_IMG_SIZE_MISALIGNED;
        goto lbl_ret;
    }

    memset(&phi_midi_bl_serflash, 0, sizeof(phi_midi_bl_serflash));
    phi_midi_bl_serflash.img_size = msg->img_size;
    phi_midi_bl_serflash.crc32 = msg->img_crc32;
    phi_midi_bl_serflash_buf_used = 0;
    phi_midi_bl_serflash_pages_written = 0;

    lcdReturnHome(&LCDD1);
	lcdWriteString(&LCDD1, "START", 40);

    ret = PHI_BL_RET_OK;

lbl_ret:
    phi_midi_tx_sysex(port, NARVI_BL_MIDI_SYSEX_CMD_BL_SERFLASH_START, &ret, 1);
#endif
}

void narvi_midi_bl_serflash_data(phi_midi_port_t port, const void * data, size_t data_len)
{
#if 0 // TODO Flash
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_data_t * msg = (phi_bl_msg_data_t *) data;
    uint32_t chunk_size;
    phi_at45_ret_t at45_ret;
    char buf[20];

    if (sizeof(*msg) != data_len)
    {
        ret = PHI_BL_RET_SIZE_MISMATCH;
        goto lbl_ret;
    }

    if (0 == phi_midi_bl_serflash.img_size)
    {
        ret = PHI_BL_RET_NOT_READY;
        goto lbl_ret;
    }

    if (phi_midi_bl_serflash.cur_offset != msg->offset)
    {
        ret = PHI_BL_RET_OFFSET_MISMATCH;
        goto lbl_ret;
    }

    /* See how many bytes we can fit inside our receive buffer */
    chunk_size = phi_min(
    	sizeof(phi_midi_bl_serflash_buf) - phi_midi_bl_serflash_buf_used,
		sizeof(msg->buf)
	);
    memcpy(
    	phi_midi_bl_serflash_buf + phi_midi_bl_serflash_buf_used,
		msg->buf,
		chunk_size
	);

    phi_midi_bl_serflash_buf_used += chunk_size;

    /* Write page if we filled one up */
    if (phi_midi_bl_serflash_buf_used == sizeof(phi_midi_bl_serflash_buf))
    {
    	at45_ret = phi_at45_page_write(
    		&at45,
			phi_midi_bl_serflash_pages_written * PHI_AT45_PAGE_SIZE,
			phi_midi_bl_serflash_buf,
			phi_midi_bl_serflash_buf_used
		);

    	if (at45_ret != PHI_AT45_RET_SUCCESS)
    	{
    		ret = PHI_BL_RET_AT45_ERROR;
    		goto lbl_ret;
    	}

    	phi_midi_bl_serflash_buf_used = 0;
    	phi_midi_bl_serflash_pages_written++;

    	memcpy(
			phi_midi_bl_serflash_buf,
			msg->buf + chunk_size,
			sizeof(msg->buf) - chunk_size
		);
    }

    phi_midi_bl_serflash.cur_offset += sizeof(msg->buf);
    ret = PHI_BL_RET_OK;

	chsnprintf(buf, sizeof(buf) - 1, "%d%%    ", phi_midi_bl_serflash.cur_offset * 100 / phi_midi_bl_serflash.img_size);
	lcdReturnHome(&LCDD1);
	lcdWriteString(&LCDD1, buf, 40);

lbl_ret:
    if (PHI_BL_RET_OK != ret)
    {
        // we're fucked
        memset(&phi_midi_bl_serflash, 0, sizeof(phi_midi_bl_serflash));
    }

    phi_midi_tx_sysex(port, NARVI_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DATA, &ret, 1);
#endif
}

void narvi_midi_bl_serflash_done(phi_midi_port_t port, const void * data, size_t data_len)
{
#if 0 // TODO Flash
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    uint32_t crc = 0;
    uint32_t offset = 0;
    uint32_t chunk_size;
    phi_at45_ret_t at45_ret;
    char buf[20];

    (void) data;
    (void) data_len;

    if (0 == phi_midi_bl_serflash.img_size)
    {
        ret = PHI_BL_RET_NOT_READY;
        goto lbl_ret;
    }

    if (phi_midi_bl_serflash.cur_offset != phi_midi_bl_serflash.img_size)
    {
        ret = PHI_BL_RET_MISSING_DATA;
        goto lbl_ret;
    }

    /* Write whatever's left in the buffer */
    if (phi_midi_bl_serflash_buf_used > 0)
    {
    	at45_ret = phi_at45_page_write(
			&at45,
			phi_midi_bl_serflash_pages_written * PHI_AT45_PAGE_SIZE,
			phi_midi_bl_serflash_buf,
			phi_midi_bl_serflash_buf_used
		);

		if (at45_ret != PHI_AT45_RET_SUCCESS)
		{
			ret = PHI_BL_RET_AT45_ERROR;
			goto lbl_ret;
		}
    }

    /* Verify CRC */
    while (offset < phi_midi_bl_serflash.img_size)
    {
    	chunk_size = phi_min(
    		sizeof(phi_midi_bl_serflash_buf),
			phi_midi_bl_serflash.img_size - offset
    	);

    	phi_at45_read(&at45, offset, phi_midi_bl_serflash_buf, chunk_size);
    	crc = phi_crc32_incremental(crc, phi_midi_bl_serflash_buf, chunk_size);
    	offset += chunk_size;
    }

    if (phi_midi_bl_serflash.crc32 != crc)
    {
        ret = PHI_BL_RET_BAD_CRC;
        goto lbl_ret;
    }

    ret = PHI_BL_RET_OK;

lbl_ret:
    chsnprintf(buf, sizeof(buf) - 1, "DONE %d", ret);
    lcdReturnHome(&LCDD1);
	lcdWriteString(&LCDD1, buf, 40);

    memset(&phi_midi_bl_serflash, 0, sizeof(phi_midi_bl_serflash));

    phi_midi_tx_sysex(port, NARVI_BL_MIDI_SYSEX_CMD_BL_SERFLASH_DONE, &ret, 1);
#endif
}

void narvi_midi_bl_update_self_from_serflash(phi_midi_port_t port, const void * data, size_t data_len)
{
#if 0 // TODO Flash
	uint32_t flash_start_offset = 0;
	uint32_t offset = 0;
    uint32_t addr = 0;
    uint32_t total_len = 0;
    uint8_t buf[PHI_BL_DATA_PACKET_SIZE];
	phi_bl_hdr_t bl_hdr;
    phi_bl_ret_t ret;

    /* See if we can find an image for ourselves */
    ui = narvi_ui_lock(1);
	strcpy(ui->state.boot.misc_text[2], "FindBlHdr");
	narvi_ui_unlock(ui);

	ret = phi_bl_multiimg_find_bl_hdr(&at45, NARVI_DEV_ID, NARVI_HW_SW_VER, &flash_start_offset, &bl_hdr);
	if (ret != PHI_BL_RET_OK)
	{
		goto lbl_ret;
	}

	total_len = sizeof(bl_hdr) + bl_hdr.fw_data_size;

	/* Copy from serial flash to our flash */
	HAL_FLASH_Unlock();

	for (offset = 0; offset < total_len; offset += PHI_BL_DATA_PACKET_SIZE)
	{
	    ui = narvi_ui_lock(1);
		chsnprintf(ui->state.boot.misc_text[2], sizeof(ui->state.boot.misc_text[2]) - 1, "0: %d%%", offset * 100 / total_len);
		narvi_ui_unlock(ui);

		phi_at45_read(&at45, flash_start_offset + offset, buf, sizeof(buf));

		for (uint32_t i = 0; i < sizeof(buf); i += 4)
		{
			addr = NARVI_BL_USER_ADDR + offset + i;

			// If current offset is beginning of a page, erase page
			if (phi_is_aligned(addr, NARVI_BL_FLASH_PAGE_SIZE))
			{
				FLASH_EraseInitTypeDef erase;
				HAL_StatusTypeDef status;
				uint32_t err;

				erase.TypeErase = FLASH_TYPEERASE_PAGES;
				erase.PageAddress = addr;
				erase.NbPages = 1;

				status = HAL_FLASHEx_Erase(&erase, &err);
				if (status != HAL_OK)
				{
					err = PHI_BL_RET_FLASH_ERASE_ERR;
					goto lbl_ret;
				}
			}

			// Write
			if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, ((uint32_t *)buf)[i / 4]))
			{
				ui = narvi_ui_lock(1);
				chsnprintf(ui->state.boot.misc_text[2], sizeof(ui->state.boot.misc_text[2]) - 1, "0: err %d %d%%", offset * 100 / total_len, PHI_BL_RET_FLASH_WRITE_ERR);
				narvi_ui_unlock(ui);
				// TODO shitty error handling
				goto lbl_ret;
			}
		}
	}

	ret = PHI_BL_RET_OK;

lbl_ret:
	HAL_FLASH_Lock();

	ui = narvi_ui_lock(1);
	if (ret == PHI_BL_RET_OK)
	{
		strcpy(ui->state.boot.misc_text[2], "UPD SELF OK");
	}
	else
	{
		chsnprintf(ui->state.boot.misc_text[2], sizeof(ui->state.boot.misc_text[2]) - 1, "0: err %d %d%%", offset * 100 / total_len, ret);
	}
	narvi_ui_unlock(ui);

	phi_midi_tx_sysex(port, NARVI_BL_MIDI_SYSEX_CMD_BL_UPDATE_SELF_FROM_SERFLASH, &ret, 1);

    if (ret == PHI_BL_RET_OK)
    {
        chThdSleepMilliseconds(100);
        NVIC_SystemReset();
    }
#endif
}
