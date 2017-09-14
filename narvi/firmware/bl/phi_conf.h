#ifndef PHI_CONF_H_
#define PHI_CONF_H_

#include "phi_lib/phi_lib.h"


/******************************************************************************
 * NARVI Specific stuff
 *****************************************************************************/

#define NARVI_HW_SW_VER  PHI_HW_SW_VER(1, PHI_BL_HW_VER, PHI_BL_SW_VER)
#define NARVI_DEV_ID     PHI_BL_DEV_ID



/******************************************************************************
 * Bootloader
 *****************************************************************************/

// These two come from a phi_bl.mk compilation flag
#define NARVI_BL_USER_ADDR        PHI_BL_USER_ADDR
#define NARVI_BL_FLASH_PAGE_SIZE  PHI_BL_PAGE_SIZE

#endif /* PHI_CONF_H_ */
