#pragma once
#include "TypeDefs.cpp"

struct IDT64 {
    uint_16 offset_low;
    uint_16 selector;
    uint_8 ist;
    uint_8 types_attr;
    uint_16 offset_mid;
    uint_32 offset_hugh;
    uint_32 zero;
};


extern "C" void LoadIDT();
extern IDT64 _idt[256];
extern uint_64 isr1;

void InitializeIDT(){
    
    LoadIDT();
}

void isr1_handler(){
    outb(0x20, 0x20);
    outb(0xA0, 0x20);

}