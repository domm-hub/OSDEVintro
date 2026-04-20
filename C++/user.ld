OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
    . = 0x800000;

    .text : 
    {
        *(.entry)
        *(.text)
        *(.text.*)
    }

    .rodata : 
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data : 
    {
        *(.data)
        *(.data.*)
    }

    .bss : 
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
}
