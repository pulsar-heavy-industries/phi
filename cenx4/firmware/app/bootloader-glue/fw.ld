ENTRY(_start);

SECTIONS
{
    _start = 0x08000000;
    . = 0x08010000;
    .data : {
        *(.data)
    }
}
