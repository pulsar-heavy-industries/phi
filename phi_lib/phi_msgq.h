#ifndef PHI_LIB_PHI_MSGQ_H_
#define PHI_LIB_PHI_MSGQ_H_

#include "phi_lib/phi_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define phi_msgq_static_storage_t(name, num_msgs, msg_size) \
    msg_t name[num_msgs * (2 + (msg_size / sizeof(msg_t)))]

typedef struct
{
    size_t num_msgs;
    size_t msg_size;

    msg_t * free_msgs_q;
    mailbox_t free_msgs;
    msg_t * used_msgs_q;
    mailbox_t used_msgs;
} phi_msgq_t;

void phi_msgq_init_static(phi_msgq_t * msgq, size_t num_msgs, size_t msg_size, void * storage, size_t storage_size);
void * phi_msgq_alloc(phi_msgq_t * msgq, systime_t timeout);
void * phi_msgq_allocI(phi_msgq_t * msgq);
void phi_msgq_free(phi_msgq_t * msgq, void * msg);
void phi_msgq_freeI(phi_msgq_t * msgq, void * msg);
void phi_msgq_post(phi_msgq_t * msgq, void * msg);
void phi_msgq_postI(phi_msgq_t * msgq, void * msg);
void * phi_msgq_get(phi_msgq_t * msgq, systime_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* PHI_LIB_PHI_MSGQ_H_ */
