#include "phi_lib/phi_lib.h"
#include "phi_lib/st/stm32f4xx_flash.h"
#include "hyperion_can_bl.h"
#include "hyperion_ui.h"


static phi_bl_state_t phi_can_bl = {
    .img_size = 0,
    .crc32 = 0,
    .cur_offset = 0,
};

void hyperion_can_bl_handle_start(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_start_t * msg = (phi_bl_msg_start_t *) data;
    hyperion_ui_t * ui;

    (void) context;

    if (sizeof(*msg) != len)
    {
        ret = PHI_BL_RET_SIZE_MISMATCH;
        goto lbl_ret;
    }

    if (msg->bl_hdr.write_addr != HYPERION_BL_USER_ADDR)
    {
        ret = PHI_BL_RET_START_ADDR_MISMATCH;
        goto lbl_ret;
    }

    if (!phi_is_aligned(msg->bl_hdr.write_addr, HYPERION_BL_FLASH_PAGE_SIZE))
    {
        ret = PHI_BL_RET_START_ADDR_MISALIGNED;
        goto lbl_ret;
    }

    if (msg->bl_hdr.dev_id != can->cfg->dev_id)
    {
        ret = PHI_BL_RET_DEV_ID_MISMATCH;
        goto lbl_ret;
    }

    memset(&phi_can_bl, 0, sizeof(phi_can_bl));
    phi_can_bl.img_size = msg->img_size;
    phi_can_bl.crc32 = msg->img_crc32;

    FLASH_Unlock();

    ui = hyperion_ui_lock();
    strcpy(ui->state.boot.misc_text, "START");
    hyperion_ui_unlock(ui);

    ret = PHI_BL_RET_OK;

lbl_ret:
  phi_can_send_reply(can, prio, msg_id, src, chan_id, &ret, 1, PHI_CAN_DEFAULT_TIMEOUT);
}


void hyperion_can_bl_handle_data(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_data_t * msg = (phi_bl_msg_data_t *) data;
    uint32_t addr, i;
    hyperion_ui_t * ui;

    (void) context;

    if (sizeof(*msg) != len)
    {
        ret = PHI_BL_RET_SIZE_MISMATCH;
        goto lbl_ret;
    }

    if (0 == phi_can_bl.img_size)
    {
        ret = PHI_BL_RET_NOT_READY;
        goto lbl_ret;
    }

    if (phi_can_bl.cur_offset != msg->offset)
    {
        ret = PHI_BL_RET_OFFSET_MISMATCH;
        goto lbl_ret;
    }

    for (i = 0; i < sizeof(msg->buf); i += 4)
    {
        addr = HYPERION_BL_USER_ADDR + phi_can_bl.cur_offset + i;

        if ((phi_can_bl.cur_offset + i) >= phi_can_bl.img_size)
        {
            ret = PHI_BL_RET_OFFSET_OVERFLOW;
        }

        // If current offset is beginning of a page, erase page
        if (phi_is_aligned(addr, HYPERION_BL_FLASH_PAGE_SIZE))
        {
        	/* This is ugly. EraseSector needs a sector number. Based on the stuff inside stm32f4xx_flash.h, we can learn:
			 * 1) FLASH_Sector_5 is the sector where HYPERION_BL_USER_ADDR starts
			 * 2) The delta between each consecutive sectors is 8.
			 * The math below is a ghetto way of calculating the sector number we need to erase
			 */
			if (FLASH_COMPLETE != FLASH_EraseSector(FLASH_Sector_5 + (8 * (phi_can_bl.cur_offset / HYPERION_BL_FLASH_PAGE_SIZE)), VoltageRange_3))
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

    phi_can_bl.cur_offset += sizeof(msg->buf);
    ret = PHI_BL_RET_OK;

    ui = hyperion_ui_lock();
    chsnprintf(ui->state.boot.misc_text, sizeof(ui->state.boot.misc_text) - 1, "%d%%", phi_can_bl.cur_offset * 100 / phi_can_bl.img_size);
    hyperion_ui_unlock(ui);

lbl_ret:
    if (PHI_BL_RET_OK != ret)
    {
        // we're fucked
        memset(&phi_can_bl, 0, sizeof(phi_can_bl));
        FLASH_Lock();
    }

  phi_can_send_reply(can, prio, msg_id, src, chan_id, &ret, 1, PHI_CAN_DEFAULT_TIMEOUT);
}

void hyperion_can_bl_handle_done(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    hyperion_ui_t * ui;

    (void) context;
    (void) data;
    (void) len;

    if (0 == phi_can_bl.img_size)
    {
        ret = PHI_BL_RET_NOT_READY;
        goto lbl_ret;
    }

    if (phi_can_bl.cur_offset != phi_can_bl.img_size)
    {
        ret = PHI_BL_RET_MISSING_DATA;
        goto lbl_ret;
    }

    if (phi_can_bl.crc32 != phi_crc32((void *) HYPERION_BL_USER_ADDR, phi_can_bl.img_size))
    {
        ret = PHI_BL_RET_BAD_CRC;
        goto lbl_ret;
    }

    ret = PHI_BL_RET_OK;

lbl_ret:
    ui = hyperion_ui_lock();
    chsnprintf(ui->state.boot.misc_text, sizeof(ui->state.boot.misc_text) - 1, "DONE %d", ret);
    hyperion_ui_unlock(ui);

    memset(&phi_can_bl, 0, sizeof(phi_can_bl));
    FLASH_Lock();

    phi_can_send_reply(can, prio, msg_id, src, chan_id, &ret, 1, PHI_CAN_DEFAULT_TIMEOUT);

    if (ret == PHI_BL_RET_OK)
    {
        chThdSleepMilliseconds(100);
        NVIC_SystemReset();
    }
}
