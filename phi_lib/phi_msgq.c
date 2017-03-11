#include "phi_lib/phi_msgq.h"

void phi_msgq_init_static(phi_msgq_t * msgq, size_t num_msgs, size_t msg_size, void * storage, size_t storage_size)
{
    size_t i;
    uint32_t msgs_base;

    chDbgCheck(storage_size == (num_msgs * (msg_size + sizeof(msg_t) + sizeof(msg_t))));
    chDbgCheck(phi_is_aligned(msg_size, 4));

    memset(msgq, 0, sizeof(*msgq));
    memset(storage, 0, sizeof(storage_size));

    msgq->num_msgs = num_msgs;
    msgq->msg_size = msg_size;

    msgq->free_msgs_q = (msg_t *) storage;
    msgq->used_msgs_q = (msg_t *) ( (uint8_t *)storage + (sizeof(msg_t) * num_msgs) );
    msgs_base = (uint32_t) ( (uint8_t *)msgq->used_msgs_q + (sizeof(msg_t) * num_msgs) );

    chMBObjectInit(&(msgq->free_msgs), (msg_t *) msgq->free_msgs_q, num_msgs);
    chMBObjectInit(&(msgq->used_msgs), (msg_t *) msgq->used_msgs_q, num_msgs);
    for (i = 0; i < num_msgs; ++i)
    {
        chMBPost(&(msgq->free_msgs), (msg_t)(msgs_base + (i * msg_size)), TIME_INFINITE);
    }
}

void * phi_msgq_alloc(phi_msgq_t * msgq, systime_t timeout)
{
    msg_t ret;
    void * msg;

    ret = chMBFetch(&(msgq->free_msgs), (msg_t *) &msg, timeout);
    if (ret != MSG_OK)
    {
        return NULL;
    }

    return msg;
}

void * phi_msgq_allocI(phi_msgq_t * msgq)
{
    msg_t ret;
    void * msg;

    ret = chMBFetchI(&(msgq->free_msgs), (msg_t *) &msg);
    if (ret != MSG_OK)
    {
        return NULL;
    }

    return msg;
}

void phi_msgq_free(phi_msgq_t * msgq, void * msg)
{
    msg_t ret;
    ret = chMBPost(&(msgq->free_msgs), (msg_t) msg, TIME_INFINITE);
    chDbgAssert(ret == MSG_OK, "WTF");
}

void phi_msgq_freeI(phi_msgq_t * msgq, void * msg)
{
    msg_t ret;
    ret = chMBPostI(&(msgq->free_msgs), (msg_t) msg);
    chDbgAssert(ret == MSG_OK, "WTF");
}

void phi_msgq_post(phi_msgq_t * msgq, void * msg)
{
    msg_t ret;
    ret = chMBPost(&(msgq->used_msgs), (msg_t) msg, TIME_INFINITE);
    chDbgAssert(ret == MSG_OK, "WTF");
}

void phi_msgq_postI(phi_msgq_t * msgq, void * msg)
{
    msg_t ret;
    ret = chMBPostI(&(msgq->used_msgs), (msg_t) msg);
    chDbgAssert(ret == MSG_OK, "WTF");
}

void * phi_msgq_get(phi_msgq_t * msgq, systime_t timeout)
{
    msg_t ret;
    void * msg;

    ret = chMBFetch(&(msgq->used_msgs), (msg_t *) &msg, timeout);
    if (ret != MSG_OK)
    {
        return NULL;
    }
    return msg;
}
