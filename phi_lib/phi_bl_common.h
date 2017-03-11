#ifndef PHI_LIB_PHI_BL_COMMON_H_
#define PHI_LIB_PHI_BL_COMMON_H_

#include "phi_lib/phi_lib.h"

#define PHI_BL_DATA_PACKET_SIZE      128
#define PHI_BL_HDR_MAGIC             0xC0DE1337


typedef enum phi_bl_ret_e
{
    PHI_BL_RET_ERR_UNKNOWN = 0,
    PHI_BL_RET_OK,
    PHI_BL_RET_SIZE_MISMATCH,
    PHI_BL_RET_START_ADDR_MISMATCH,
    PHI_BL_RET_START_ADDR_MISALIGNED,
    PHI_BL_RET_IMG_SIZE_MISALIGNED,
    PHI_BL_RET_DEV_ID_MISMATCH,
    PHI_BL_RET_NOT_READY,
    PHI_BL_RET_OFFSET_MISMATCH,
    PHI_BL_RET_OFFSET_OVERFLOW,
    PHI_BL_RET_FLASH_ERASE_ERR,
    PHI_BL_RET_FLASH_WRITE_ERR,
    PHI_BL_RET_MISSING_DATA,
    PHI_BL_RET_BAD_CRC,
} phi_bl_ret_t;

typedef struct phi_bl_hdr_s
{
    uint32_t magic;
    uint32_t crc32;
    uint32_t img_size;
    uint32_t load_addr;
    uint8_t  data[0];
} phi_bl_hdr_t;

typedef struct phi_bl_state_s
{
    uint32_t img_size;
    uint32_t crc32;
    uint32_t cur_offset;
} phi_bl_state_t;

// The message that is sent to start a bootloading process (if it gets ACKed successfully)
typedef struct phi_bl_msg_start_s
{
    uint32_t img_size;
    uint32_t img_start_addr;
    uint32_t dev_id;
    uint32_t crc32;
    uint32_t reserved[4];
} phi_bl_msg_start_t;

// A packet with data
typedef struct phi_bl_msg_data_s
{
    uint32_t offset;
    uint8_t  buf[PHI_BL_DATA_PACKET_SIZE];
} phi_bl_msg_data_t;


#endif /* PHI_LIB_PHI_BL_COMMON_H_ */
