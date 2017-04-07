#ifndef PHI_LIB_PHI_MTF_H_
#define PHI_LIB_PHI_MTF_H_

#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_msgq.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PHI_MTF_MAX_ARGS 7

typedef struct
{
    void * func;
    uint32_t args[PHI_MTF_MAX_ARGS];
    uint8_t n_args;
    binary_semaphore_t done;
} phi_mtf_msg_t;

#define phi_mtf_static_storage_t(name, num_msgs) phi_msgq_static_storage_t(name, num_msgs, sizeof(phi_mtf_msg_t))
typedef phi_msgq_t phi_mtf_t;

void phi_mtf_init_static(phi_mtf_t * mtf, size_t num_msgs, void * storage, size_t storage_size);
void phi_mtf_stop(phi_mtf_t * mtf);
msg_t phi_mtf_async_call(phi_mtf_t * mtf, systime_t timeout, void * func, uint8_t n_args, ...);
msg_t phi_mtf_async_callI(phi_mtf_t * mtf, void * func, uint8_t n_args, ...);
msg_t phi_mtf_call(phi_mtf_t * mtf, void * func, uint8_t n_args, ...);
void phi_mtf_thread_process_msg(phi_mtf_t * mtf, systime_t timeout);


#ifdef __cplusplus
}
#endif

#endif /* PHI_LIB_PHI_MTF_H_ */
