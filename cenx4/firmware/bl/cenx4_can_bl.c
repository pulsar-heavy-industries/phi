#include "phi_lib/phi_lib.h"
#include "phi_lib/st/stm32f3xx_hal_flash.h"
#include "phi_lib/st/stm32f3xx_hal_flash_ex.h"
#include "cenx4_can_bl.h"
#include "cenx4_ui.h"


static phi_bl_state_t phi_can_bl = {
    .img_size = 0,
    .crc32 = 0,
    .cur_offset = 0,
};

void cenx4_can_bl_handle_start(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_start_t * msg = (phi_bl_msg_start_t *) data;
    cenx4_ui_t * ui;

    (void) context;

    if (sizeof(*msg) != len)
    {
        ret = PHI_BL_RET_SIZE_MISMATCH;
        goto lbl_ret;
    }

    if (msg->img_start_addr != CENX4_BL_USER_ADDR)
    {
        ret = PHI_BL_RET_START_ADDR_MISMATCH;
        goto lbl_ret;
    }

    if (!phi_is_aligned(msg->img_start_addr, CENX4_BL_FLASH_PAGE_SIZE))
    {
        ret = PHI_BL_RET_START_ADDR_MISALIGNED;
        goto lbl_ret;
    }

    if (msg->dev_id != can->cfg->dev_id)
    {
        ret = PHI_BL_RET_DEV_ID_MISMATCH;
        goto lbl_ret;
    }

    memset(&phi_can_bl, 0, sizeof(phi_can_bl));
    phi_can_bl.img_size = msg->img_size;
    phi_can_bl.crc32 = msg->crc32;

    HAL_FLASH_Unlock();

    ui = cenx4_ui_lock(1);
    strcpy(ui->state.boot.misc_text[2], "START");
    cenx4_ui_unlock(ui);

    ret = PHI_BL_RET_OK;

lbl_ret:
  phi_can_send_reply(can, prio, msg_id, src, chan_id, &ret, 1, PHI_CAN_DEFAULT_TIMEOUT);
}


void cenx4_can_bl_handle_data(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_data_t * msg = (phi_bl_msg_data_t *) data;
    uint32_t addr, i;
    cenx4_ui_t * ui;

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
        addr = CENX4_BL_USER_ADDR + phi_can_bl.cur_offset + i;

        if ((phi_can_bl.cur_offset + i) >= phi_can_bl.img_size)
        {
            ret = PHI_BL_RET_OFFSET_OVERFLOW;
        }

        // If current offset is beginning of a page, erase page
        if (phi_is_aligned(addr, CENX4_BL_FLASH_PAGE_SIZE))
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

				ret = PHI_BL_RET_FLASH_ERASE_ERR;
				goto lbl_ret;
			}
        }

        if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, ((uint32_t *)msg->buf)[i / 4]))
        {
            ret = PHI_BL_RET_FLASH_WRITE_ERR;
            goto lbl_ret;
        }
    }

    phi_can_bl.cur_offset += sizeof(msg->buf);
    ret = PHI_BL_RET_OK;

    ui = cenx4_ui_lock(1);
    chsnprintf(ui->state.boot.misc_text[2], sizeof(ui->state.boot.misc_text[2]) - 1, "%d%%", phi_can_bl.cur_offset * 100 / phi_can_bl.img_size);
    cenx4_ui_unlock(ui);

lbl_ret:
    if (PHI_BL_RET_OK != ret)
    {
        // we're fucked
        memset(&phi_can_bl, 0, sizeof(phi_can_bl));
        HAL_FLASH_Lock();
    }

  phi_can_send_reply(can, prio, msg_id, src, chan_id, &ret, 1, PHI_CAN_DEFAULT_TIMEOUT);
}

void cenx4_can_bl_handle_done(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    cenx4_ui_t * ui;

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

    if (phi_can_bl.crc32 != phi_crc32((void *) CENX4_BL_USER_ADDR, phi_can_bl.img_size))
    {
        ret = PHI_BL_RET_BAD_CRC;
        goto lbl_ret;
    }

    ret = PHI_BL_RET_OK;

lbl_ret:
    ui = cenx4_ui_lock(1);
    chsnprintf(ui->state.boot.misc_text[2], sizeof(ui->state.boot.misc_text[2]) - 1, "DONE %d", ret);
    cenx4_ui_unlock(ui);

    memset(&phi_can_bl, 0, sizeof(phi_can_bl));
    HAL_FLASH_Lock();

    phi_can_send_reply(can, prio, msg_id, src, chan_id, &ret, 1, PHI_CAN_DEFAULT_TIMEOUT);

    if (ret == PHI_BL_RET_OK)
    {
        chThdSleepMilliseconds(100);
        NVIC_SystemReset();
    }
}
