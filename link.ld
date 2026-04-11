OUTPUT_FORMAT(binary)

ENTRY(_start)

INPUT
(
    bin/O/ExtendedProgram.o
    bin/O/Kernel.o
    bin/O/IDT.o
    bin/O/IO.o
    bin/O/Keyboard.o
    bin/O/MemoryMap.o
    bin/O/TextPrint.o
    bin/O/Heap.o
    bin/O/Binaries.o
    bin/O/Memory.o
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
        *(.text.*)
        *(.rodata)      /* Moved strings up here! */
        *(.rodata.*)    /* Added the SSE wildcard! */
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
    
    /* Deleted the old .rodata section from here entirely */

    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss)
    }
}