#ifndef PHI_CONF_H_
#define PHI_CONF_H_

#include "phi_lib/phi_lib.h"


/******************************************************************************
 * HYPERION Specific stuff
 *****************************************************************************/

#define HYPERION_HW_SW_VER  PHI_HW_SW_VER(0, 1, 1)
#define HYPERION_DEV_ID     PHI_DEV_ID('H', 'Y', 'P', '1')



/******************************************************************************
 * Bootloader
 *****************************************************************************/

// These two come from a phi_bl.mk compilation flag
#define HYPERION_BL_USER_ADDR        PHI_BL_USER_ADDR
#define HYPERION_BL_FLASH_PAGE_SIZE  PHI_BL_PAGE_SIZE

#endif /* PHI_CONF_H_ */
