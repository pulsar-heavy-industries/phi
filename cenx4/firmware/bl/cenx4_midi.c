#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_midi.h"
#include "phi_lib/phi_bl_common.h"

#include "usbcfg.h"
#include "cenx4_midi.h"
#include "cenx4_ui.h"

const phi_midi_cfg_t cenx4_midi_cfg = {
    .in_handler   = cenx4_midi_in_handler,
    .in_sysex     = cenx4_midi_in_sysex,
    .get_dev_info = cenx4_midi_get_dev_info,
	.tx_pkt       = cenx4_midi_tx_pkt,
	.tx_sysex     = cenx4_midi_tx_sysex,
};

static THD_WORKING_AREA(midi_thread_wa, 512 + PHI_MIDI_SYSEX_MAX_LEN);
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

		phi_midi_rx_pkt(PHI_MIDI_PORT_USB, (const phi_midi_pkt_t *) r);
	}
}

void cenx4_midi_init(void)
{
    phi_midi_init(&cenx4_midi_cfg);

    mduObjectInit(&MDU1);
    mduStart(&MDU1, &midiusbcfg);

    chThdCreateStatic(midi_thread_wa, sizeof(midi_thread_wa), NORMALPRIO + 1, midi_thread, NULL);
}

void cenx4_midi_in_handler(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	(void) port; (void) pkt;
}

void cenx4_midi_in_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    (void) port;
    (void) cmd;
    (void) data;
    (void) data_len;

    switch (cmd)
    {
    case CENX4_BL_MIDI_SYSEX_CMD_BL_START:
        cenx4_midi_bl_start(port, data, data_len);
        break;

    case CENX4_BL_MIDI_SYSEX_CMD_BL_DATA:
        cenx4_midi_bl_data(port, data, data_len);
        break;

    case CENX4_BL_MIDI_SYSEX_CMD_BL_DONE:
        cenx4_midi_bl_done(port, data, data_len);
    }
}

void cenx4_midi_get_dev_info(phi_midi_sysex_dev_info_t * dev_info)
{
    dev_info->dev_id = CENX4_DEV_ID;
    dev_info->hw_sw_ver = CENX4_HW_SW_VER;
    memcpy(&(dev_info->uid[0]), &(dev_info->dev_id), sizeof(uint32_t));
    memcpy(&(dev_info->uid[4]), (void *) STM32_REG_UNIQUE_ID, 12);
}

void cenx4_midi_tx_pkt(phi_midi_port_t port, const phi_midi_pkt_t * pkt)
{
	if  (port & PHI_MIDI_PORT_USB)
	{
		phi_usb_midi_send3(&MDU1, 1, pkt->chn_event, pkt->val1, pkt->val2); // TODO 1? need to come from pkt
	}
}

void cenx4_midi_tx_sysex(phi_midi_port_t port, const uint8_t * data, size_t len)
{
	if  (port & PHI_MIDI_PORT_USB)
	{
        phi_usb_midi_send_sysex(&MDU1, 1, data, len);
    }
}

/******************************************************************************
 * Bootloader
 ******************************************************************************/

#include "phi_lib/st/stm32f3xx_hal_flash.h"
#include "phi_lib/st/stm32f3xx_hal_flash_ex.h"

static phi_bl_state_t phi_midi_bl = {
    .img_size = 0,
    .crc32 = 0,
    .cur_offset = 0,
};


void cenx4_midi_bl_start(phi_midi_port_t port, const void * data, size_t data_len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_start_t * msg = (phi_bl_msg_start_t *) data;
    cenx4_ui_t * ui;

    if (sizeof(*msg) != data_len)
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

    if (!phi_is_aligned(msg->img_size, PHI_BL_DATA_PACKET_SIZE))
    {
        ret = PHI_BL_RET_IMG_SIZE_MISALIGNED;
        goto lbl_ret;
    }

    if (msg->dev_id != CENX4_DEV_ID)
    {
        ret = PHI_BL_RET_DEV_ID_MISMATCH;
        goto lbl_ret;
    }

    memset(&phi_midi_bl, 0, sizeof(phi_midi_bl));
    phi_midi_bl.img_size = msg->img_size;
    phi_midi_bl.crc32 = msg->crc32;

    HAL_FLASH_Unlock();

    ui = cenx4_ui_lock(1);
	strcpy(ui->state.boot.misc_text[2], "START");
	cenx4_ui_unlock(ui);

    ret = PHI_BL_RET_OK;

lbl_ret:
    phi_midi_tx_sysex(port, CENX4_BL_MIDI_SYSEX_CMD_BL_START, &ret, 1);
}

void cenx4_midi_bl_data(phi_midi_port_t port, const void * data, size_t data_len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    const phi_bl_msg_data_t * msg = (phi_bl_msg_data_t *) data;
    uint32_t addr, i;
    cenx4_ui_t * ui;

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
        addr = CENX4_BL_USER_ADDR + phi_midi_bl.cur_offset + i;

        if ((phi_midi_bl.cur_offset + i) >= phi_midi_bl.img_size)
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

    phi_midi_bl.cur_offset += sizeof(msg->buf);
    ret = PHI_BL_RET_OK;

    ui = cenx4_ui_lock(1);
	chsnprintf(ui->state.boot.misc_text[2], sizeof(ui->state.boot.misc_text[2]) - 1, "%d%%", phi_midi_bl.cur_offset * 100 / phi_midi_bl.img_size);
	cenx4_ui_unlock(ui);

lbl_ret:
    if (PHI_BL_RET_OK != ret)
    {
        // we're fucked
        memset(&phi_midi_bl, 0, sizeof(phi_midi_bl));
        HAL_FLASH_Lock();
    }

    phi_midi_tx_sysex(port, CENX4_BL_MIDI_SYSEX_CMD_BL_DATA, &ret, 1);
}

void cenx4_midi_bl_done(phi_midi_port_t port, const void * data, size_t data_len)
{
    phi_bl_ret_t ret = PHI_BL_RET_ERR_UNKNOWN;
    cenx4_ui_t * ui;

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

    if (phi_midi_bl.crc32 != phi_crc32((void *) CENX4_BL_USER_ADDR, phi_midi_bl.img_size))
    {
        ret = PHI_BL_RET_BAD_CRC;
        goto lbl_ret;
    }

    ret = PHI_BL_RET_OK;

lbl_ret:
	ui = cenx4_ui_lock(1);
    chsnprintf(ui->state.boot.misc_text[2], sizeof(ui->state.boot.misc_text[2]) - 1, "DONE %d", ret);
    cenx4_ui_unlock(ui);

    memset(&phi_midi_bl, 0, sizeof(phi_midi_bl));
    HAL_FLASH_Lock();

    phi_midi_tx_sysex(port, CENX4_BL_MIDI_SYSEX_CMD_BL_DONE, &ret, 1);

    if (ret == PHI_BL_RET_OK)
    {
        chThdSleepMilliseconds(100);
        NVIC_SystemReset();
    }
}
