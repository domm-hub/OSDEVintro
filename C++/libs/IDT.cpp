#pragma once
#include "TypeDefs.cpp"
#include "drivers/IO.cpp"
#include "drivers/TextPrint.cpp"
#include "Sets/KBSCodesS1.cpp"

#define KeyBoardInput 0x60

struct IDT64 {
    uint_16 offset_low;
    uint_16 selector;
    uint_8 ist;
    uint_8 types_attr;
    uint_16 offset_mid;
    uint_32 offset_high;
    uint_32 zero;
} __attribute__((packed));

extern IDT64 _idt[256];
extern "C" void LoadIDT();
extern "C" uint_64 isr1();

void (*MainKeyboardHandler)(uint_8 scanCode, uint_8 chr);
extern bool LeftShiftPressed;
extern bool RightShiftPressed;

void InitializeIDT(){
    RemapPic(); // 1. Move IRQs to 32+

    // 2. Get the 64-bit address of your ASM stub
    uint_64 handler = (uint_64)isr1; 

    // 3. Fill Entry 33 (Keyboard) - IRQ1 is 32 + 1 = 33
    _idt[33].offset_low  = (uint_16)(handler & 0xFFFF);
    _idt[33].offset_mid  = (uint_16)((handler >> 16) & 0xFFFF);
    _idt[33].offset_high = (uint_32)((handler >> 32) & 0xFFFFFFFF);
    _idt[33].selector    = 0x08;   // Your Kernel Code Segment
    _idt[33].types_attr  = 0x8e;   // Present, Ring 0, Interrupt Gate
    _idt[33].ist         = 0;
    _idt[33].zero        = 0;

    // 4. Unmask the Keyboard (IRQ 1) on the PIC
    // 0xFD is 11111101 (Enables only Bit 1 - Keyboard)
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF); // Mask all of Slave PIC

    LoadIDT(); // 5. lidt and sti
}

extern "C" void isr1_handler(){
    uint_8 scanCode = inb(KeyBoardInput);
    uint_8 chr = KBSet1::ScanCodeToChar(scanCode, LeftShiftPressed | RightShiftPressed);
    
    if (MainKeyboardHandler != 0){
        MainKeyboardHandler(scanCode, chr);
    }
    
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}
