#include "TypeDefs.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21

#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// ICW flags
#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086     0x01

// End of interrupt
#define PIC_EOI      0x20



 void outb(uint_16 port, uint_8 val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

 uint_8 inb(uint_16 port) {
    uint_8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

 void outl(uint_16 port, uint_32 val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

 uint_32 inl(uint_16 port) {
    uint_32 ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// MSR
void wrmsr(uint_32 msr, uint_64 value) {
    uint_32 low = (uint_32)value;
    uint_32 high = (uint_32)(value >> 32);
    asm volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

uint_64 rdmsr(uint_32 msr) {
    uint_32 low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint_64)high << 32) | low;
}

// SSE enable
void activate_sse() {
    asm volatile (
        "mov %%cr0, %%rax\n\t"
        "and $0xFFFB, %%ax\n\t"
        "or $0x2, %%ax\n\t"
        "mov %%rax, %%cr0\n\t"

        "mov %%cr4, %%rax\n\t"
        "or $0x600, %%rax\n\t"
        "mov %%rax, %%cr4\n\t"
        :
        :
        : "rax", "cc"
    );
}

// Send EOI
void PIC_SendEOI(uint_8 irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

// =====================
// PROPER PIC REMAP
// =====================
void RemapPic() {
    uint_8 mask1 = inb(PIC1_DATA);
    uint_8 mask2 = inb(PIC2_DATA);

    // Start initialization
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // Vector offsets
    outb(PIC1_DATA, 0x20); // IRQ0-7 -> 32-39
    outb(PIC2_DATA, 0x28); // IRQ8-15 -> 40-47

    // Wiring
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    // 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Restore masks (IMPORTANT: after init)
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}