#ifndef CENX4_CAN_H_
#define CENX4_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "phi_lib/phi_can.h"
#include "cenx4_ui.h"

extern phi_can_t cenx4_can;

void cenx4_can_init(void);

void cenx4_can_handle_reset(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CENX4_CAN_H_ */
