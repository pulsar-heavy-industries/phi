#include "phi_lib/phi_can.h"

#define MBOX 1
extern volatile unsigned int can_errs, loops;
static THD_FUNCTION(phi_can_read_thread, arg)
{
    phi_can_t * can = (phi_can_t *) arg;
    CANDriver * drv = can->cfg->drv;
    event_listener_t rx_full_evt;
    event_listener_t err_evt;
    eventmask_t mask;
    CANRxFrame * rxmsg = NULL;
    volatile eventflags_t errors = 0;
    msg_t ret;

    chRegSetThreadName("canrdr");
    chEvtRegister(&(drv->rxfull_event), &rx_full_evt, 0);
    chEvtRegister(&(drv->error_event), &err_evt, 1);

    while (!chThdShouldTerminateX())
    {
        if ((mask = chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100))) == 0)
        {
              continue;
        }

        // RX FULL
        if(mask & EVENT_MASK(0))
        {
            for (;;)
            {
                // Get a buffer if we need one
                if (NULL == rxmsg)
                {
                    ret = chMBFetch(&(can->free_bufs), (msg_t *) &rxmsg, TIME_INFINITE);
                    chDbgAssert(ret == MSG_OK, "WTF");
                }

                ret = canReceive(drv, CAN_ANY_MAILBOX, rxmsg, TIME_IMMEDIATE);
                if (ret != MSG_OK)
                {
                    // Couldn't receive, we're done here
                    break;
                }

                // Got a buffer, put that on queue
                ret = chMBPost(&(can->rx_bufs), (msg_t) rxmsg, TIME_INFINITE);
                chDbgAssert(ret == MSG_OK, "WTF");
                rxmsg = NULL;
            }
        }

        // ERROR
        if (mask & EVENT_MASK(1))
        {
            errors |= chEvtGetAndClearFlags(&err_evt);
            ++(can->stat_read_errs);
        }
    }

    chEvtUnregister(&(drv->rxfull_event), &rx_full_evt);
    chEvtUnregister(&(drv->error_event), &err_evt);
}

