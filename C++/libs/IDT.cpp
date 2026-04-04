#pragma once
#include "TypeDefs.cpp"
#include "drivers/IO.cpp"

struct IDT64 {
    uint_16 offset_low;
    uint_16 selector;
    uint_8 ist;
    uint_8 types_attr;
    uint_16 offset_mid;
    uint_32 offset_high;
    uint_32 zero;
};


extern "C" void LoadIDT();
extern IDT64 _idt[256];
extern uint_64 isr1;

void InitializeIDT(){
    _idt[1].zero = 0;
    _idt[1].offset_low = (uint_16)((uint_64)& isr1 & 0xFFFF);
    _idt[1].offset_mid = (uint_16)(((uint_64)& isr1 & 0xFFFF) >> 16);
    _idt[1].offset_high = (uint_16)(((uint_64)& isr1 & 0xFFFF) >> 32);
    _idt[1].ist = 0;
    _idt[1].selector = 0x08;
    _idt[1].selector = 0x8e;
    RemapPic();

    

    LoadIDT();
}

void isr1_handler(){

    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}