#ifndef CENX4_APP_CFG_H_
#define CENX4_APP_CFG_H_

#include "phi_lib/phi_can.h"

#define CENX4_APP_CFG_MAGIC    PHI_DEV_ID('C', 'F', 'G', '1')
#define CENX4_APP_CFG_VER      1
#define CENX4_APP_CFG_ADDR     0x803f800 // 0x08000000 + 256k - 2k (must be aligned on a 2k page)

#define CENX4_APP_CFG_MAX_MODULES (PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1)

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
            uint8_t cenx4_mod_num_to_uid[CENX4_APP_CFG_MAX_MODULES][PHI_CAN_AUTO_ID_UNIQ_ID_LEN];
            uint8_t hyperion_mod_num_to_uid[CENX4_APP_CFG_MAX_MODULES][PHI_CAN_AUTO_ID_UNIQ_ID_LEN];

            // Ableton specific: Make the last encoder switch banks
            bool ableton_enable_banks_enc;

        } cur;
    };
} cenx4_app_cfg_t;


extern cenx4_app_cfg_t cenx4_app_cfg;

void cenx4_app_cfg_load(void);
void cenx4_app_cfg_init_default(cenx4_app_cfg_t * cfg);
void cenx4_app_cfg_save(cenx4_app_cfg_t * cfg);

/******************************************************************************
 * Module ID mapping stuff
 *****************************************************************************/

#define CENX4_APP_CFG_INVALID_MODULE_NUM 0xff

/* Gets a "mapped node id" from a node id number:
 * a mapped node id is a CAN node id mapped to fit inside an array.
 * Node id 0 (self) will always be 0, and auto-ids will follow.
 * PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID will be mapped node id 1, etc  */
static inline uint8_t cenx4_app_cfg_get_mapped_node_id(uint8_t node_id)
{
	return (node_id == 0) ? 0 : (node_id - PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + 1);
}

/*
 * Returns a map of "mapped node id" to module number.
 */
void cenx4_app_cfg_get_node_id_to_mod_num_map(uint32_t dev_id, uint8_t * map, uint32_t max_entries);


#endif /* CENX4_APP_CFG_H_ */