void process_rx(phi_can_t * can, const CANRxFrame * rxmsg)
{
    phi_can_eid_t eid = EID_UNPACK(rxmsg->EID);
    phi_can_msg_xfer_desc_t xfer_desc = { .val = rxmsg->data8[0] };
    const uint8_t * data = &(rxmsg->data8[1]);
    uint8_t data_len = rxmsg->DLC - 1;
    uint32_t i;
    uint32_t t_xfer_id;
    phi_can_xfer_t * xfer;
    systime_t now;

    ++can->stat_process_rx;

    if ((1 != rxmsg->IDE) || (0 != rxmsg->RTR))
    {
        return;
    }

    // Not to us?
    if (((0 != eid.dst) &&
        (can->node_id != eid.dst)) &&
        ((can->node_id == PHI_CAN_AUTO_ID_ALLOCATOR_NODE) && (eid.src != 0)))
    {
        return;
    }

    // We must have at least 1 byte of data - the xfer id
    if (0 == rxmsg->DLC)
    {
        ++can->stat_process_rx_err;
        return;
    }

    // See if we recognize this transfer id
    chMtxLock(&(can->xfer_lock));

    t_xfer_id = PACK_XFER_ID(
        eid.is_reply ? eid.src : can->node_id,
        eid.is_reply ? can->node_id : eid.src,
        xfer_desc.chan_id,
        eid.is_reply ? 1 : 0
    );

    xfer = NULL;
    for (i = 0; i < PHI_CAN_MAX_XFERS; ++i)
    {
        if (can->xfers[i].xfer_id == t_xfer_id) {
            xfer = &(can->xfers[i]);
            break;
        }

    }

    // See if this transfer might've timeouted
    now = chVTGetSystemTimeX();
    if ((xfer != NULL) && (xfer->last_seen_at != 0))
    {
        if ((now - xfer->last_seen_at) > PHI_CAN_DEFAULT_TIMEOUT)
        {
            phi_can_free_xfer(can, FALSE, xfer);
            xfer = NULL;
            ++can->stat_process_rx_timeout;
        }
        else
        {
            xfer->last_seen_at = now;
        }
    }

    // See if this is a start of a transfer
    if (xfer_desc.start)
    {
        // If we didn't recognize this transfer, it means we were not expecting it.
        // It is either an incoming request, or a reply that came too late.
        if (NULL == xfer)
        {
            if (xfer_desc.toggle_or_is_reply)
            {
                // This is a reply to something we don't recognize, nothing to do about it
            	++can->stat_process_rx_err;
            }
            else
            {
                // We got a start of a single frame transfer
                if (xfer_desc.end)
                {
                    // handle incoming message: it may or may not want to send a reply.
                    // if no reply, our work here is done. if it does want to send a reply
                    // that reply may be single or multi msg. it needs to get replied
                    // to the same xfer id that sent it.
                    if (eid.src == PHI_CAN_AUTO_ID)
                    {
                        if ((PHI_CAN_AUTO_ID_ALLOCATOR_NODE == can->node_id) &&
                            (7 == data_len))
                        {
                            phi_can_auto_handle_incoming_msg(can, eid.prio, eid.msg_id, eid.dst, xfer_desc.chan_id, data);
                        }
                    }
                    else
                    {
                        phi_can_handle_incoming_msg(can, eid.prio, eid.msg_id, eid.src, xfer_desc.chan_id, data, data_len);
                    }
                }
                // Start of a multi-frame transfer
                else
                {
                    // Try to allocate an xfer object to hold the data for this incoming
                    // multi-msg request. If allocate fails, nothing we can do.
                    // If it succeeds we need to start a timer to release this multi-msg
                    // request after some time.
                    xfer = phi_can_alloc_xfer(can, FALSE, FALSE, eid.src, xfer_desc.chan_id, TIME_IMMEDIATE);
                    if (NULL == xfer)
                    {
                        can->stat_process_rx_dropped_multi_reqs++;
                    }
                    else
                    {
                        // Store the data we have so far
                    	if (data_len == 7)
                    	{
							xfer->expected_crc = data[0] << 8 | data[1];
							memcpy(xfer->rx_data, &(data[2]), 5);
							xfer->rx_len = 5;
                    	}
                    	else
                    	{
                    		++can->stat_process_rx_err;
                    	}
                    }
                }
            }
        }
        else
        {
            // A start of a transfer we recognize, which means we got ourselves a start of a reply
            chDbgAssert(xfer->xfer_id & 1, "WTF"); // TODO is_reply - will fail if we get a duplicate start. what we should do is reset the transfer
            chDbgAssert(xfer->rx_len == 0, "WTF");

            // A single message reply
            if (xfer_desc.end)
            {
                xfer->rx_len = data_len;
                memcpy(xfer->rx_data, data, data_len);

                // signal whoever is waiting on this reply that we're done
                chBSemSignal(&(xfer->rx_done));

                // Whoever waits on the reply is also the one that releases it. If the waiter
                // timeouts, it will attempt to lock chMtxLock and will execute once this loop
                // is done, afterwards releasing the xfer
            }
            else
            {
                // Start of a multi-message reply
                if (data_len == 7)
                {
					xfer->expected_crc = data[0] << 8 | data[1];
					memcpy(xfer->rx_data, &(data[2]), 5);
					xfer->rx_len = 5;
                }
                else
                {
                	++can->stat_process_rx_err;
                }
            }
        }
    }
    // End of a recognized multi-msg transfer
    else if ((xfer_desc.end) && (NULL != xfer))
    {
        bool error = (
        	(xfer->toggle != xfer_desc.toggle_or_is_reply) ||
			((xfer->rx_len + data_len) > sizeof(xfer->rx_data)) ||
			(phi_crc16(xfer->rx_data, (xfer->rx_len + data_len)) != xfer->expected_crc)
		);

        if (!error)
        {
            memcpy(xfer->rx_data + xfer->rx_len, data, data_len);
            xfer->rx_len += data_len;
        }
        else
        {
        	++can->stat_process_rx_err;
        }

        if (xfer->xfer_id & 1) // TODO is_reply
        {
            // signal whoever is waiting on this reply that we're done
            chBSemSignal(&(xfer->rx_done));
        }
        else
        {
            // Finished receiving a request
            if (!error)
            {
                phi_can_handle_incoming_msg(can, eid.prio, eid.msg_id, eid.src, xfer_desc.chan_id, xfer->rx_data, xfer->rx_len);
            }
            phi_can_free_xfer(can, FALSE, xfer);
        }
    }
    // Data in a recognized multi message transfer
    else if (NULL != xfer)
    {
        // Looks like we missed a packet or too much data :(
        if ((xfer->toggle != xfer_desc.toggle_or_is_reply) ||
            ((xfer->rx_len + data_len) > sizeof(xfer->rx_data)))
        {
            if (xfer->xfer_id & 1) // TODO is_reply
            {
                // signal whoever is waiting on this reply that we're done
                // TODO set error flag
            	++can->stat_process_rx_err;
                chBSemSignal(&(xfer->rx_done));
            }
            else
            {
                phi_can_free_xfer(can, FALSE, xfer);
            }
        }
        else
        {
            xfer->toggle = !xfer->toggle;

            memcpy(xfer->rx_data + xfer->rx_len, data, data_len);
            xfer->rx_len += data_len;
        }
    }

    chMtxUnlock(&(can->xfer_lock));
}

