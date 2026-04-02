OUTPUT_FORMAT(binary)

ENTRY(_start)

INPUT
(
    O/ExtendedProgram.o
    O/Kernel.o
    O/Binaries.o
)

OUTPUT
(
    kernel.bin
)

SECTIONS
{
    . = 0x8000;

    .text : 
    {
        *(.text)
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
