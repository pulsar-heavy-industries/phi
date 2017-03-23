ENTRY(_start);

SECTIONS
{
    _start = BL_FLASH_START;
    . = BL_FLASH_START + BL_RESERVED_SIZE;
    .data : {
        *(.data)
    }
}
