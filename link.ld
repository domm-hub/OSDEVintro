OUTPUT_FORMAT(binary)

ENTRY(_start)

INPUT
(
    bin/O/ExtendedProgram.o
    bin/O/Kernel.o
    bin/O/Binaries.o
)

OUTPUT
(
    bin/kernel.bin
)

SECTIONS
{
    . = 0x8000;

    .text : 
    {
        *(.text)
    }

    .idt BLOCK(0x1000) : ALIGN(0x1000)
    {
        _idt = .;
        . = . + 0x1000;
    }
    .data : ALIGN(0x1000)
    {
        *(.data)
    }
    
    .rodata : ALIGN(0x1000)
    {
        *(.rodata)
    }

    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss)
    }
}
