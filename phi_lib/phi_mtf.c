#include "phi_lib/phi_mtf.h"

void phi_mtf_init_static(phi_mtf_t * mtf, size_t num_msgs, void * storage, size_t storage_size)
{
    phi_msgq_init_static((phi_msgq_t *) mtf, num_msgs, sizeof(phi_mtf_msg_t), storage, storage_size);
}

void phi_mtf_stop(phi_mtf_t * mtf)
{
    (void) mtf;
    // TODO wait for queue to drain?
    chDbgCheck(FALSE);
}

msg_t phi_mtf_async_call(phi_mtf_t * mtf, systime_t timeout, void * func, uint8_t n_args, ...)
{
    va_list ap;
    int i;
    msg_t ret;
    phi_mtf_msg_t * msg;

    va_start(ap, n_args);

    chDbgCheck(n_args <= PHI_MTF_MAX_ARGS);

    msg = phi_msgq_alloc((phi_msgq_t *) mtf, timeout);
    if (msg == NULL)
    {
        ret =  MSG_TIMEOUT;
        goto lbl_ret;
    }

    chBSemObjectInit(&(msg->done), TRUE); // TODO not used for waiting but is always signalled
    msg->func = func;
    msg->n_args = n_args;
    for (i = 0; i < n_args; ++i)
    {
        msg->args[i] = va_arg(ap, uint32_t);
    }

    phi_msgq_post((phi_msgq_t *) mtf, msg);
    ret = MSG_OK;

lbl_ret:
    va_end(ap);
    return ret;
}

msg_t phi_mtf_async_callI(phi_mtf_t * mtf, void * func, uint8_t n_args, ...)
{
    va_list ap;
    int i;
    msg_t ret;
    phi_mtf_msg_t * msg;

    va_start(ap, n_args);

    chDbgCheck(n_args <= PHI_MTF_MAX_ARGS);

    msg = phi_msgq_allocI((phi_msgq_t *) mtf);
    if (msg == NULL)
    {
        ret =  MSG_TIMEOUT;
        goto lbl_ret;
    }

    chBSemObjectInit(&(msg->done), TRUE); // TODO not used for waiting but is always signalled
    msg->func = func;
    msg->n_args = n_args;
    for (i = 0; i < n_args; ++i)
    {
        msg->args[i] = va_arg(ap, uint32_t);
    }

    phi_msgq_postI((phi_msgq_t *) mtf, msg);
    ret = MSG_OK;

lbl_ret:
    va_end(ap);
    return ret;
}


msg_t phi_mtf_call(phi_mtf_t * mtf, void * func, uint8_t n_args, ...)
{
    va_list ap;
    int i;
    msg_t ret;
    phi_mtf_msg_t * msg;

    va_start(ap, n_args);

    chDbgCheck(n_args <= PHI_MTF_MAX_ARGS);

    msg = phi_msgq_alloc((phi_msgq_t *) mtf, TIME_INFINITE);
    if (msg == NULL)
    {
        ret =  MSG_TIMEOUT;
        goto lbl_ret;
    }

    chBSemObjectInit(&(msg->done), TRUE);
    msg->func = func;
    msg->n_args = n_args;
    for (i = 0; i < n_args; ++i)
    {
        msg->args[i] = va_arg(ap, uint32_t);
    }

    phi_msgq_post((phi_msgq_t *) mtf, msg);

    chBSemWaitTimeout(&(msg->done), TIME_INFINITE);

    ret = MSG_OK;

lbl_ret:
    va_end(ap);
    return ret;
}

void phi_mtf_thread_process_msg(phi_mtf_t * mtf, systime_t timeout)
{
    phi_mtf_msg_t * msg;

    msg = (phi_mtf_msg_t *) phi_msgq_get((phi_msgq_t *) mtf, timeout);
    if (NULL == msg)
    {
        return;
    }

    switch (msg->n_args)
    {
    case 0:
        ((void (*)(void)) msg->func)();
        break;

    case 1:
        ((void (*)(uint32_t)) msg->func)(msg->args[0]);
        break;

    case 2:
        ((void (*)(uint32_t, uint32_t)) msg->func)(msg->args[0], msg->args[1]);
        break;

    case 3:
        ((void (*)(uint32_t, uint32_t, uint32_t)) msg->func)(msg->args[0], msg->args[1], msg->args[2]);
        break;

    case 4:
        ((void (*)(uint32_t, uint32_t, uint32_t, uint32_t)) msg->func)(msg->args[0], msg->args[1], msg->args[2], msg->args[3]);
        break;

    case 5:
        ((void (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)) msg->func)(msg->args[0], msg->args[1], msg->args[2], msg->args[3], msg->args[4]);
        break;

    default:
        chDbgCheck(FALSE);
        break;
    }

    chBSemSignal(&(msg->done));

    phi_msgq_free((phi_msgq_t *) mtf, msg);
}
