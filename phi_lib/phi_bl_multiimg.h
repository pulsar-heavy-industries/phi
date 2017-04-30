#ifndef PHI_LIB_PHI_BL_MULTIIMG_H_
#define PHI_LIB_PHI_BL_MULTIIMG_H_

#include "phi_lib/phi_bl_common.h"
#include "phi_lib/phi_at45.h"


#define PHI_BL_MULTIIMG_HDR_MAGIC  0x4d4c5431 // MLT1

#pragma pack (1)

typedef struct phi_bl_multiimg_hdr_s
{
	uint32_t magic;
	uint32_t payload_crc32; // CRC of bl_hdrs and all the data that follows
	uint32_t payload_size; // combined size of all images
	uint8_t  num_bl_hdrs;
} phi_bl_multiimg_hdr_t;

// The message that is sent to start a boot-loading process (if it gets ACKed successfully)
typedef struct phi_bl_multiimg_msg_start_s
{
	// Size of image about to be transferred
	uint32_t img_size;

	// CRC32 of image about to be transferred
	uint32_t img_crc32;

} phi_bl_multiimg_msg_start_t;

#pragma pack()

phi_bl_ret_t phi_bl_multiimg_find_bl_hdr(
	phi_at45_t * at45,
	uint32_t dev_id,
	uint32_t hw_sw_ver,
	uint32_t * out_offset,
	phi_bl_hdr_t * out_bl_hdr
);

#endif /* PHI_LIB_PHI_BL_MULTIIMG_H_ */
