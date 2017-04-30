#ifndef PHI_CONF_H_
#define PHI_CONF_H_

#include "phi_lib/phi_lib.h"


/******************************************************************************
 * HYPERION Specific stuff
 *****************************************************************************/

#define HYPERION_HW_SW_VER  PHI_HW_SW_VER(1, 1, 1)
#define HYPERION_DEV_ID     PHI_BL_DEV_ID


#ifndef HYPERION_IS_SLAVE
#define HYPERION_IS_SLAVE 0
#endif


/******************************************************************************
 * Bootloader
 *****************************************************************************/

// These two come from a phi_bl.mk compilation flag
#define HYPERION_BL_USER_ADDR        PHI_BL_USER_ADDR
#define HYPERION_BL_FLASH_PAGE_SIZE  PHI_BL_PAGE_SIZE

#endif /* PHI_CONF_H_ */
