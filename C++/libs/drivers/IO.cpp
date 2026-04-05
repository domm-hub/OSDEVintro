#pragma once
#include "../TypeDefs.cpp"

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