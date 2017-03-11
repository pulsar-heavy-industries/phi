#include "phi_lib/phi_lib.h"
#include "cenx4_can.h"
#include "cenx4_ui.h"

#if GDISP_TOTAL_DISPLAYS != CENX4_UI_NUM_DISPS
#error Nope
#endif

static cenx4_ui_t uis[CENX4_UI_NUM_DISPS];

static font_t font;
static coord_t font_height;
#define cenx4_ui_text(ui, x, y, w, justify, text) gdispGFillStringBox(ui->g, x, y, w, font_height, text, font, White, Black, justify)

void (*cenx4_ui_renderers[])(cenx4_ui_t *) = {
	cenx4_ui_render_logo,
	cenx4_ui_render_boot,
};

static THD_WORKING_AREA(cenx4_ui_thread_wa, 1024);
static THD_FUNCTION(cenx4_ui_thread, arg)
{
	int i;
	cenx4_ui_t * ui;
	cenx4_ui_dispmode_t last_mode[CENX4_UI_NUM_DISPS] = {0, };

	(void) arg;

	chRegSetThreadName("ui");

	while (!chThdShouldTerminateX())
	{
		for (i = 0; i < CENX4_UI_NUM_DISPS; ++i)
		{
			ui = &(uis[i]);

			chMtxLock(&(ui->lock));
			if (ui->dispmode != last_mode[i])
			{
				last_mode[i] = ui->dispmode;
				gdispGClear(ui->g, Black);
			}
			cenx4_ui_renderers[ui->dispmode](ui);
			chMtxUnlock(&(ui->lock));

			gdispGFlush(ui->g);
		}

		chThdSleepMilliseconds(50);
	}
}

void cenx4_ui_init_obj(cenx4_ui_t * ui, GDisplay * g)
{
	chDbgCheck((ui != NULL) && (g != NULL));

	memset(ui, 0, sizeof(*ui));
	ui->g = g;
	ui->w = gdispGGetWidth(ui->g);
	ui->h = gdispGGetHeight(ui->g);

	chMtxObjectInit(&(ui->lock));
}

void cenx4_ui_init(void)
{
	int i;

	for (i = 0; i < CENX4_UI_NUM_DISPS; ++i)
	{
		cenx4_ui_init_obj(&(uis[i]), gdispGetDisplay(i));
	}

	font = gdispOpenFont("DejaVuSans10");
	font_height = gdispGetFontMetric(font, fontHeight);

	uis[0].dispmode = CENX4_UI_DISPMODE_LOGO;
	uis[1].dispmode = CENX4_UI_DISPMODE_BOOT;

	chThdCreateStatic(cenx4_ui_thread_wa, sizeof(cenx4_ui_thread_wa), NORMALPRIO-10, cenx4_ui_thread, NULL);
}

cenx4_ui_t * cenx4_ui_lock(uint8_t idx)
{
	cenx4_ui_t * ui = &(uis[idx]);
	chDbgCheck(idx < CENX4_UI_NUM_DISPS);
	//chMtxLock(&(ui->lock));
	return ui;
}
void cenx4_ui_unlock(cenx4_ui_t * ui)
{
	chDbgCheck(ui);
	//chMtxUnlock(&(ui->lock));
}

