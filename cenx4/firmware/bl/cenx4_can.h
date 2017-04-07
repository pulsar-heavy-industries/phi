#ifndef CENX4_CAN_H_
#define CENX4_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "phi_lib/phi_can.h"
#include "cenx4_ui.h"

extern phi_can_t cenx4_can;

void cenx4_can_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CENX4_CAN_H_ */
