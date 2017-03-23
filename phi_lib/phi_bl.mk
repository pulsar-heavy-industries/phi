
# https://stackoverflow.com/questions/10858261/abort-makefile-if-variable-not-set
bl_check_defined = \
    $(strip $(foreach 1,$1, \
        $(call __bl_check_defined,$1,$(strip $(value 2)))))
__bl_check_defined = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $2, ($2))))

# Magic word to identify a valid bootloader image
# Example: BL_MAGIC = 0xC0D31337
$(call bl_check_defined, BL_MAGIC)

# Flash Page size
# Example: BL_PAGE_SIZE = 2048
$(call bl_check_defined, BL_PAGE_SIZE)

# Where does the flash start?
# Example: BL_FLASH_START = 0x08000000
$(call bl_check_defined, BL_FLASH_START)

# How many bytes are reserved for the bootloader
# (Must be on BL_PAGE_SIZE boundry!)
# Example: BL_RESERVED_SIZE = 65536
$(call bl_check_defined, BL_RESERVED_SIZE)

# Where does user code actually starts?
# User firmware goes at FLASH_START + BL_RESERVED_SIZE but due to
# vector table alignment constraints the code can't always be placed
# right there. BL_START_OFFSET offsets it from FLASH_START + BL_RESERVED_SIZE
# so the alignment constraints can be met
# Example: BL_START_OFFSET = 0x200
$(call bl_check_defined, BL_START_OFFSET)


# Expose variables to linker script
BL_LDOPT = --defsym=BL_FLASH_START=$(BL_FLASH_START)
BL_LDOPT := $(BL_LDOPT),--defsym=BL_RESERVED_SIZE=$(BL_RESERVED_SIZE)
BL_LDOPT := $(BL_LDOPT),--defsym=BL_START_OFFSET=$(BL_START_OFFSET)

ifneq ($(LDOPT),)
	LDOPT := $(LDOPT),$(BL_LDOPT)
else
	LDOPT = $(BL_LDOPT)
endif

# Expose variables to C code
DEFS += -DPHI_BL_HDR_MAGIC=$(BL_MAGIC) -DPHI_BL_USER_ADDR="($(BL_FLASH_START) + $(BL_RESERVED_SIZE))" -DPHI_BL_PAGE_SIZE=$(BL_PAGE_SIZE)

# Create the binary firmware image - this is the file that can get downloaded
# using the bootloader mechanism
$(BUILDDIR)/bl-$(PROJECT).bin: $(BUILDDIR)/$(PROJECT).bin $(PHI_LIB)/phi_bl.mk
	python $(PHI_LIB)/phi_bl_make_img.py \
		--magic $(BL_MAGIC) \
		--page-size $(BL_PAGE_SIZE) \
		--start-offset $(BL_START_OFFSET) \
		$< $@

# Create an ELF file out of the binary file so that GDB can write it to the target.
# This makes debugging easier (but keep in mind it does not have the symbols in it,
# so while it can be loaded to the target symbols should we loaded from $(PROJECT).elf
$(BUILDDIR)/bl-$(PROJECT).elf: $(BUILDDIR)/bl-$(PROJECT).bin $(PHI_LIB)/phi_bl.mk $(PHI_LIB)/phi_bl_glue.ld
	$(TRGT)objcopy -I binary -O elf32-littlearm --binary-architecture arm $< $(OBJDIR)/bl-$(PROJECT).o
	$(TRGT)ld --defsym=BL_FLASH_START=$(BL_FLASH_START) --defsym=BL_RESERVED_SIZE=$(BL_RESERVED_SIZE) $(OBJDIR)/bl-$(PROJECT).o -T $(PHI_LIB)/phi_bl_glue.ld -o $@