const uint8_t phi_logo[][8] = {
    {0, 0, 31, 255, 255, 248, 0, 0},
    {0, 0, 31, 255, 255, 248, 0, 0},
    {0, 0, 31, 255, 255, 248, 0, 0},
    {0, 0, 31, 255, 255, 248, 0, 0},
    {0, 0, 0, 31, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 63, 248, 0, 0, 0},
    {0, 0, 3, 255, 255, 192, 0, 0},
    {0, 0, 31, 255, 255, 248, 0, 0},
    {0, 0, 127, 255, 255, 254, 0, 0},
    {0, 0, 255, 143, 241, 255, 0, 0},
    {0, 3, 252, 15, 240, 127, 192, 0},
    {0, 7, 248, 15, 240, 31, 224, 0},
    {0, 15, 240, 15, 240, 31, 240, 0},
    {0, 31, 240, 15, 240, 15, 248, 0},
    {0, 31, 224, 15, 240, 15, 248, 0},
    {0, 63, 224, 15, 240, 7, 248, 0},
    {0, 63, 224, 15, 240, 7, 252, 0},
    {0, 127, 224, 15, 240, 7, 252, 0},
    {0, 63, 192, 15, 240, 7, 252, 0},
    {0, 63, 224, 15, 240, 7, 252, 0},
    {0, 63, 224, 15, 240, 7, 252, 0},
    {0, 63, 224, 15, 240, 7, 252, 0},
    {0, 63, 224, 15, 240, 7, 248, 0},
    {0, 31, 224, 15, 240, 15, 248, 0},
    {0, 31, 240, 15, 240, 15, 240, 0},
    {0, 7, 248, 15, 240, 31, 224, 0},
    {0, 3, 252, 15, 240, 63, 192, 0},
    {0, 1, 254, 15, 240, 255, 128, 0},
    {0, 0, 255, 207, 247, 254, 0, 0},
    {0, 0, 63, 255, 255, 248, 0, 0},
    {0, 0, 7, 255, 255, 224, 0, 0},
    {0, 0, 0, 255, 254, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 0, 15, 240, 0, 0, 0},
    {0, 0, 5, 127, 254, 96, 0, 0},
	{0, 0, 31, 255, 255, 248, 0, 0},
	{0, 0, 31, 255, 255, 248, 0, 0},
	{0, 0, 31, 255, 255, 248, 0, 0},
	{0, 0, 31, 255, 255, 248, 0, 0},
};
void cenx4_ui_render_logo(cenx4_ui_t * ui)
{
	for (int disp = 0; disp < 2; ++disp)
    {
	   for (int row = 0; row < PHI_ARRLEN(phi_logo); ++row)
	   {
		   for (int byte = 0; byte < 8; ++byte)
		   {
			   for (int bit = 0; bit < 8; ++bit)
			   {
				   if (phi_logo[row][byte] & (1 << (7-bit)))
				   {
					   gdispGDrawPixel(ui->g, (byte * 8) + bit, 32 + row, White);
				   }
			   }
		   }
	   }
	   gdispGFlush(ui->g);
   }
}

void cenx4_ui_render_boot(cenx4_ui_t * ui)
{
	coord_t         y = 0;
	char            buf[CENX4_UI_MAX_LINE_TEXT_LEN];
	systime_t       ping = chVTGetSystemTimeX();
	uint32_t        up_secs = ST2S(ping);
	msg_t           ret;

	cenx4_ui_text(ui, 0, 0, ui->w, justifyCenter, "cenx4-bl");
	y += font_height + 2;

	chsnprintf(buf, sizeof(buf) - 1, "HwSw %04x", CENX4_HW_SW_VER);
	cenx4_ui_text(ui, 0, y, ui->w, justifyLeft, buf);
	y += font_height + 2;

	chsnprintf(buf, sizeof(buf) - 1, "NodeId %d", cenx4_can.node_id);
	cenx4_ui_text(ui, 0, y, ui->w, justifyLeft, buf);
	y += font_height + 2;

	if (cenx4_can.node_id != PHI_CAN_AUTO_ID)
	{
		ret = phi_can_xfer(
			&cenx4_can,
			PHI_CAN_PRIO_LOWEST,
			PHI_CAN_MSG_ID_ECHO,
			1,
			(const uint8_t *) &ping,
			sizeof(ping),
			(uint8_t *) &ping,
			sizeof(ping),
			NULL,
			PHI_CAN_DEFAULT_TIMEOUT);
		switch (ret)
		{
		case MSG_RESET:
			strcpy(buf, "P: RST");
			break;

		case MSG_TIMEOUT:
			strcpy(buf, "P: Timeout");

			break;

		case MSG_OK:
			chsnprintf(buf, sizeof(buf) - 1, "P: ~%2dms", ST2MS(chVTGetSystemTimeX() - ping));
			break;

		default:
			chDbgAssert(FALSE, "WTF");
			break;
		}
	}
	else
	{
		strcpy(buf, "P: NoNodeId");
	}
	cenx4_ui_text(ui, 0, y, ui->w, justifyLeft, buf);
	y += font_height + 2;

	chsnprintf(buf, sizeof(buf) - 1, "CanOk %d", cenx4_can.stat_process_rx);
	cenx4_ui_text(ui, 0, y, ui->w, justifyLeft, buf);
	y += font_height + 2;

	cenx4_ui_text(ui, 0, y, ui->w, justifyLeft, ui->state.boot.misc_text[0]);
	y += font_height + 2;

	cenx4_ui_text(ui, 0, y, ui->w, justifyLeft, ui->state.boot.misc_text[1]);
    y += font_height + 2;

    cenx4_ui_text(ui, 0, y, ui->w, justifyLeft, ui->state.boot.misc_text[2]);
    y += font_height + 2;

    chsnprintf(buf, sizeof(buf) - 1, "UP %02d:%02d", up_secs / 60, up_secs % 60);
	cenx4_ui_text(ui, 0, gdispGGetHeight(ui->g) - font_height - 3, ui->w, justifyLeft, buf);
}