static THD_FUNCTION(phi_can_process_thread, arg)
{
    phi_can_t * can = (phi_can_t *) arg;
    CANRxFrame * rxmsg = NULL;

    chRegSetThreadName("canproc");

    while (!chThdShouldTerminateX())
    {
        if (MSG_OK != chMBFetch(&(can->rx_bufs), (msg_t *) &rxmsg, MS2ST(100)))
        {
              continue;
        }

        process_rx(can, rxmsg);

        chMBPost(&(can->free_bufs), (msg_t) rxmsg, TIME_INFINITE);
        rxmsg = NULL;
    }
}

bool phi_can_init(phi_can_t * can, const phi_can_config_t * cfg, uint8_t node_id)
{
    int i;

#if PHI_CAN_ENABLE_ALLOCATOR
    chDbgCheck(PHI_CAN_AUTO_ID_ALLOCATOR_NODE == node_id);
#endif

    memset(can, 0, sizeof(*can));

    can->cfg = cfg;
    can->next_chan_id = 1;

    chMtxObjectInit(&(can->xfer_lock));
    chSemObjectInit(&(can->xfer_sem), PHI_CAN_MAX_XFERS);


    chMBObjectInit(&(can->free_bufs), (msg_t *) &(can->free_bufs_q), PHI_CAN_MSG_POOL_SIZE);
    chMBObjectInit(&(can->rx_bufs), (msg_t *) &(can->rx_bufs_q), PHI_CAN_MSG_POOL_SIZE);
    for (i = 0; i < PHI_CAN_MSG_POOL_SIZE; ++i)
    {
        chMBPost(&(can->free_bufs), (msg_t)&(can->bufs[i]), TIME_INFINITE);
    }

    canObjectInit(can->cfg->drv);
    phi_can_set_node_id(can, node_id);

    can->process_rx_thread = chThdCreateStatic(can->process_rx_thread_wa, sizeof(can->process_rx_thread_wa), HIGHPRIO - 10, phi_can_process_thread, can);
    can->read_thread = chThdCreateStatic(can->read_thread_wa, sizeof(can->read_thread_wa), HIGHPRIO, phi_can_read_thread, can);

    return TRUE;
}

void phi_can_set_node_id(phi_can_t * can, uint8_t node_id)
{
    const CANFilter filters[] = {
        // Filter for our node id
        {
            .filter = 1,
            .mode = 0, // mask
            .scale = 1,
            .assignment = 0,
            .register1 = (node_id << (9 + 3)) |  0x04, // 00000 00000000 0000011 0000000 0 0 | 1 0 0
            .register2 = (0x7f << (9 + 3))|  0x07,
        },
        // Filter for SRC NODE ID 0 - only used for allocators
        {
            .filter = 2,
            .mode = 0, // mask
            .scale = 1,
            .assignment = 0,
            .register1 = (0 << (3)) |  0x04, // 00000 00000000 0000011 0000000 0 0 | 1 0 0
            .register2 = (0x7f << (3))|  0x07,
        },
    };

    chDbgCheck(0 == can->node_id);

    canStop(can->cfg->drv);
    canSTM32SetFilters(1, node_id == PHI_CAN_AUTO_ID_ALLOCATOR_NODE ? 2 : 1, filters);
    canStart(can->cfg->drv, can->cfg->drv_cfg);

    can->node_id = node_id;
}

