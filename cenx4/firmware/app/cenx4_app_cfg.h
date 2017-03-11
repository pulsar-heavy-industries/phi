#ifndef CENX4_APP_CFG_H_
#define CENX4_APP_CFG_H_

#include "phi_lib/phi_can.h"

#define CENX4_APP_CFG_MAGIC    PHI_DEV_ID('C', 'F', 'G', '1')
#define CENX4_APP_CFG_VER      1
#define CENX4_APP_CFG_ADDR     0x803f800 // 0x08000000 + 256k - 2k (must be aligned on a 2k page)

typedef struct cenx4_app_cfg_s
{
    uint32_t magic;
    uint32_t crc;
    uint8_t  ver;

    union
    {
    	// Current version
        struct
        {
            // A map of module numbers to their UIDs
            // The module number is derived from the index in the array.
            // Unused entries have all 0s
            uint8_t mod_num_to_uid[PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1][PHI_CAN_AUTO_ID_UNIQ_ID_LEN];
        } cur;
    };
} cenx4_app_cfg_t;


extern cenx4_app_cfg_t cenx4_app_cfg;

void cenx4_app_cfg_load(void);
void cenx4_app_cfg_init_default(cenx4_app_cfg_t * cfg);
void cenx4_app_cfg_save(cenx4_app_cfg_t * cfg);
//uint8_t cenx4_app_cfg_get_module_number_by_node_id(uint8_t node_id);

#endif /* CENX4_APP_CFG_H_ */
