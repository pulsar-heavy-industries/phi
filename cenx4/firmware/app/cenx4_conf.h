#ifndef CENX4_CONF_H_
#define CENX4_CONF_H_

#include "phi_lib/phi_can.h"

/******************************************************************************
 * CENX4 Specific stuff
 *****************************************************************************/

#define CENX4_HW_SW_VER  PHI_HW_SW_VER(0, 1, 1)
#define CENX4_DEV_ID     PHI_DEV_ID('C', 'N', 'X', '4')
#define CENX4_MAX_POTS   ((PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1) * 4) // 4 pots per module and 1 extra module for the master

/* Are we a master or a slave? */
extern bool cenx4_is_master;


#endif /* CENX4_CONF_H_ */
