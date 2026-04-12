#include "TypeDefs.h"
#include "IO.h"
#include "BasicRenderer.h"
#include "KBSCodesS1.h"
#include "IDT.h"
#include "PIT.h"

__attribute__((no_caller_saved_registers))
void (*MainKeyboardHandler)(uint_8 scanCode, uint_8 chr) = 0;

// IDTR structure for the lidt instruction
struct IDTR {
    uint_16 Limit;
    uint_64 Offset;
} __attribute__((packed));

IDT64 _idt[256];

// External assembly symbols
extern "C" __attribute__((interrupt)) void isr0(void* frame);
extern "C" __attribute__((interrupt)) void isr1(void* frame);
extern "C" __attribute__((interrupt)) void isr13(void* frame, uint_64 errorCode);

// Define LoadIDT using inline assembly
extern "C" void LoadIDT() {
    IDTR idtr;
    idtr.Limit = (sizeof(IDT64) * 256) - 1;
    idtr.Offset = (uint_64)_idt;
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

uint_16 GetCS() {
    uint_16 cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

// Timer ISR
extern "C" __attribute__((interrupt)) void isr0(void* frame) {
    PIT::Tick();
    outb(0x20, 0x20);
}

// Keyboard ISR
extern "C" __attribute__((interrupt)) void isr1(void* frame) {
    uint_8 scanCode = inb(0x60);
    if (MainKeyboardHandler != 0){
        MainKeyboardHandler(scanCode, KBSet1::ScanCodeLookupTable[scanCode]);
    }
    outb(0x20, 0x20);
}

extern "C" __attribute__((interrupt)) void isr13(void* frame, uint_64 errorCode) {
    GPF_Handler("GENERAL PROTECTION FAULT", errorCode);
}

void MakeIDTEntry(uint_64 handler, uint_16 index, uint_16 selector, uint_8 types_attr){
    _idt[index].offset_low  =  (uint_16)((handler & 0xFFFF));
    _idt[index].selector    =  selector;
    _idt[index].ist         =  0;
    _idt[index].types_attr  =  types_attr;
    _idt[index].offset_mid  =  (uint_16)((handler >> 16) & 0xFFFF);
    _idt[index].offset_high =  (uint_32)((handler >> 32) & 0xFFFFFFFF);
    _idt[index].reserved    =  0;
}

void DisableAPIC() {
    uint_32 lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1B));
    lo &= ~(1 << 11); // Clear APIC Software Enable bit
    __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(0x1B));
}

void InitializeIDT(){
    DisableAPIC();
    RemapPic();
    
    uint_16 cs = GetCS();
    if (GlobalRenderer != nullptr) {
        GlobalRenderer->Print("System Code Segment: 0x");
        GlobalRenderer->Print(HexToString(cs));
        GlobalRenderer->Print("\n");
    }

    MakeIDTEntry((uint_64)isr0, 32, cs, 0x8e);
    MakeIDTEntry((uint_64)isr1, 33, cs, 0x8e);
    MakeIDTEntry((uint_64)isr13, 13, cs, 0x8e);

    outb(0x21, 0xfc); // Enable IRQ0 and IRQ1
    outb(0xA1, 0xff);
    LoadIDT();
}

__attribute__((no_caller_saved_registers))
extern "C" void GPF_Handler(const char* message, uint_64 errorCode) {
    if (GlobalRenderer != nullptr) {
        GlobalRenderer->Clear(0x440000); // Red screen
        GlobalRenderer->Print(message);
        GlobalRenderer->Print("\nError Code: 0x");
        GlobalRenderer->Print(HexToString(errorCode));
    }
    while(1);
}
