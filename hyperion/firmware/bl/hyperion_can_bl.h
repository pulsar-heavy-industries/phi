#ifndef HYPERION_CAN_BL_H_
#define HYPERION_CAN_BL_H_

#include "phi_lib/phi_can.h"
#include "phi_lib/phi_bl_common.h"

#define HYPERION_CAN_BL_MSG_HANDLERS \
    {PHI_CAN_MSG_BL_START, hyperion_can_bl_handle_start, NULL}, \
    {PHI_CAN_MSG_BL_DATA, hyperion_can_bl_handle_data, NULL}, \
    {PHI_CAN_MSG_BL_DONE, hyperion_can_bl_handle_done, NULL}

void hyperion_can_bl_handle_start(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void hyperion_can_bl_handle_data(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);
void hyperion_can_bl_handle_done(phi_can_t * can, void * context, uint8_t prio, uint8_t msg_id, uint8_t src, uint8_t chan_id, const uint8_t * data, size_t len);


#endif /* HYPERION_CAN_BL_H_ */