msg_t phi_can_xfer(
    phi_can_t * can,
    uint8_t prio,
    uint8_t msg_id,
    uint8_t dst,
    const uint8_t * req_buf,
    size_t req_len,
    uint8_t * resp_buf,
    uint32_t max_resp_len,
    uint32_t * out_resp_len,
    systime_t timeout
)
{
    phi_can_xfer_t * xfer = NULL;
    msg_t ret = MSG_OK;

    chDbgCheck(req_len <= PHI_CAN_MAX_MSG_DATA_LEN);

    // If we expect a response or if we are sending more than 1 message, we will need an xfer object
    if ((max_resp_len && resp_buf) ||
        (req_len > 7))
    {
        chDbgAssert(max_resp_len <= PHI_CAN_MAX_MSG_DATA_LEN, "WTF");

        xfer = phi_can_alloc_xfer(can, TRUE, TRUE, (can->node_id == PHI_CAN_AUTO_ID) ? PHI_CAN_AUTO_ID_ALLOCATOR_NODE : dst, 0, timeout);
        if (NULL == xfer)
        {
            return MSG_RESET;
        }
    }

    ret = phi_can_send_internal(can, FALSE, xfer ? XFER_ID_GET_CHAN(xfer->xfer_id) : 0, prio, msg_id, dst, req_buf, req_len, timeout);
    if (MSG_OK != ret)
    {
        goto lbl_ret;
    }

    // See if we're waiting for a reply
    if (max_resp_len && resp_buf)
    {
        ret = chBSemWaitTimeout(&(xfer->rx_done), timeout);
        if (MSG_OK != ret)
        {
            goto lbl_ret;
        }

        // Got reply, give it to caller
        if (max_resp_len > xfer->rx_len)
        {
            max_resp_len = xfer->rx_len;
        }
        memcpy(resp_buf, xfer->rx_data, max_resp_len);

        if (out_resp_len)
        {
            *out_resp_len = xfer->rx_len;
        }
    }

    // Release xfer
lbl_ret:
    if (xfer)
    {
        phi_can_free_xfer(can, TRUE, xfer);
    }

    return ret;
}

phi_can_xfer_t * phi_can_alloc_xfer(phi_can_t * can, bool lock, bool is_reply, uint8_t node_id, uint8_t chan_id, systime_t timeout)
{
    phi_can_xfer_t * xfer;
    int i;

    chDbgAssert(lock || (timeout == TIME_IMMEDIATE), "WTF");

    if (MSG_TIMEOUT == chSemWaitTimeout(&(can->xfer_sem), timeout))
    {
        return NULL;
    }

    if (lock)
    {
        chMtxLock(&(can->xfer_lock));
    }

    if (0 == chan_id)
    {
        chan_id = can->next_chan_id;
        can->next_chan_id = (can->next_chan_id + 1) & 0x1F;
        if (0 == can->next_chan_id)
        {
            can->next_chan_id = 1;
        }
    }

	uint32_t _x = PACK_XFER_ID(
		is_reply ? node_id : can->node_id,
		is_reply ? can->node_id : node_id,
		chan_id,
		is_reply ? 1 : 0
	);

	// Sanity - this id should never be in the table
#if PHI_CAN_SANITY_TESTS
        for (i = 0; i < PHI_CAN_MAX_XFERS; ++i)
        {
            chDbgAssert(can->xfers[i].xfer_id != _x, "WTF");
        }
#endif

    for (i = 0; i < PHI_CAN_MAX_XFERS; ++i)
    {
        xfer = &(can->xfers[i]);
        if (xfer->xfer_id == 0)
        {
            chBSemObjectInit(&(xfer->rx_done), TRUE);
            xfer->toggle = false;
            xfer->rx_len = 0;

            // replies timeout in phi_can_xfer, but requests might never get completed
            // so we store their time and the rx loop will monitor it
            xfer->last_seen_at = is_reply ? 0 : chVTGetSystemTimeX();

            xfer->xfer_id = _x;

            if (lock)
            {
                chMtxUnlock(&(can->xfer_lock));
            }

            return xfer;
        }
    }

    chDbgAssert(lock == FALSE, "WTF");
    return NULL;
}

