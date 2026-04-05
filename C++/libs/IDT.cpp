#pragma once
#include "TypeDefs.cpp"
#include "drivers/IO.cpp"
#include "drivers/TextPrint.cpp"
#include "Sets/KBSCodesS1.cpp"


struct IDT64 {
    uint_16 offset_low;
    uint_16 selector;
    uint_8 ist;
    uint_8 types_attr;
    uint_16 offset_mid;
    uint_32 offset_high;
    uint_32 zero;
};

extern IDT64 _idt[256];
extern uint_64 isr1;
extern "C" void LoadIDT();

void MakeIDTEntry(uint_64 handler, uint_16 index, uint_8 selector, uint_8 types_attr){
    uint_64 off1 = 0x000000000000FFFF;
    uint_64 off2 = 0x00000000FFFF0000;
    uint_64 off3 = 0xFFFFFFFF00000000;
    _idt[index].zero = 0;
    _idt[index].offset_low = (uint_16)(((uint_64) & handler & off1));
    _idt[index].offset_mid = (uint_16)(((uint_64) & handler & off2) >> 16);
    _idt[index].offset_high = (uint_32)(((uint_64) & handler & off3) >> 32);
    _idt[index].ist = 0;
    _idt[index].selector = selector;
    _idt[index].types_attr = types_attr;


}

void InitializeIDT(){
    MakeIDTEntry(isr1, 1, 0x08, 0x8e);

    outb(0x21, 0xfd);
    outb(0xA1, 0xff);
    LoadIDT();
    
}

extern "C" void isr1_handler(){
    uint_8 scanCode = inb(0x60);
    PrintChar(KBSet1::ScanCodeLookupTable[scanCode]),
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}
