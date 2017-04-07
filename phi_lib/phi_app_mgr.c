#include "phi_lib/phi_app_mgr.h"
#include "phi_lib/phi_mtf.h"

#define PHI_APP_MGR_MTF_NUM_MSGS 8

static const phi_app_desc_t * phi_app_cur_desc = NULL;
static void * phi_app_cur_ctx = NULL;
static phi_mtf_static_storage_t(phi_app_mgr_mtf_storage, PHI_APP_MGR_MTF_NUM_MSGS);
static phi_mtf_t phi_app_mgr_mtf;
static virtual_timer_t phi_app_mgr_tick_timer;

static void phi_app_mgr_do_switch_app(const phi_app_desc_t * desc, void * ctx);

static thread_t * phi_app_mgr_thread;
static THD_WORKING_AREA(phi_app_mgr_thread_wa, 1024);
static THD_FUNCTION(phi_app_mgr_thread_func, arg)
{
    (void) arg;

    chRegSetThreadName("app_mgr");

    while (!chThdShouldTerminateX())
    {
        phi_mtf_thread_process_msg(&phi_app_mgr_mtf, TIME_INFINITE);
    }
}

static void tick_timer_cb(void * arg)
{
    (void) arg;

    chSysLockFromISR();
    phi_mtf_async_callI(
        &phi_app_mgr_mtf,
        phi_app_cur_desc->tick,
        1,
        phi_app_cur_ctx
    );
    chVTSetI(&phi_app_mgr_tick_timer, phi_app_cur_desc->tick_interval, tick_timer_cb, NULL);
    chSysUnlockFromISR();
}

void phi_app_mgr_init(void)
{
    chDbgCheck(phi_app_cur_desc == NULL);

    phi_mtf_init_static(&phi_app_mgr_mtf, PHI_APP_MGR_MTF_NUM_MSGS, phi_app_mgr_mtf_storage, sizeof(phi_app_mgr_mtf_storage));
    phi_app_mgr_thread = chThdCreateStatic(
       phi_app_mgr_thread_wa,
       sizeof(phi_app_mgr_thread_wa),
       HIGHPRIO - 9,
       phi_app_mgr_thread_func,
       NULL);
}

void phi_app_mgr_switch_app(const phi_app_desc_t * desc, void * ctx)
{
	// If running inside our MTF, do the switch async
	if (chThdGetSelfX()  == phi_app_mgr_thread) {
		phi_mtf_async_call(
			&phi_app_mgr_mtf,
			TIME_IMMEDIATE,
			phi_app_mgr_do_switch_app,
			2,
			desc,
			ctx
		);
	}
	else
	{
		phi_mtf_call(
			&phi_app_mgr_mtf,
			phi_app_mgr_do_switch_app,
			2,
			desc,
			ctx
		);
	}
}

static  void phi_app_mgr_do_switch_app(const phi_app_desc_t * desc, void * ctx)
{
	// Stop current app, if we have one
	if (NULL != phi_app_cur_desc)
	{
		if (0 != phi_app_cur_desc->tick_interval)
		{
			chVTReset(&phi_app_mgr_tick_timer);
		}

		phi_app_cur_desc->stop(phi_app_cur_ctx);
		phi_app_cur_desc = NULL;
		phi_app_cur_ctx = NULL;
	}

	// Start new app
	phi_app_cur_desc = desc;
	phi_app_cur_ctx = ctx;
	phi_app_cur_desc->start(phi_app_cur_ctx);

	if (0 != desc->tick_interval)
	{
		chVTObjectInit(&phi_app_mgr_tick_timer);
		chVTSet(&phi_app_mgr_tick_timer, desc->tick_interval, tick_timer_cb, NULL);
	}
}

void phi_app_mgr_notify_encoder_event(uint8_t node_id, uint8_t encoder_num, int8_t val_change)
{
    if ((NULL != phi_app_cur_desc) &&
        (NULL != phi_app_cur_desc->encoder_event))
    {
        phi_mtf_async_call(
            &phi_app_mgr_mtf,
            MS2ST(50),
            phi_app_cur_desc->encoder_event,
            4,
            phi_app_cur_ctx, node_id, encoder_num, val_change
        );
    }
}

void phi_app_mgr_notify_btn_event(uint8_t node_id, uint8_t btn_num, phi_btn_event_t event, uint32_t param)
{
    if ((NULL != phi_app_cur_desc) &&
        (NULL != phi_app_cur_desc->btn_event))
    {
        phi_mtf_async_call(
            &phi_app_mgr_mtf,
            MS2ST(50),
            phi_app_cur_desc->btn_event,
            5,
            phi_app_cur_ctx, node_id, btn_num, event, param
        );
    }
}

void phi_app_mgr_notify_pot_event(uint8_t node_id, uint8_t pot_num, uint8_t val)
{
    if ((NULL != phi_app_cur_desc) &&
        (NULL != phi_app_cur_desc->pot_event))
    {
        phi_mtf_async_call(
            &phi_app_mgr_mtf,
            MS2ST(50),
            phi_app_cur_desc->pot_event,
            4,
            phi_app_cur_ctx, node_id, pot_num, val
        );
    }
}

void phi_app_mgr_notify_midi_cc(phi_midi_port_t port, uint8_t ch, uint8_t cc, uint8_t val)
{
    /* This will get called from some MIDI thread */
    if ((NULL != phi_app_cur_desc) &&
        (NULL != phi_app_cur_desc->midi_cc))
    {
        phi_mtf_call(
            &phi_app_mgr_mtf,
            phi_app_cur_desc->midi_cc,
            5,
            phi_app_cur_ctx,
            port,
            ch,
            cc,
            val
        );
    }
}

void phi_app_mgr_notify_midi_sysex(phi_midi_port_t port, uint8_t cmd, const void * data, size_t data_len)
{
    /* This will get called from some MIDI thread */
    if ((NULL != phi_app_cur_desc) &&
        (NULL != phi_app_cur_desc->midi_sysex))
    {
        phi_mtf_call(
            &phi_app_mgr_mtf,
            phi_app_cur_desc->midi_sysex,
            5,
            phi_app_cur_ctx,
            port,
            cmd,
            data,
            data_len
        );
    }
}

void phi_app_mgr_notify_can_cmd(uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t xfer_id, const uint8_t * data, size_t len)
{
	/* This will get called from CAN thread */
	    if ((NULL != phi_app_cur_desc) &&
	        (NULL != phi_app_cur_desc->can_cmd))
	    {
	    	phi_mtf_async_call(
	            &phi_app_mgr_mtf,
				MS2ST(50),
	            phi_app_cur_desc->can_cmd,
				7,
				phi_app_cur_ctx,
				prio,
				msg_id,
				src,
				xfer_id,
				data,
				len
			);
	    }
}
