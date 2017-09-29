#include "phi_lib/phi_bl_common.h"
#include "cenx4_app_cfg.h"
#include "cenx4_app_registry.h"
#include "cenx4_app_init.h"
#include "cenx4_can.h"

extern phi_at45_t at45;

const phi_app_desc_t cenx4_app_init_desc = {
    .start = cenx4_app_init_start,
    .stop = cenx4_app_init_stop,
};

void cenx4_app_init_start(void * _ctx)
{
    cenx4_app_init_context_t * ctx = (cenx4_app_init_context_t *) _ctx;

    // Basic init
    chDbgCheck(ctx != NULL);
    chDbgCheck(cenx4_can.node_id == PHI_CAN_AUTO_ID_ALLOCATOR_NODE); // init app can only run on allocator
    memset(ctx, 0, sizeof(*ctx));

    // Some logging
    cenx4_app_log("Init!");
    cenx4_app_log_fmt("Hw@%d", PHI_HW_SW_VER_GET_HW(CENX4_HW_SW_VER));
    cenx4_app_log_fmt("Sw@%d", PHI_HW_SW_VER_GET_SW(CENX4_HW_SW_VER));
    cenx4_app_log_fmt("@%x", CENX4_DEV_ID);

    // Enter init mode locally
	cenx4_app_init_enter_init_mode(ctx);

	// Update slaves
	// TODO error check?
	cenx4_app_init_update_slaves(ctx);

	// Lets get this party started
	switch (cenx4_app_cfg.cur.startup_app)
	{
	case CENX4_APP_CFG_STARTUP_APP_ABLETON:
		phi_app_mgr_switch_app(&cenx4_app_ableton_desc, &(cenx4_app_contexts.ableton));
		break;

	case CENX4_APP_CFG_STARTUP_APP_ABLETON2:
		phi_app_mgr_switch_app(&cenx4_app_ableton2_desc, &(cenx4_app_contexts.ableton2));
		break;

	case CENX4_APP_CFG_STARTUP_APP_TRAKTOR:
		phi_app_mgr_switch_app(&cenx4_app_traktor_desc, &(cenx4_app_contexts.traktor));
		break;

	default:
		phi_app_mgr_switch_app(&cenx4_app_setup_desc, &(cenx4_app_contexts.setup));
		break;
	}
}

void cenx4_app_init_stop(void * ctx)
{
    (void) ctx;
}


void cenx4_app_init_enter_init_mode(cenx4_app_init_context_t * ctx)
{
	cenx4_ui_t * ui;

	// Move our displays into init mode - display 0
	ui = cenx4_ui_lock(0);

	ui->dispmode = CENX4_UI_DISPMODE_TEXTS;

	memset(&(ui->state), 0, sizeof(ui->state));

	ui->state.text.flags[0] = CENX4_UI_DISPMODE_TEXT_FLAGS_ALIGN_CENTER;
	ui->state.text.flags[3] = CENX4_UI_DISPMODE_TEXT_FLAGS_ALIGN_CENTER | CENX4_UI_DISPMODE_TEXT_FLAGS_BOTTOM | CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_MED;
	strcpy(ui->state.text.lines[0], "Init");
	strcpy(ui->state.text.lines[1], "MASTER");

	cenx4_ui_unlock(ui);

	// Move our displays into init mode - display 1
	ui = cenx4_ui_lock(1);
	ui->dispmode = CENX4_UI_DISPMODE_LOGO;
	cenx4_ui_unlock(ui);
}

void cenx4_app_init_update_slaves(cenx4_app_init_context_t * ctx)
{
	int i;
	for (i = 0; i < cenx4_can.auto_alloc_num_devs; ++i)
	{
		if (!cenx4_app_init_bootload_slave(ctx, PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + i))
		{
			break;
		}
	}
}

