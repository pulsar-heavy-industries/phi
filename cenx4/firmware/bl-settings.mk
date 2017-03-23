###############################################################################
# This file holds common bootloader settings used by both the app and
# bootloader projects
###############################################################################

# See phi_bl.mk for an explanation of these variables
BL_MAGIC = 0xC0DE1337
BL_PAGE_SIZE = 2048
BL_FLASH_START = 0x8000000
BL_RESERVED_SIZE = 65536
BL_START_OFFSET = 0x200