void phi_can_free_xfer(phi_can_t * can, bool lock, phi_can_xfer_t * xfer)
{
    chDbgAssert(xfer->xfer_id, "WTF");

    if (lock)
    {
        chMtxLock(&(can->xfer_lock));
    }
    xfer->rx_len = 0;
    xfer->expected_crc = 0;
    memset(xfer->rx_data, 0, sizeof(xfer->rx_data));
    xfer->xfer_id = 0;

    if (lock)
    {
        chMtxUnlock(&(can->xfer_lock));
    }

    chSemSignal(&(can->xfer_sem));
}

void phi_can_handle_incoming_msg(phi_can_t * can, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, uint8_t data_len)
{
    uint32_t i;

    if (src == PHI_CAN_AUTO_ID)
    {
        return;
    }

    for (i = 0; i < can->cfg->n_handlers; ++i)
    {
        const phi_can_msg_handler_t * handler = &(can->cfg->handlers[i]);
        if (handler->msg_id == msg_id)
        {
            handler->handler(can, handler->context, prio, msg_id, src, chan_id, data, data_len);
            break;
        }
    }
}

msg_t phi_can_send_reply(phi_can_t * can, uint8_t prio, uint8_t msg_id,  uint8_t dst, uint8_t chan_id, const uint8_t * data, uint8_t data_len, systime_t timeout)
{
    if (!chan_id)
    {
        return MSG_RESET;
    }

    return phi_can_send_internal(can, TRUE, chan_id, prio, msg_id, dst, data, data_len, timeout);
}

msg_t phi_can_send_internal(
    phi_can_t * can,
    bool is_reply,
    uint8_t chan_id,
    uint8_t prio,
    uint8_t msg_id,
    uint8_t dst,
    const uint8_t * data,
    uint8_t data_len,
    systime_t timeout
) {
    phi_can_msg_xfer_desc_t xfer_desc;
    CANTxFrame frame;
    bool toggle = false;
    uint16_t crc;
    phi_can_eid_t eid = {
        .prio = prio,
        .msg_id = msg_id,
        .dst = dst,
        .src = can->node_id,
        .reserved = 0,
        .is_reply = is_reply ? 1 : 0,
    };

    // Single message reply
    if (data_len <= 7)
    {
        xfer_desc.start = 1;
        xfer_desc.end = 1;
        xfer_desc.toggle_or_is_reply = is_reply;
        xfer_desc.chan_id = chan_id;

        frame.DLC = data_len + 1;
        frame.RTR = 0;
        frame.IDE = 1;
        // NOTES:
        // hopefully we can filter on dst, if not maybe rearrange fields
        // need each node to filter on destination that is either 0 (broadcast)
        // or their can->node_id
        frame.EID = EID_PACK(eid);
        frame.data8[0] = xfer_desc.val;

        if (data_len)
        {
            memcpy(&(frame.data8[1]), data, data_len);
        }

        return canTransmit(can->cfg->drv, MBOX, &frame, timeout);
    }
    // Multi message reply
    else
    {
        // Send start frame
        xfer_desc.start = 1;
        xfer_desc.end = 0;
        xfer_desc.toggle_or_is_reply = is_reply;
        xfer_desc.chan_id = chan_id;

        crc = phi_crc16(data, data_len);

        frame.DLC = 8; // 2 bytes for crc, 1 byte for xfer desc, we're left with 5 for data
        frame.RTR = 0;
        frame.IDE = 1;
        frame.EID = EID_PACK(eid);
        frame.data8[0] = xfer_desc.val;
        frame.data8[1] = (crc >> 8) & 0xFF;
        frame.data8[2] = crc & 0xFF;
        frame.data8[3] = data[0];
        frame.data8[4] = data[1];
        frame.data8[5] = data[2];
        frame.data8[6] = data[3];
        frame.data8[7] = data[4];

        data += 5;
        data_len -= 5;

        for (;;)
        {
            int chunk_size = data_len <= 7 ? data_len : 7;

            if (MSG_OK != canTransmit(can->cfg->drv, MBOX, &frame, timeout))
            {
                return MSG_TIMEOUT;
            }

            if (data_len == 0)
            {
                break;
            }

            // Prepare the next frame
            xfer_desc.start = 0;
            xfer_desc.end = data_len == chunk_size;
            xfer_desc.toggle_or_is_reply = toggle ? 1 : 0;
            xfer_desc.chan_id = chan_id;

            frame.DLC = 1 + chunk_size;
            frame.RTR = 0;
            frame.IDE = 1;
            frame.EID = EID_PACK(eid);
            frame.data8[0] = xfer_desc.val;
            memcpy(&(frame.data8[1]), data, chunk_size);

            data += chunk_size;
            data_len -= chunk_size;
            toggle = !toggle;
        }

        return MSG_OK;
    }
}

