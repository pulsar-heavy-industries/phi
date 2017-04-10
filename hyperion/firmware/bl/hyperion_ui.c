#include "hyperion_ui.h"
#include "hyperion_can.h"

static font_t font;
static coord_t font_height;
static coord_t lcd_w, lcd_h;
#define hyperion_ui_text(x, y, w, justify, text) gdispFillStringBox(x, y, w, font_height, text, font, White, Black, justify)

extern char boot_user_status[];

static THD_WORKING_AREA(hyperion_ui_thread_wa, 1024);
static THD_FUNCTION(hyperion_ui_thread, arg)
{
    (void) arg;

    chRegSetThreadName("ui");

    while (!chThdShouldTerminateX())
    {
        hyperion_ui_render();
        gdispFlush();
        chThdSleepMilliseconds(50);
    }
}

void hyperion_ui_init(void)
{
    lcd_w = gdispGetWidth();
    lcd_h = gdispGetHeight();
    font = gdispOpenFont("DejaVuSans10");
    font_height = gdispGetFontMetric(font, fontHeight);

    chThdCreateStatic(hyperion_ui_thread_wa, sizeof(hyperion_ui_thread_wa), NORMALPRIO-10, hyperion_ui_thread, NULL);
}

void hyperion_ui_render(void)
{
    coord_t         y = 0;
    char            buf[HYPERION_UI_MAX_LINE_TEXT_LEN];
    systime_t       ping = chVTGetSystemTimeX();
    uint32_t        up_secs = ST2S(ping);
    msg_t           ret;

    hyperion_ui_text(0, 0, lcd_w, justifyCenter, "hyperion-bl");
    y += font_height + 5;

    chsnprintf(buf, sizeof(buf) - 1, "HwSw %08x", HYPERION_HW_SW_VER);
    hyperion_ui_text(0, y, lcd_w, justifyLeft, buf);
    y += font_height + 2;

    chsnprintf(buf, sizeof(buf) - 1, "NodeId %d", hyperion_can.node_id);
    hyperion_ui_text(0, y, lcd_w, justifyLeft, buf);
    y += font_height + 2;

    if (hyperion_can.node_id != PHI_CAN_AUTO_ID)
    {
        ret = phi_can_xfer(
            &hyperion_can,
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
    hyperion_ui_text(0, y, lcd_w, justifyLeft, buf);
    y += font_height + 2;

	hyperion_ui_text(0, y, lcd_w, justifyLeft, boot_user_status);

//    chsnprintf(buf, sizeof(buf) - 1, "CanOk %d", hyperion_can.stat_process_rx);
//    hyperion_ui_text(0, y, lcd_w, justifyLeft, buf);
//    y += font_height + 2;
//
//    chsnprintf(buf, sizeof(buf) - 1, "UP %02d:%02d", up_secs / 60, up_secs % 60);
//    hyperion_ui_text(0, lcd_h - font_height - 3, lcd_w, justifyLeft, buf);
}
