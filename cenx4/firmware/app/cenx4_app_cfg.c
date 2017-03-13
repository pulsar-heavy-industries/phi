#include "phi_lib/phi_lib.h"
#include "phi_lib/st/stm32f3xx_hal_flash.h"
#include "phi_lib/st/stm32f3xx_hal_flash_ex.h"

#include "cenx4_app_cfg.h"
#include "cenx4_can.h"

cenx4_app_cfg_t cenx4_app_cfg;

void cenx4_app_cfg_load(void)
{
	cenx4_app_cfg_t cfg;
	uint32_t crc;

	cenx4_app_cfg_init_default(&cenx4_app_cfg);

    memcpy(&cfg, (void *)CENX4_APP_CFG_ADDR, sizeof(cfg));

    crc = cfg.crc;
    cfg.crc = 0;
    if ((cfg.magic != CENX4_APP_CFG_MAGIC) ||
    	(cfg.ver != CENX4_APP_CFG_VER) ||
		(crc != phi_crc32(&cfg, sizeof(cfg))))
    {
    	return;
    }

    memcpy(&cenx4_app_cfg, &cfg, sizeof(cfg));

}

void cenx4_app_cfg_init_default(cenx4_app_cfg_t * cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->magic = CENX4_APP_CFG_MAGIC;
    cfg->ver = CENX4_APP_CFG_VER;
}

void cenx4_app_cfg_save(cenx4_app_cfg_t * cfg)
{
	FLASH_EraseInitTypeDef erase;
	HAL_StatusTypeDef status;
	uint32_t err, i;

	chDbgCheck(sizeof(*cfg) < 2048);

	cfg->crc = 0;
	cfg->crc = phi_crc32(cfg, sizeof(*cfg));

	chSysDisable();

	status = HAL_FLASH_Unlock();
	chDbgCheck(status == HAL_OK);

	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	erase.PageAddress = CENX4_APP_CFG_ADDR;
	erase.NbPages = 1;

	status = HAL_FLASHEx_Erase(&erase, &err);
	chDbgCheck(status == HAL_OK);

	for (i = 0; i < sizeof(*cfg); i += 8)
	{
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, CENX4_APP_CFG_ADDR + i, *(uint64_t *)((uint32_t)cfg + i));
		chDbgCheck(status == HAL_OK);
	}

	status = HAL_FLASH_Lock();
	chDbgCheck(status == HAL_OK);

	memcpy(&cenx4_app_cfg, cfg, sizeof(*cfg));

	chSysEnable();
}

void cenx4_app_cfg_get_node_id_to_mod_num_map(uint8_t * map, uint32_t max_entries)
{
    uint8_t our_uid[PHI_CAN_AUTO_ID_UNIQ_ID_LEN];
    uint8_t node_id;
    uint8_t mod_num;

	chDbgCheck(max_entries >= PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1);

	memset(map, CENX4_APP_CFG_INVALID_MODULE_NUM, max_entries);

	phi_can_auto_get_dev_uid(&cenx4_can, our_uid);
	for (mod_num = 0; mod_num < PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS + 1; ++mod_num)
	{
		// If stored UID matches ours, set node id 0
		if (memcmp(
			&(cenx4_app_cfg.cur.mod_num_to_uid[mod_num][0]),
			&(our_uid[0]),
			PHI_CAN_AUTO_ID_UNIQ_ID_LEN) == 0)
		{
			map[0] = mod_num;
			continue;
		}

		// Get the node id of the current module
		for (node_id = 0; node_id < cenx4_can.auto_alloc_num_devs; ++node_id)
		{
			if (memcmp(
				&(cenx4_app_cfg.cur.mod_num_to_uid[mod_num][0]),
				&(cenx4_can.auto_alloc_table[node_id][0]),
				PHI_CAN_AUTO_ID_UNIQ_ID_LEN) == 0)
			{
				map[node_id + 1] = mod_num;
				break;
			}
		}
	}
}
