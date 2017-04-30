#include "phi_lib/phi_bl_multiimg.h"

phi_bl_ret_t phi_bl_multiimg_find_bl_hdr(
	phi_at45_t * at45,
	uint32_t dev_id,
	uint32_t hw_sw_ver,
	uint32_t * out_offset,
	phi_bl_hdr_t * out_bl_hdr
)
{
	const uint8_t hw_ver = PHI_HW_SW_VER_GET_HW(hw_sw_ver);
	phi_bl_multiimg_hdr_t multiimg_hdr;
	uint32_t offset;
	int i;

	/* Read header */
	phi_at45_read(at45, 0, &multiimg_hdr, sizeof(multiimg_hdr));

	/* Sanity checks */
	chDbgCheck(NULL != out_offset);
	chDbgCheck(NULL != out_bl_hdr);

	if (multiimg_hdr.magic != PHI_BL_MULTIIMG_HDR_MAGIC)
	{
		return PHI_BL_RET_BAD_MAGIC;
	}

	offset = sizeof(multiimg_hdr);
	for (i = 0; i < multiimg_hdr.num_bl_hdrs; ++i)
	{
		/* Read header at current offset */
		phi_at45_read(at45, offset, out_bl_hdr, sizeof(*out_bl_hdr));

		/* Verify magic */
		if (out_bl_hdr->magic != PHI_BL_HDR_MAGIC)
		{
			return PHI_BL_RET_BAD_MAGIC;
		}

		/* Verify header CRC */
	    if (out_bl_hdr->hdr_data_crc32 != phi_crc32(&(out_bl_hdr->hdr_data[0]), PHI_BL_HDR_DATA_SIZE))
	    {
			return PHI_BL_RET_BAD_MAGIC;
		}

	    /* Check if this header matches the request device/hwsw */
	    if ((out_bl_hdr->dev_id == dev_id) &&
			((hw_ver & out_bl_hdr->hw_ver_mask) == out_bl_hdr->hw_ver))
		{
	    	*out_offset = offset;
	    	return PHI_BL_RET_OK;
		}

	    /* Move to the next image */
	    offset += sizeof(*out_bl_hdr) + out_bl_hdr->fw_data_size;
	}

	/* Not found */
	return PHI_BL_RET_NOT_FOUND;
}
