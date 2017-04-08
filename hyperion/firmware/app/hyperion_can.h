#ifndef HYPERION_CAN_H_
#define HYPERION_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "phi_lib/phi_can.h"


extern phi_can_t hyperion_can;

void hyperion_can_init(void);

#ifdef __cplusplus
}
#endif

#endif /* HYPERION_CAN_H_ */
