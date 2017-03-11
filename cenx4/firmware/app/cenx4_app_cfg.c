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

#if 0
// Given a CAN node_id, get the user assigned module number
uint8_t cenx4_app_cfg_get_module_number_by_node_id(uint8_t node_id)
{
    uint8_t i;
    uint8_t allocator_idx;

    chDbgCheck(node_id >= PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID);
    chDbgCheck(node_id < (PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID + PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS));

    allocator_idx = node_id - PHI_CAN_AUTO_ID_ALLOCATOR_FIRST_DEV_ID;
    if (allocator_idx >= cenx4_can.auto_alloc_num_devs)
    {
        // Node ID was never assigned
        return 0;
    }

    for (i = 0; i < PHI_CAN_AUTO_ID_ALLOCATOR_MAX_DEVS; ++i)
    {
        if (0 == memcmp(&(cenx4_can.auto_alloc_table[allocator_idx]),
                        &(cenx4_app_cfg.cur.module_numbers[i]),
                        PHI_CAN_AUTO_ID_UNIQ_ID_LEN))
        {
            return i + 1;
        }
    }

    return 0;
}
#endif