void phi_can_handle_sysinfo(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    phi_can_msg_data_sysinfo_t sysinfo = {
        .can_version = PHI_CAN_VERSION,
        .dev_id = can->cfg->dev_id,
        .hw_sw_ver = can->cfg->hw_sw_ver,
        .reserved = {0, },
    };

    (void) context;
    (void) data;
    (void) len;

    phi_can_send_reply(can, prio, msg_id, src, chan_id, (const uint8_t *) &sysinfo, sizeof(sysinfo), PHI_CAN_DEFAULT_TIMEOUT);
}

void phi_can_handle_echo(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len)
{
    (void) context;

    phi_can_send_reply(can, prio, msg_id, src, chan_id, data, len, PHI_CAN_DEFAULT_TIMEOUT);
}

// auto id allocation
msg_t phi_can_auto_get_id(phi_can_t * can)
{
    msg_t ret;
    uint8_t req[7], resp[7];
    uint16_t crc;
    uint32_t resp_len;
    uint8_t uid[PHI_CAN_AUTO_ID_UNIQ_ID_LEN];

    // Sanity
    chDbgCheck(0 == can->node_id);

    // Get uid
    phi_can_auto_get_dev_uid(can, uid);

    // Random 0-5msec delay
    chThdSleepMicroseconds(50 + (rand() % 5000));

    // Send part 1 - uid[0-5]
    req[0] = PHI_CAN_AUTO_ID_OP_PART1;
    memcpy(&(req[1]), &(uid[0]), 6);
    crc = phi_crc16(req, 7);

    ret = phi_can_xfer(can, PHI_CAN_AUTO_ID_PRIO, crc & 0xFF, (crc >> 8) & 0x7F, req, sizeof(req), resp, sizeof(resp), &resp_len, PHI_CAN_DEFAULT_TIMEOUT);
    if (MSG_OK != ret)
    {
        goto lbl_ret;
    }

    if ((resp_len != 7) ||
        (0 != memcmp(req, resp, resp_len)))
    {
        ret = MSG_RESET;
        goto lbl_ret;
    }

    // Random 0-5msec delay
    chThdSleepMicroseconds(50 + (rand() % 5000));

    // Send part 2 - uid[6-11]
    req[0] = PHI_CAN_AUTO_ID_OP_PART2;
    memcpy(&(req[1]), &(uid[6]), 6);
    crc = phi_crc16(req, 7);

    ret = phi_can_xfer(can, PHI_CAN_AUTO_ID_PRIO, crc & 0xFF, (crc >> 8) & 0x7F, req, sizeof(req), resp, sizeof(resp), &resp_len, PHI_CAN_DEFAULT_TIMEOUT);
    if (MSG_OK != ret)
    {
        goto lbl_ret;
    }

    if ((resp_len != 7) ||
        (0 != memcmp(req, resp, resp_len)))
    {
        ret = MSG_RESET;
        goto lbl_ret;
    }

    // Random 0-5msec delay
    chThdSleepMicroseconds(50 + (rand() % 5000));

    // Send part 3 - uid[12-15]
    req[0] = PHI_CAN_AUTO_ID_OP_PART3;
    memcpy(&(req[1]), &(uid[12]), 4);
    req[5] = 0;
    req[6] = 0;
    crc = phi_crc16(req, 7);

    ret = phi_can_xfer(can, PHI_CAN_AUTO_ID_PRIO, crc & 0xFF, (crc >> 8) & 0x7F, req, sizeof(req), resp, sizeof(resp), &resp_len, PHI_CAN_DEFAULT_TIMEOUT);
    if (MSG_OK != ret)
    {
        goto lbl_ret;
    }

    if ((resp_len != 7) ||
        (0 != memcmp(req, resp, resp_len - 1)) ||
        (0 == resp[6]) ||
        (PHI_CAN_MAX_NODE_ID < resp[6]))
    {
        ret = MSG_RESET;
        goto lbl_ret;
    }

    // Done
    phi_can_set_node_id(can, resp[6]);
    ret = MSG_OK;

lbl_ret:

    return ret;
}