bool cenx4_app_init_bootload_slave(cenx4_app_init_context_t * ctx, uint8_t node_id)
{
	char err[CENX4_UI_MAX_LINE_TEXT_LEN];
	phi_bl_msg_start_t msg_start;
	phi_bl_ret_t bl_ret;
	msg_t ret;
	uint32_t resp_len;
	uint32_t offset;
	uint32_t total_len;
	uint32_t flash_start_offset;
	phi_bl_msg_data_t msg_data;
	phi_can_msg_data_sysinfo_t sysinfo;
	uint32_t crc;
	cenx4_ui_t * ui;
	phi_bl_hdr_t bl_hdr;

	// Starting
	ui=cenx4_ui_lock(0);

	memset(&(ui->state), 0, sizeof(ui->state));
	ui->dispmode = CENX4_UI_DISPMODE_TEXTS;
	ui->state.text.flags[0] = CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;
	ui->state.text.flags[1] = CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;
	ui->state.text.flags[2] = CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;
	ui->state.text.flags[3] = CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL;

	chsnprintf(ui->state.text.lines[0], CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Update #%d", node_id);
	strcpy(ui->state.text.lines[1], "Starting");
	cenx4_ui_unlock(ui);

	// Get dev info to see if we support updating it
	ret = phi_can_xfer(
		&cenx4_can,
		PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_ID_SYSINFO,
		node_id,
		NULL,
		0,
		(uint8_t *) &sysinfo,
		sizeof(sysinfo),
		&resp_len,
		PHI_CAN_DEFAULT_TIMEOUT
	);
	if (MSG_OK != ret)
	{
		chsnprintf(err, sizeof(err) - 1, "SIErr1 %d", ret);
		goto lbl_err;
	}
	if (sizeof(sysinfo) != resp_len)
	{
		chsnprintf(err, sizeof(err) - 1, "SIErr2 %d", resp_len);
		goto lbl_err;
	}

	// See if we know how to update this thing
	ret = phi_bl_multiimg_find_bl_hdr(&at45, sysinfo.dev_id, sysinfo.hw_sw_ver, &flash_start_offset, &bl_hdr);
	if (ret == PHI_BL_RET_NOT_FOUND)
	{
		ui = cenx4_ui_lock(0);
		strcpy(ui->state.text.lines[1], "NoImg");
		cenx4_ui_unlock(ui);

		return TRUE;
	}
	else if (ret != PHI_BL_RET_OK)
	{
		chsnprintf(err, sizeof(err) - 1, "FindBlHdr%d", resp_len);
		goto lbl_err;
	}

	// See if there's anything to update
	if (bl_hdr.sw_ver == PHI_HW_SW_VER_GET_SW(sysinfo.hw_sw_ver))
	{
		ui = cenx4_ui_lock(0);
		strcpy(ui->state.text.lines[1], "Uptodate");
		cenx4_ui_unlock(ui);

		return TRUE;
	}

	// Get crc32 of the image we're about to send
	total_len = sizeof(bl_hdr) + bl_hdr.fw_data_size;

	ui=cenx4_ui_lock(0);
	strcpy(ui->state.text.lines[1], "CRCCalc");
	cenx4_ui_unlock(ui);

	crc = 0;
	for (offset = 0; offset < total_len; offset += PHI_BL_DATA_PACKET_SIZE)
	{
		phi_at45_read(&at45, flash_start_offset + offset, msg_data.buf, sizeof(msg_data.buf));
		crc = phi_crc32_incremental(crc, msg_data.buf, sizeof(msg_data.buf));
	}

	// start bootloader if needed
	if (!PHI_HW_SW_VER_GET_IS_BOOTLOADER(sysinfo.hw_sw_ver))
	{
		ui=cenx4_ui_lock(0);
		strcpy(ui->state.text.lines[1], "BLPreStart");
		cenx4_ui_unlock(ui);

		phi_can_xfer(
			&cenx4_can,
			PHI_CAN_PRIO_LOWEST + 1,
			PHI_CAN_MSG_ID_START_BOOTLOADER,
			node_id,
			NULL,
			0,
			NULL,
			0,
			NULL,
			PHI_CAN_DEFAULT_TIMEOUT
		);
		chThdSleepMilliseconds(500);

		// Try getting device info again
		// Get dev info to see if we support updating it
		ret = phi_can_xfer(
			&cenx4_can,
			PHI_CAN_PRIO_LOWEST + 1,
			PHI_CAN_MSG_ID_SYSINFO,
			node_id,
			NULL,
			0,
			(uint8_t *) &sysinfo,
			sizeof(sysinfo),
			&resp_len,
			PHI_CAN_DEFAULT_TIMEOUT
		);
		if (MSG_OK != ret)
		{
			chsnprintf(err, sizeof(err) - 1, "BLSErr1 %d", ret);
			goto lbl_err;
		}
		if (sizeof(sysinfo) != resp_len)
		{
			chsnprintf(err, sizeof(err) - 1, "BLSErr2 %d", resp_len);
			goto lbl_err;
		}
		if (!PHI_HW_SW_VER_GET_IS_BOOTLOADER(sysinfo.hw_sw_ver))
		{
			chsnprintf(err, sizeof(err) - 1, "B%x", sysinfo.hw_sw_ver);
			goto lbl_err;
		}
	}

	// Bootloader start command
	ui=cenx4_ui_lock(0);
	strcpy(ui->state.text.lines[1], "BLStart");
	cenx4_ui_unlock(ui);

	memset(&msg_start, 0, sizeof(msg_start));
	msg_start.img_size = total_len;
	msg_start.img_crc32 = crc;
	memcpy(&(msg_start.bl_hdr), &bl_hdr, sizeof(bl_hdr));

	bl_ret = PHI_BL_RET_ERR_UNKNOWN;
	ret = phi_can_xfer(
		&cenx4_can,
		PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_BL_START,
		node_id,
		(const uint8_t *) &msg_start,
		sizeof(msg_start),
		&bl_ret,
		sizeof(bl_ret),
		&resp_len,
		PHI_CAN_DEFAULT_TIMEOUT
	);
	if (MSG_OK != ret)
	{
		chsnprintf(err, sizeof(err) - 1, "StartErr1 %d", ret);
		goto lbl_err;
	}
	if (sizeof(bl_ret) != resp_len)
	{
		chsnprintf(err, sizeof(err) - 1, "StartErr2 %d", resp_len);
		goto lbl_err;
	}
	if (PHI_BL_RET_OK != bl_ret)
	{
		chsnprintf(err, sizeof(err) - 1, "StartErr3 %d", bl_ret);
		goto lbl_err;
	}

	for (offset = 0; offset < total_len; offset += sizeof(msg_data.buf))
	{
		memset(&msg_data, 0, sizeof(msg_data));
		msg_data.offset = offset;

		phi_at45_read(&at45, flash_start_offset + offset, msg_data.buf, sizeof(msg_data.buf));

		ui=cenx4_ui_lock(0);
		chsnprintf(ui->state.text.lines[1], CENX4_UI_MAX_LINE_TEXT_LEN - 1, "%d", offset);
		chsnprintf(ui->state.text.lines[2], CENX4_UI_MAX_LINE_TEXT_LEN - 1, "%d%%", offset * 100 / total_len);
		cenx4_ui_unlock(ui);

		bl_ret = PHI_BL_RET_ERR_UNKNOWN;
		ret = phi_can_xfer(
			&cenx4_can,
			PHI_CAN_PRIO_LOWEST + 1,
			PHI_CAN_MSG_BL_DATA,
			node_id,
			(const uint8_t *) &msg_data,
			sizeof(msg_data),
			&bl_ret,
			sizeof(bl_ret),
			&resp_len,
			MS2ST(3000)
		);
		if (MSG_OK != ret)
		{
			chsnprintf(err, sizeof(err) - 1, "DataErr1 %d", ret);
			goto lbl_err;
		}
		if (sizeof(bl_ret) != resp_len)
		{
			chsnprintf(err, sizeof(err) - 1, "DataErr2 %d", resp_len);
			goto lbl_err;
		}
		if (PHI_BL_RET_OK != bl_ret)
		{
			chsnprintf(err, sizeof(err) - 1, "DataErr3 %d", bl_ret);
			goto lbl_err;
		}
	}

	ui=cenx4_ui_lock(0);
	strcpy(ui->state.text.lines[1], "SendDone");
	strcpy(ui->state.text.lines[2], "");
	cenx4_ui_unlock(ui);


	bl_ret = PHI_BL_RET_ERR_UNKNOWN;
	ret = phi_can_xfer(
		&cenx4_can,
		PHI_CAN_PRIO_LOWEST + 1,
		PHI_CAN_MSG_BL_DONE,
		node_id,
		NULL,
		0,
		&bl_ret,
		sizeof(bl_ret),
		&resp_len,
		PHI_CAN_DEFAULT_TIMEOUT
	);
	if (MSG_OK != ret)
	{
		chsnprintf(err, sizeof(err) - 1, "DoneErr1 %d", ret);
		goto lbl_err;
	}
	if (sizeof(bl_ret) != resp_len)
	{
		chsnprintf(err, sizeof(err) - 1, "DoneErr2 %d", resp_len);
		goto lbl_err;
	}
	if (PHI_BL_RET_OK != bl_ret)
	{
		chsnprintf(err, sizeof(err) - 1, "DoneErr3 %d", bl_ret);
		goto lbl_err;
	}

	/* Wait for reboot */
	ui=cenx4_ui_lock(0);
	strcpy(ui->state.text.lines[1], "Wait");
	cenx4_ui_unlock(ui);

	chThdSleepMilliseconds(500);

	/* Done */

	return TRUE;

lbl_err:
	ui=cenx4_ui_lock(0);
	strcpy(ui->state.text.lines[1], err);
	cenx4_ui_unlock(ui);

	return FALSE;
}
