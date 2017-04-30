#ifndef PHI_LIB_PHI_AT45_H_
#define PHI_LIB_PHI_AT45_H_

#include "phi_lib/phi_lib.h"

#define PHI_AT45_PAGE_SIZE 512

typedef enum {
	PHI_AT45_RET_WTF = 0,
	PHI_AT45_RET_SUCCESS,
	PHI_AT45_RET_TIMEOUT,
	PHI_AT45_RET_NOT_512,
	PHI_AT45_RET_UNEXPECTED_DENSITY,
	PHI_AT45_RET_UNKNOWN_DEVICE,
	PHI_AT45_RET_EPE,
} phi_at45_ret_t;

typedef struct phi_at45_cfg_s
{
	SPIDriver * spi;
	SPIConfig spi_cfg;
} phi_at45_cfg_t;

typedef struct phi_at45_desc_s
{
	/* device id */
	uint16_t dev_id;

	/* device name */
	const char * name;

	/* number of pages in device */
	uint16_t num_pages;

} phi_at45_desc_t;

typedef struct phi_at45_s
{
	const phi_at45_cfg_t * cfg;
	const phi_at45_desc_t * desc;
} phi_at45_t;


phi_at45_ret_t phi_at45_init(phi_at45_t * at45, const phi_at45_cfg_t * cfg);
phi_at45_ret_t phi_at45_erase_chip(phi_at45_t * at45);
phi_at45_ret_t phi_at45_page_write(phi_at45_t * at45, uint32_t addr, const void * buf, size_t len);
void phi_at45_read(phi_at45_t * at45, uint32_t addr, void * buf, size_t len);

/* Internal methods */
phi_at45_ret_t phi_at45_wait_for_rdy(phi_at45_t * at45, uint16_t * out_last_status);

#endif
