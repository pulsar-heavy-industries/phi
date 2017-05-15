###############################################################################
# This file holds common bootloader settings used by both the app and
# bootloader projects
###############################################################################

# See phi_bl.mk for an explanation of these variables
BL_DEV_ID = 0x4e525048 # "HPRN"
BL_SW_VER = 3
BL_HW_VER = 1
BL_HW_VER_MASK = 0x7f

BL_PAGE_SIZE = 131072
BL_FLASH_START = 0x8000000
BL_RESERVED_SIZE = 131072
BL_VECTORS_OFFSET = 0x200
