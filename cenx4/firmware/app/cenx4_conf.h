#ifndef CENX4_CONF_H_
#define CENX4_CONF_H_

#include "phi_lib/phi_can.h"

/******************************************************************************
 * CENX4 Specific stuff
 *****************************************************************************/

#define CENX4_HW_SW_VER 0x0101
#define CENX4_DEV_ID     PHI_DEV_ID('C', 'N', 'X', '4')
#define CENX4_MAX_POTS   ((PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1) * 4) // 4 pots per module and 1 extra module for the master


#ifndef CENX4_IS_SLAVE
#define CENX4_IS_SLAVE 0
#endif

#endif /* CENX4_CONF_H_ */
