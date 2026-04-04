#pragma once
#include "../TypeDefs.cpp"

// Master PIC
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21

// Slave PIC
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// Initialization Control Words (ICW)
#define ICW1_INIT      0x11  // Initialize the PIC
#define ICW1_ICW4      0x01  // ICW4 (8086/88 mode) will be present
#define ICW4_8086      0x01  // 8086/88 (MCS-80/85) mode

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

void RemapPic() {

    uint_8 a1 = inb(PIC1_DATA);
    uint_8 a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 8);
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a1);

}

