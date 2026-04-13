#include "../../headers/TypeDefs.h"
#include "IO.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21

#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT      0x10  
#define ICW1_ICW4      0x01  
#define ICW4_8086      0x01  

// End of Interrupt (EOI) command
#define PIC_EOI        0x20

void outb(uint_16 port, uint_8 val){
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint_8 inb(uint_16 port){
    uint_8 returnVal;
    asm volatile ("inb %1, %0" 
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

void outl(uint_16 port, uint_32 val){
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

uint_32 inl(uint_16 port){
    uint_32 returnVal;
    asm volatile ("inl %1, %0" 
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

void wrmsr(uint_32 msr, uint_64 value) {
    uint_32 low = (uint_32)value;
    uint_32 high = (uint_32)(value >> 32);
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

uint_64 rdmsr(uint_32 msr) {
    uint_32 low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint_64)high << 32) | low;
}

void activate_sse() {
    __asm__ __volatile__ (
        "mov %%cr0, %%rax\n\t"
        "and $0xFFFB, %%ax\n\t"  /* Clear CR0.EM (Bit 2): Disable FPU emulation */
        "or $0x2, %%ax\n\t"      /* Set CR0.MP (Bit 1): Enable monitor coprocessor */
        "mov %%rax, %%cr0\n\t"
        "mov %%cr4, %%rax\n\t"
        "or $0x600, %%rax\n\t"   /* Set CR4.OSFXSR (Bit 9) and CR4.OSXMMEXCPT (Bit 10) */
        "mov %%rax, %%cr4\n\t"
        :
        :
        : "rax", "cc"
    );
}

void RemapPic(){
    uint_8 a1, a2;

    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW4_8086);
    outb(PIC2_COMMAND, ICW1_INIT | ICW4_8086);

    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}