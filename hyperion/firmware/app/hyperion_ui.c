#include "hyperion_can.h"
#include "hyperion_ui.h"


static hyperion_ui_t ui;

static font_t fonts[3];
static coord_t font_heights[3];

void (*hyperion_ui_renderers[])(hyperion_ui_t *) = {
	hyperion_ui_render_boot,
	hyperion_ui_render_texts,
	hyperion_ui_render_logo,
	hyperion_ui_render_callback,
};

static THD_WORKING_AREA(hyperion_ui_thread_wa, 1024);
static THD_FUNCTION(hyperion_ui_thread, arg)
{
	hyperion_ui_dispmode_t last_mode = 0;

	(void) arg;

	chRegSetThreadName("ui");

	while (!chThdShouldTerminateX())
	{
		chMtxLock(&(ui.lock));
		if (ui.dispmode != last_mode)
		{
			last_mode = ui.dispmode;
			gdispGClear(ui.g, Black);
		}
		hyperion_ui_renderers[ui.dispmode](&ui);
		chMtxUnlock(&(ui.lock));

		gdispGFlush(ui.g);

		chThdSleepMilliseconds(10);
	}
}

void hyperion_ui_init_obj(hyperion_ui_t * ui, GDisplay * g)
{
	chDbgCheck((ui != NULL) && (g != NULL));

	memset(ui, 0, sizeof(*ui));
	ui->g = g;
	ui->w = gdispGGetWidth(ui->g);
	ui->h = gdispGGetHeight(ui->g);

	chMtxObjectInit(&(ui->lock));
}

void hyperion_ui_init(void)
{
	hyperion_ui_init_obj(&ui, gdispGetDisplay(0));

	fonts[0] = gdispOpenFont("DejaVuSans10");
	font_heights[0] = gdispGetFontMetric(fonts[0], fontHeight);

	fonts[1] = gdispOpenFont("DejaVuSans12");
	font_heights[1] = gdispGetFontMetric(fonts[1], fontHeight);

	fonts[2] = gdispOpenFont("DejaVuSans20");
	font_heights[2] = gdispGetFontMetric(fonts[2], fontHeight);

	ui.dispmode = HYPERION_UI_DISPMODE_LOGO;

	chThdCreateStatic(hyperion_ui_thread_wa, sizeof(hyperion_ui_thread_wa), NORMALPRIO-10, hyperion_ui_thread, NULL);
}

hyperion_ui_t * hyperion_ui_lock(void)
{
	chMtxLock(&(ui.lock));
	return &ui;
}
void hyperion_ui_unlock(hyperion_ui_t * _ui)
{
	chDbgCheck(_ui == &ui);
	chMtxUnlock(&(ui.lock));
}

void hyperion_ui_text(hyperion_ui_t * ui, coord_t x, coord_t y, coord_t w, uint8_t font_idx, uint8_t justify, const char * text)
{
	gdispGFillStringBox(ui->g, x, y, w, font_heights[font_idx], text, fonts[font_idx], White, Black, justify);
}


