OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
    /* UEFI kernels are often loaded at higher addresses, 
       but for a flat binary loaded by your bootloader, 
       any safe address works. 0x100000 (1MB) is common. */
    . = 0x100000;

    .text : 
    {
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

    /* Keep the IDT section if you still use it in the kernel */
    .idt : ALIGN(0x1000)
    {
        _idt = .;
        . = . + 0x1000;
    }
}
