#ifndef PHI_LIB_PHI_BL_COMMON_H_
#define PHI_LIB_PHI_BL_COMMON_H_

#include "phi_lib/phi_lib.h"

#define PHI_BL_DATA_PACKET_SIZE      128
#define PHI_BL_HDR_MAGIC			 0xC0DE1337

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
	PHI_BL_RET_BAD_MAGIC,
	PHI_BL_RET_AT45_ERROR,
	PHI_BL_RET_NOT_FOUND,
} phi_bl_ret_t;

#pragma pack(1)

#define PHI_BL_HDR_DATA_SIZE ((uint32_t) &(((phi_bl_hdr_t *) 0)->fw_data) - (uint32_t) &(((phi_bl_hdr_t *) 0)->hdr_data[0]))

typedef struct phi_bl_hdr_s
{
	// Needs to be equal to PHI_BL_HDR_MAGIC
    uint32_t magic;

    // CRC of all the fields that follow, excluding the data
    uint32_t hdr_data_crc32;

    union
	{
    	uint8_t hdr_data[0];
    	struct
		{
			// Device ID this image is for
			uint32_t dev_id;

			// Software version this image holds
			uint8_t  sw_ver;

			// Hardware version + mask this image can run on
			uint8_t  hw_ver;
			uint8_t  hw_ver_mask;

			// Address this image should be written to
			uint32_t write_addr;

			// Address to jump to when starting
			uint32_t start_addr;

			// Size of fw data
			uint32_t fw_data_size;

			// CRC of the fw data that follows
			uint32_t fw_data_crc32;
		};
	};

    // Firmware data
    uint8_t fw_data[0];
} phi_bl_hdr_t;

typedef struct phi_bl_state_s
{
    uint32_t img_size;
    uint32_t crc32;
    uint32_t cur_offset;
} phi_bl_state_t;

// The message that is sent to start a boot-loading process (if it gets ACKed successfully)
typedef struct phi_bl_msg_start_s
{
	// Size of image about to be transferred
	uint32_t img_size;

	// CRC32 of image about to be transferred
	uint32_t img_crc32;

	// Bootloader header of image being transferred
	phi_bl_hdr_t bl_hdr;

} phi_bl_msg_start_t;

// A packet with data
typedef struct phi_bl_msg_data_s
{
    uint32_t offset;
    uint8_t  buf[PHI_BL_DATA_PACKET_SIZE];
} phi_bl_msg_data_t;

#pragma pack()

#endif /* PHI_LIB_PHI_BL_COMMON_H_ */