void hyperion_ui_render_boot(hyperion_ui_t * ui)
{
	coord_t         y = 0;
	char            buf[HYPERION_UI_MAX_LINE_TEXT_LEN];
	systime_t       ping = chVTGetSystemTimeX();
	uint32_t        up_secs = ST2S(ping);
	msg_t           ret;

	hyperion_ui_text(ui, 0, 0, ui->w, hyperion_ui_get_biggest_possible_font_idx(ui, "hyperion"), justifyCenter, "hyperion");
	y += font_heights[2] + 2;

	chsnprintf(buf, sizeof(buf) - 1, "HwSw %08x", HYPERION_HW_SW_VER);
	hyperion_ui_text(ui, 0, y, ui->w, 0, justifyLeft, buf);
	y += font_heights[0] + 2;

	chsnprintf(buf, sizeof(buf) - 1, "NodeId %d", hyperion_can.node_id);
	hyperion_ui_text(ui, 0, y, ui->w, 0, justifyLeft, buf);
	y += font_heights[0] + 2;

	if (hyperion_can.node_id != PHI_CAN_AUTO_ID)
	{
		ping = chVTGetSystemTimeX();
		ret = phi_can_xfer(
			&hyperion_can,
			PHI_CAN_PRIO_LOWEST,
			PHI_CAN_MSG_ID_ECHO,
			PHI_CAN_AUTO_ID_ALLOCATOR_NODE,
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
			chsnprintf(buf, sizeof(buf) - 1, "P: %2.2fms", ((float)ST2US(chVTGetSystemTimeX() - ping)) / 1000.0f);
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
	hyperion_ui_text(ui, 0, y, ui->w, 0, justifyLeft, buf);
	y += font_heights[0] + 2;

	chsnprintf(buf, sizeof(buf) - 1, "CanOk %d", hyperion_can.stat_process_rx);
	hyperion_ui_text(ui, 0, y, ui->w, 0, justifyLeft, buf);
	y += font_heights[0] + 2;

	hyperion_ui_text(ui, 0, y, ui->w, 0, justifyLeft, ui->state.boot.misc_text);
	y += font_heights[0] + 2;

	chsnprintf(buf, sizeof(buf) - 1, "UP %02d:%02d", up_secs / 60, up_secs % 60);
	hyperion_ui_text(ui, 0, gdispGGetHeight(ui->g) - font_heights[1] - 3, ui->w, 1, justifyLeft, buf);
}

void hyperion_ui_render_texts(hyperion_ui_t * ui)
{
	int i;
	coord_t y = 0;

	uint8_t flags;
	int     font_idx;
	int     justify;

	// Texts from the top
	for (i = 0; i < HYPERION_UI_DISPMODE_TEXTS_LINES; ++i)
	{
		flags = ui->state.text.flags[i];
		if (flags & HYPERION_UI_DISPMODE_TEXT_FLAGS_BOTTOM)
		{
			continue;
		}

		font_idx = (flags & 3);
		justify = (flags >> 2) & 3;

		if (font_idx == 0)
		{
			font_idx = hyperion_ui_get_biggest_possible_font_idx(ui, ui->state.text.lines[i]);
		}
		else
		{
			font_idx--;
		}

		ui->state.text.lines[i][HYPERION_UI_MAX_LINE_TEXT_LEN - 1] = 0;
		hyperion_ui_text(ui, 0, y, ui->w, font_idx, justify, ui->state.text.lines[i]);
		if (i == 0)
		{
			y += 16;
		} else {
			y += font_heights[font_idx] + 2;
		}
	}

	// Texts from the bottom
	y = gdispGGetHeight(ui->g) - font_heights[1] - 3;
	for (i = HYPERION_UI_DISPMODE_TEXTS_LINES - 1; i >= 0; --i)
	{
		flags = ui->state.text.flags[i];
		if (!(flags & HYPERION_UI_DISPMODE_TEXT_FLAGS_BOTTOM))
		{
			continue;
		}

		font_idx = (flags & 3);
		justify = (flags >> 2) & 3;

		if (font_idx == 0)
		{
			font_idx = hyperion_ui_get_biggest_possible_font_idx(ui, ui->state.text.lines[i]);
		}
		else
		{
			font_idx--;
		}

		ui->state.text.lines[i][HYPERION_UI_MAX_LINE_TEXT_LEN - 1] = 0;
		hyperion_ui_text(ui, 0, y, ui->w, font_idx, justify, ui->state.text.lines[i]);
		y -= font_heights[font_idx] + 2;
	}
}

uint8_t hyperion_ui_get_biggest_possible_font_idx(hyperion_ui_t * ui, const char * s)
{
	// Our fonts are from large to small
	if (gdispGetStringWidth(s, fonts[2]) < ui->w) return 2;
	if (gdispGetStringWidth(s, fonts[1]) < ui->w) return 1;
	return 0;
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
void hyperion_ui_render_logo(hyperion_ui_t * ui)
{
	char buf[HYPERION_UI_MAX_LINE_TEXT_LEN];

	chsnprintf(buf, sizeof(buf) - 1, "%08x", HYPERION_HW_SW_VER);
	hyperion_ui_text(ui, 0, 0, ui->w, 0, justifyCenter, buf);

	for (uint32_t row = 0; row < PHI_ARRLEN(phi_logo); ++row)
    {
		for (int byte = 0; byte < 8; ++byte)
	    {
			for (int bit = 0; bit < 8; ++bit)
			{
				if (phi_logo[row][byte] & (1 << (7-bit)))
				{
					gdispGDrawPixel(ui->g, 32 + (byte * 8) + bit, 16 + row, White);
				}
			}
	    }
    }
}

void hyperion_ui_render_callback(hyperion_ui_t * ui)
{
	ui->state.callback.func(ui, ui->state.callback.ctx);
}
