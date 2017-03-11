#include "cenx4_app_test.h"
#include "cenx4_app_cfg.h"
#include "cenx4_midi.h"
#include "cenx4_ui.h"


const phi_app_desc_t cenx4_app_test_desc = {
    .start = cenx4_app_test_start,
    .stop = cenx4_app_test_stop,
    .encoder_event = cenx4_app_test_encoder_event,
    .btn_event = cenx4_app_test_btn_event,
};

void cenx4_app_test_start(void * _ctx)
{
    cenx4_app_test_context_t * ctx = (cenx4_app_test_context_t *) _ctx;
    cenx4_ui_t * ui;
    uint8_t i;

    chDbgCheck(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    ctx->num_pots = 4; // TODO constant
    for (i = 0; i < ctx->num_pots; ++i) {
        ctx->pots[i] = i ? i * 5 : 1;
    }

    // Move our displays into pots mode for test
    for (i = 0; i < 2; ++i)
    {
        ui = cenx4_ui_lock(i);

        ui->dispmode = CENX4_UI_DISPMODE_SPLIT_POT;

        memset(&(ui->state), 0, sizeof(ui->state));

        ui->state.split_pot.pots[0].flags =
                CENX4_UI_DISPMODE_POT_FLAGS_RENDER_VAL |
                CENX4_UI_DISPMODE_POT_FLAGS_ROUND |
                CENX4_UI_DISPMODE_POT_FLAGS_MERGE_BOTTOM |
                CENX4_UI_DISPMODE_POT_FLAGS_FILL |
                CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO;
        chsnprintf(ui->state.split_pot.pots[0].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot %d", (i * 2) + 1);
        ui->state.split_pot.pots[0].val = ctx->pots[i * 2];

        ui->state.split_pot.pots[1].flags = ui->state.split_pot.pots[0].flags;
        chsnprintf(ui->state.split_pot.pots[1].text_top, CENX4_UI_MAX_LINE_TEXT_LEN - 1, "Pot %d", (i * 2) + 2);
        ui->state.split_pot.pots[1].val = ctx->pots[(i * 2) + 1];

        cenx4_ui_unlock(ui);
    }

    // Move our Berry modules into test mode
    //cenx4_app_test_berry_enter_test_mode(ctx, 10);
}

void cenx4_app_test_stop(void * ctx)
{
    (void) ctx;
}

// NOTES:
// - For slave, this needs to do nothing
// - For master, node_id is either 0 for encoders on the master,
//   or need to figure out module number from configuration...
void cenx4_app_test_encoder_event(void * _ctx, uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    cenx4_app_test_context_t * ctx = (cenx4_app_test_context_t *) _ctx;
    (void) ctx; (void) _ctx; (void) node_id;

    uint8_t lcd_num = encoder_num / 2;
    uint8_t pot_num = encoder_num % 2;
    cenx4_ui_t * ui = cenx4_ui_lock(lcd_num);
    ui->state.split_pot.pots[pot_num].val += val_change;
    cenx4_ui_unlock(ui);


#if CENX4_IS_SLAVE
    phi_midi_pkt_t pkt;

    pkt.chn = 2;
    pkt.event = 0xB; // Control Change
    pkt.val1 = encoder_num;
    pkt.val2 = 64 + val_change; // "Relative Binary Offset". Positive offsets are sent as offset plus 64 and negative offsets are sent as 64 minus offset

    phi_midi_tx_pkt(PHI_MIDI_PORT_USB, &pkt);
#endif
}

// NOTES:
// - For slave, this needs to do nothing
// - For master, node_id is either 0 for encoders on the master,
//   or need to figure out module number from configuration...
void cenx4_app_test_btn_event(void * _ctx, uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
    cenx4_app_test_context_t * ctx = (cenx4_app_test_context_t *) _ctx;
    (void) ctx; (void) _ctx; (void) node_id; (void) param;

    uint8_t lcd_num = btn_num / 2;
    uint8_t pot_num = btn_num % 2;
    cenx4_ui_t * ui = cenx4_ui_lock(lcd_num);

    switch (event)
    {
    case PHI_BTN_EVENT_PRESSED:
        ui->state.split_pot.pots[pot_num].text_bottom[0] = '+';
        break;

    case PHI_BTN_EVENT_HELD:
        ui->state.split_pot.pots[pot_num].text_bottom[0] = '@';
        break;

    case PHI_BTN_EVENT_RELEASED:
        ui->state.split_pot.pots[pot_num].text_bottom[0] = '-';
        break;

    default:
        ui->state.split_pot.pots[pot_num].text_bottom[0] = '?';
        break;
    }

    cenx4_ui_unlock(ui);
}