void phi_can_auto_get_dev_uid(phi_can_t * can, uint8_t * uid)
{
	memcpy(&(uid[0]), &(can->cfg->dev_id), sizeof(uint32_t));
	memcpy(&(uid[4]), (void *) STM32_REG_UNIQUE_ID, 12);
}


void phi_can_auto_handle_incoming_msg(phi_can_t * can, uint8_t prio, uint16_t msg_id, uint8_t dst, uint8_t chan_id, const uint8_t * data)
{
#if PHI_CAN_ENABLE_ALLOCATOR
    uint16_t crc = phi_crc16(data, 7);
    systime_t now = chVTGetSystemTimeX();
    bool timeout = (now - can->auto_last_at) > PHI_CAN_DEFAULT_TIMEOUT;
    int i;

    // Verify CRC
    if ((dst != ((crc >> 8) & 0x7f)) ||
        (msg_id != (crc & 0xff)))
    {
        return;
    }

    // Check if even have room for an allocation
    if (PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS == can->auto_alloc_num_devs)
    {
        return;
    }

    // Execute opcode
    switch (data[0])
    {
    case PHI_CAN_AUTO_ID_OP_PART1:
        if ((0 == can->auto_parts_received) || timeout)
        {
            memcpy(&(can->auto_alloc_cur_uid[0]), &(data[1]), 6);
            can->auto_parts_received = 1;
            can->auto_last_at = now;

            phi_can_send_reply(can, prio, PHI_CAN_MSG_ID_AUTO_ID_ACK, PHI_CAN_AUTO_ID, chan_id, data, 7, PHI_CAN_DEFAULT_TIMEOUT);
        }
        break;

    case PHI_CAN_AUTO_ID_OP_PART2:
        if ((1 == can->auto_parts_received) && !timeout)
        {
            memcpy(&(can->auto_alloc_cur_uid[6]), &(data[1]), 6);
            ++can->auto_parts_received;
            can->auto_last_at = now;

            phi_can_send_reply(can, prio, PHI_CAN_MSG_ID_AUTO_ID_ACK, PHI_CAN_AUTO_ID, chan_id, data, 7, PHI_CAN_DEFAULT_TIMEOUT);
        }

        break;

    case PHI_CAN_AUTO_ID_OP_PART3:
        if ((2 == can->auto_parts_received) && !timeout)
        {
            memcpy(&(can->auto_alloc_cur_uid[12]), &(data[1]), 4);

            // See if we can find this uid
            // Ugly hack, but we know we can write buf and the allocatee expects node id here
            ((uint8_t *)data)[6] = 0;
            for (i = 0; i < PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS; ++i)
            {
                if (0 == memcmp(can->auto_alloc_cur_uid, can->auto_alloc_table[i], PHI_CAN_AUTO_ID_UNIQ_ID_LEN))
                {
                    ((uint8_t *)data)[6] = PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + i;
                    break;
                }
            }

            if (data[6] == 0)
            {
                memcpy(&(can->auto_alloc_table[can->auto_alloc_num_devs]), can->auto_alloc_cur_uid, PHI_CAN_AUTO_ID_UNIQ_ID_LEN);
                ((uint8_t *)data)[6] = PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + can->auto_alloc_num_devs;
                ++can->auto_alloc_num_devs;
            }

            phi_can_send_reply(can, prio, PHI_CAN_MSG_ID_AUTO_ID_ACK, PHI_CAN_AUTO_ID, chan_id, data, 7, PHI_CAN_DEFAULT_TIMEOUT);
        }

        break;
    }

#else
    (void) can; (void) prio; (void) msg_id; (void) dst; (void) chan_id; (void) data;
#endif
}
