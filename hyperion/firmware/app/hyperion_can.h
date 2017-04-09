#ifndef HYPERION_CAN_H_
#define HYPERION_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "phi_lib/phi_can.h"


extern phi_can_t hyperion_can;

void hyperion_can_init(void);

void hyperion_can_handle_unknown_cmd(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);


#ifdef __cplusplus
}
#endif

#endif /* HYPERION_CAN_H_ */
