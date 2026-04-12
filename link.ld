OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
    . = 0x100000;

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

    .data : ALIGN(0x1000)
    {
        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    .idt : ALIGN(0x1000)
    {
        _idt = .;
        . = . + 0x1000;
    }
}
